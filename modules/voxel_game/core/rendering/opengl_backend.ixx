module;

#include "voxel_game/debug_macros.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <memory>
#include <vector>
#include <iostream>
#include <type_traits>
#include <string>
#include <cstdint>
#include <set>
#include <algorithm>
#include <ranges>

export module voxel_game.core.rendering:opengl_backend;

import :rendering_backend;
import :window_manager;
import :structs;
import voxel_game.exceptions;
import voxel_game.utilities;

struct DrawArraysIndirectCommand {
	GLuint count;
	GLuint instanceCount;
	GLuint firstVertex;
	GLuint baseInstance;
};

struct DrawElementsIndirectCommand {
	GLuint count;
	GLuint instanceCount;
	GLuint firstIndex;
	GLint baseVertex;
	GLuint baseInstance;
};

// Taken from https://learnopengl.com/In-Practice/Debugging
void APIENTRY debug_callback(GLenum source, GLenum type, unsigned int id, GLenum severity, [[maybe_unused]] GLsizei length, const char* message, [[maybe_unused]] const void* userParam) {
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}

void compile_shader(GLuint shader, const std::string& shaderDisplayName) {
	int success;
	char infoLog[512];

	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		throw vxg::exceptions::InitError(shaderDisplayName + " compilation failed.\n" + std::string(infoLog));
	}
}

struct DrawCommandRegion {
	uint_fast32_t bufferIndex;
	uint_fast32_t size;
};

// Sort in memory order in order to efficiently merge blocks on free
template <>
struct std::less<vxg::core::rendering::GPUAllocationIdentifier> {
	constexpr bool operator()
		(const vxg::core::rendering::GPUAllocationIdentifier& left, const vxg::core::rendering::GPUAllocationIdentifier& right) const
	{
		return left.memoryPoolIndex == right.memoryPoolIndex ? left.bufferOffset < right.bufferOffset : left.memoryPoolIndex < right.memoryPoolIndex;
	}
};
template <>
struct std::less<DrawCommandRegion> {
	constexpr bool operator()
		(const DrawCommandRegion& left, const DrawCommandRegion& right) const
	{
		return left.bufferIndex < right.bufferIndex;
	}
};

struct MemoryPool {
	GLuint vao;
	GLuint vbo;
	GLuint drawCommandBuffer;

	GLsizei queuedDrawsCount = 0;
	uint_fast32_t freeDataMemorySize;
	std::set<DrawCommandRegion> freeDrawMemory;
};


export namespace vxg::core::rendering {

	class OpenGLBackend final : public RenderingBackend<OpenGLBackend> {
		using Base = RenderingBackend<OpenGLBackend>;
		friend Base;

		const uint16_t m_numMemoryPools;
		const uint_fast32_t m_dataBufferInitialSize;
		const uint_fast32_t m_drawBufferInitialSize;
		std::vector<MemoryPool> m_memoryPools;
		std::set<vxg::core::rendering::GPUAllocationIdentifier> m_freeDataMemory;
		GLuint m_shaderProgram;

		// Depends on a current window context
		void initialize_api_impl() {
			// Load dynamic OpenGL bindings
			if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
				throw vxg::exceptions::LoadError("Failed to load OpenGL symbols.");

			// Enable debug output
			#ifdef DEBUG
			int flags;
			glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
			if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
				glEnable(GL_DEBUG_OUTPUT);
				glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
				glDebugMessageCallback(debug_callback, nullptr);
				glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
			}
			else {
				std::cerr << "Warning: Failed to enable debug output.\n";
			}
			#endif // DEBUG

			// Initialize OpenGL resources
			// Memory pools
			for (int i = 0; i < m_numMemoryPools; i++) {
				m_memoryPools.push_back({});
				auto& memPool = m_memoryPools.back();
				glCreateVertexArrays(1, &memPool.vao);
				glCreateBuffers(1, &memPool.vbo);
				glNamedBufferData(memPool.vbo, m_dataBufferInitialSize, nullptr, GL_DYNAMIC_DRAW);
				glCreateBuffers(1, &memPool.drawCommandBuffer);
				glNamedBufferData(memPool.drawCommandBuffer, m_drawBufferInitialSize, nullptr, GL_DYNAMIC_DRAW);

				// Free memory
				memPool.freeDataMemorySize = m_dataBufferInitialSize;
				m_freeDataMemory.insert(GPUAllocationIdentifier{
					.memoryPoolIndex = static_cast<uint16_t>(i),
					.bufferOffset = 0,
					.vertexCount = m_dataBufferInitialSize / sizeof(Vertex)
				});
				memPool.freeDrawMemory.insert(DrawCommandRegion{
					.bufferIndex = 0,
					.size = m_drawBufferInitialSize / static_cast<uint_fast32_t>(sizeof(DrawArraysIndirectCommand))
				});
				
				// Setup attribute bindings
				auto vao = memPool.vao;
				auto vbo = memPool.vbo;

				// Bind VBO to binding index
				glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));
				// Bind attributes to attribute indicies
				glEnableVertexArrayAttrib(vao, 0);
				glVertexArrayAttribFormat(vao, 0, vxg::utilities::num_components<decltype(Vertex::position)>(), GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
				// Bind attribute indices to binding indices
				glVertexArrayAttribBinding(vao, 0, 0);
			}
			
			// Shaders
			const char* vertexShaderSource =
				"#version 460 core\n"
				"layout (location = 0) in vec3 vPos;\n"
				"out vec3 fPos;\n"
				"void main() {\n"
				"    fPos = vPos;\n"
				"    gl_Position = vec4(fPos, 1.0);"
				"}";

			const char* fragmentShaderSource =
				"#version 460 core\n"
				"in vec3 fPos;\n"
				"out vec4 fCol;\n"
				"void main() {\n"
				"    fCol = vec4(1.0, 0.0, 0.0, 1.0);\n"
				"}";

			auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
			auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
			//                               vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv This just fixes intelisense
			vxg::utilities::DeferredFunction<decltype(glDeleteShader), GLuint> deferredVertexShaderDelete(glDeleteShader, vertexShader);
			vxg::utilities::DeferredFunction<decltype(glDeleteShader), GLuint> deferredFragmentShaderDelete(glDeleteShader, fragmentShader);

			glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
			glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

			compile_shader(vertexShader, "Vertex shader");
			compile_shader(fragmentShader, "Fragment shader");

			m_shaderProgram = glCreateProgram();
			glAttachShader(m_shaderProgram, vertexShader);
			glAttachShader(m_shaderProgram, fragmentShader);
			glLinkProgram(m_shaderProgram);
			// TODO: Error handling for program linking

			glUseProgram(m_shaderProgram);
		}

		void terminate_impl() noexcept {
			for (auto& memPool : m_memoryPools) {
				glDeleteBuffers(1, &memPool.drawCommandBuffer);
				glDeleteBuffers(1, &memPool.vbo);
				glDeleteVertexArrays(1, &memPool.vao);
			}
			glDeleteProgram(m_shaderProgram);

			glfwTerminate();
		}

		std::unique_ptr<WindowManager> construct_window_impl(const vxg::core::rendering::WindowProperties& properties) const {
			return std::make_unique<vxg::core::rendering::WindowManager>(properties);
		}

		void clear_screen_impl() const noexcept {
			glClear(GL_COLOR_BUFFER_BIT);
		}

		void set_clear_color_impl(const glm::vec4& color) noexcept {
			glClearColor(color.r, color.g, color.b, color.a);
		}

		[[nodiscard]]
		GPUAllocationIdentifier copy_to_vram_impl(const std::vector<vxg::core::rendering::Vertex>& verts) noexcept {
			// Get free memory
			auto numVerts = verts.size();

			// Get the smallest chunk that can contain the data preferring sparse buffers
			auto validChunks = m_freeDataMemory
				| std::views::filter([numVerts](const GPUAllocationIdentifier& mem) {return numVerts <= mem.vertexCount; });
			auto freeIt = std::ranges::min_element(validChunks, [this](const GPUAllocationIdentifier& left, const GPUAllocationIdentifier& right) {
				if (m_memoryPools[left.memoryPoolIndex].freeDataMemorySize == m_memoryPools[right.memoryPoolIndex].freeDataMemorySize)
					return left.vertexCount < right.vertexCount;
				else
					return m_memoryPools[left.memoryPoolIndex].freeDataMemorySize > m_memoryPools[right.memoryPoolIndex].freeDataMemorySize;
			});

			GPUAllocationIdentifier free;
			if (freeIt != validChunks.end()) {
				free = *freeIt;
			}
			else {
				// Resize
				free = {};
			}

			// Remove memory from the free list to allocate it
			m_freeDataMemory.erase(freeIt.base());

			// Copy into memory
			auto sizeToAllocate = verts.size() * sizeof(Vertex);
			glNamedBufferSubData(m_memoryPools[free.memoryPoolIndex].vbo, free.bufferOffset, sizeToAllocate, verts.data());

			// Add remainder of free memory back to free list
			if (free.vertexCount - verts.size() > 0)
				m_freeDataMemory.insert(GPUAllocationIdentifier{
					.memoryPoolIndex = free.memoryPoolIndex,
					.bufferOffset = free.bufferOffset + static_cast<uint_fast32_t>(sizeToAllocate),
					.vertexCount = free.vertexCount - verts.size()
				});
			m_memoryPools[free.memoryPoolIndex].freeDataMemorySize -= static_cast<uint_fast32_t>(sizeToAllocate);

			std::cout << "ALLOC: ";	// TESTING, REMOVE ME
			for (const auto& x : m_freeDataMemory)
				std::cout << x.vertexCount << ", ";
			std::cout << "\n";

			// Return identifier
			return GPUAllocationIdentifier{
				.memoryPoolIndex = free.memoryPoolIndex,
				.bufferOffset = free.bufferOffset,
				.vertexCount = verts.size()
			};
		}

		void deallocate_vram_impl(const GPUAllocationIdentifier& alloc) noexcept {
			GPUAllocationIdentifier free = alloc;
			
			// Merge data with surrounding free blocks
			auto bound = m_freeDataMemory.upper_bound(alloc);	// First element at a memory location greater than the block
			bool validBound = bound != m_freeDataMemory.end();

			// Greater block
			if (validBound) {
				auto upperBound = bound;	// Copy for use in merge
				bound--;	// Decrement for lesser block - do this now as we may erase lowerBound invalidating the iterator if we do not increment
				
				if (upperBound->memoryPoolIndex == alloc.memoryPoolIndex && upperBound->bufferOffset == alloc.bufferOffset + alloc.vertexCount*sizeof(Vertex)) {
					// Merge
					free.vertexCount += upperBound->vertexCount;

					// Remove merged block
					m_freeDataMemory.erase(upperBound);
				}
			}
			else {
				bound = std::prev(m_freeDataMemory.end());	// There was no greater block, the lesser block must be the last in the set
			}

			// Lesser block
			validBound = bound != m_freeDataMemory.end();
			if (validBound) {
				if (bound->memoryPoolIndex == alloc.memoryPoolIndex && bound->bufferOffset + bound->vertexCount*sizeof(Vertex) == alloc.bufferOffset) {
					// Merge
					free.bufferOffset = bound->bufferOffset;
					free.vertexCount += bound->vertexCount;
					
					// Remove merged block
					m_freeDataMemory.erase(bound);
				}
			}
			
			// Add block to free list to deallocate
			m_freeDataMemory.insert(free);
			m_memoryPools[alloc.memoryPoolIndex].freeDataMemorySize += static_cast<uint_fast32_t>(alloc.vertexCount * sizeof(Vertex));
			// TODO: SET DRAW THING TO 0 WHEN FREEING

			std::cout << "DEALLOC: ";	// TESTING, REMOVE ME
			for (const auto& x : m_freeDataMemory) {
				std::cout << "{vertexCount: " << x.vertexCount << ", memoryPoolIndex: " << x.memoryPoolIndex << ", bufferOffset: " << x.bufferOffset << "}, ";
			}
			std::cout << "\n";
		}

		[[nodiscard]]
		DrawCommandIdentifier enqueue_draw_impl(const GPUAllocationIdentifier& alloc) noexcept {
			auto& memPool = m_memoryPools[alloc.memoryPoolIndex];
			
			if (memPool.freeDrawMemory.empty()) {
				// TODO: Resize buffer
			}

			// Get free memory
			auto freeIt = memPool.freeDrawMemory.begin();
			auto free = *freeIt;
			memPool.freeDrawMemory.erase(freeIt);
			
			// Copy draw command to memory
			DrawArraysIndirectCommand drawCommand{
				.count = static_cast<GLuint>(alloc.vertexCount),
				.instanceCount = 1,
				.firstVertex = alloc.bufferOffset,
				.baseInstance = 0,
			};
			glNamedBufferSubData(memPool.drawCommandBuffer, free.bufferIndex * sizeof(DrawArraysIndirectCommand), 1 * sizeof(DrawArraysIndirectCommand), &drawCommand);

			// Add remainder of free region back to free list
			if (free.size > 1)
				memPool.freeDrawMemory.insert(DrawCommandRegion{
					.bufferIndex = free.bufferIndex + 1,
					.size = free.size - 1
				});

			std::cout << "DRAW: " << memPool.freeDrawMemory.begin()->size << "\n";	// TESTING, REMOVE ME

			auto& queuedDrawsCount = m_memoryPools[alloc.memoryPoolIndex].queuedDrawsCount;
			queuedDrawsCount = std::max(queuedDrawsCount, static_cast<GLsizei>(free.bufferIndex / sizeof(DrawArraysIndirectCommand)) + 1);

			return DrawCommandIdentifier{
				.memoryPoolIndex = alloc.memoryPoolIndex,
				.bufferIndex = free.bufferIndex
			};
		}

		void dequeue_draw_impl(const DrawCommandIdentifier& drawCommand) noexcept {
			DrawCommandRegion free = {
				.bufferIndex = drawCommand.bufferIndex,
				.size = 1
			};

			auto& memPool = m_memoryPools[drawCommand.memoryPoolIndex];

			// Merge data with surrounding free blocks
			auto bound = memPool.freeDrawMemory.upper_bound(free);	// First element at a memory location greater than the block
			bool validBound = bound != memPool.freeDrawMemory.end();

			// Greater block
			if (validBound) {
				auto upperBound = bound;	// Copy for use in merge
				bound--;	// Decrement for lesser block - do this now as we may erase lowerBound invalidating the iterator if we do not increment

				if (upperBound->bufferIndex == drawCommand.bufferIndex + 1) {
					// Merge
					free.size += upperBound->size;

					// Remove merged block
					memPool.freeDrawMemory.erase(upperBound);
				}
			}
			else {
				bound = std::prev(memPool.freeDrawMemory.end());	// There was no greater block, the lesser block must be the last in the set
			}

			// Lesser block
			validBound = bound != memPool.freeDrawMemory.end();
			if (validBound) {
				if (bound->bufferIndex + bound->size == drawCommand.bufferIndex) {
					// Merge
					free.bufferIndex = bound->bufferIndex;
					free.size += bound->size;

					// Remove merged block
					memPool.freeDrawMemory.erase(bound);
				}
			}

			// Add block to free list to deallocate
			memPool.freeDrawMemory.insert(free);
			GLuint instanceCount = 0;
			glNamedBufferSubData(memPool.drawCommandBuffer, drawCommand.bufferIndex*sizeof(DrawArraysIndirectCommand) + offsetof(DrawArraysIndirectCommand, instanceCount), sizeof(DrawArraysIndirectCommand::instanceCount), &instanceCount);

			std::cout << "UNQUEUE DRAW: ";	// TESTING, REMOVE ME
			for (const auto& x : memPool.freeDrawMemory) {
				std::cout << "{size: " << x.size << ", bufferIndex: " << x.bufferIndex << "}, ";
			}
			std::cout << "\n";
		}

		void draw_queued_impl() const noexcept {
			for (const auto& memPool : m_memoryPools) {
				glBindVertexArray(memPool.vao);
				glBindBuffer(GL_DRAW_INDIRECT_BUFFER, memPool.drawCommandBuffer);
				glMultiDrawArraysIndirect(GL_TRIANGLES, 0, memPool.queuedDrawsCount, 0);
			}
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, NULL);
			glBindVertexArray(NULL);
		}

		void configure_viewport_impl(unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
			glViewport(x, y, width, height);
		}

	public:
		OpenGLBackend(const uint16_t numMemoryPools, const uint_fast32_t dataBufferInitialSize, const uint_fast32_t drawBufferInitialSize)
			: Base(), m_numMemoryPools(numMemoryPools), m_dataBufferInitialSize(dataBufferInitialSize * sizeof(Vertex)), m_drawBufferInitialSize(drawBufferInitialSize * sizeof(DrawArraysIndirectCommand)) {}
	};

}; // namespace vxg::core::rendering