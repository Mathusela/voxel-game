module;

#include "voxel_game/debug_macros.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>
#include <vector>
#include <type_traits>
#include <string>
#include <utility>

export module voxel_game.core.rendering:opengl_backend;

import :rendering_backend;
import :window_manager;
import :camera;
import voxel_game.core.structs;
import voxel_game.exceptions;
import voxel_game.utilities;
import voxel_game.logging;

// Adapted from https://learnopengl.com/In-Practice/Debugging
void APIENTRY debug_callback(GLenum source, GLenum type, unsigned int id, GLenum severity, [[maybe_unused]] GLsizei length, const char* message, [[maybe_unused]] const void* userParam) {
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::string output = "<OpenGL debug callback>\n";
	//output += "---------------\n";
	output += "Debug message (" + std::to_string(id) + "): " + message + "\n";

	switch (source) {
	case GL_DEBUG_SOURCE_API:             output += "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   output += "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: output += "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     output += "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     output += "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           output += "Source: Other"; break;
	}
	output += "\n";

	vxg::logging::LogType logType = vxg::logging::LogType::Warning;
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:               output += "Type: Error"; logType = vxg::logging::LogType::Error; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: output += "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  output += "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         output += "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         output += "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              output += "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          output += "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           output += "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               output += "Type: Other"; break;
	} output += "\n";

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:         output += "Severity: High"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       output += "Severity: Medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          output += "Severity: Low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: output += "Severity: Notification"; logType = vxg::logging::LogType::Info; break;
	}
	output += "\n";
	output += "---------------\n";

	vxg::logging::std_debug_log().log(logType, output);
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

export namespace vxg::core::rendering {

	struct OpenGLUniforms {
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
	};

	template <typename Allocator>
	class OpenGLBackend;

	template <typename Allocator>
	struct RenderingBackendTraits<OpenGLBackend<Allocator>> {
		using UniformType = OpenGLUniforms;
	};

	template <typename Allocator>
	class OpenGLBackend final : public RenderingBackend<OpenGLBackend<Allocator>> {
		using Base = RenderingBackend<OpenGLBackend<Allocator>>;
		friend Base;

		Allocator m_allocator;
		GLuint m_shaderProgram;
		GLuint m_ubo;

		bool m_depthTestingEnabled = false;

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
				vxg::logging::std_debug_log().log(vxg::logging::LogType::Warning, "Failed to enable debug output.\n");
			}
			#endif // DEBUG

			// Initialize allocator
			m_allocator.initialize();

			// Shaders
			const char* vertexShaderSource =
				"#version 460 core\n"
				"layout (location = 0) in vec3 vPos;\n"
				"layout (location = 1) in vec4 modelMatrixA;\n"
				"layout (location = 2) in vec4 modelMatrixB;\n"
				"layout (location = 3) in vec4 modelMatrixC;\n"
				"layout (location = 4) in vec4 modelMatrixD;\n"
				"layout (std140, binding=0) uniform Uniforms {\n"
				"    mat4 viewMatrix;\n"
				"    mat4 projectionMatrix;\n"
				"};\n"
				"out vec3 fWorldPos;\n"
				"void main() {\n"
				"    mat4 modelMatrix = mat4(modelMatrixA, modelMatrixB, modelMatrixC, modelMatrixD);\n"
				"    vec4 worldPos = modelMatrix * vec4(vPos, 1.0);\n"
				"    fWorldPos = worldPos.xyz;\n"
				"    gl_Position = projectionMatrix * viewMatrix * worldPos;\n"
				"}";

			const char* fragmentShaderSource =
				"#version 460 core\n"
				"in vec3 fWorldPos;\n"
				"out vec4 fCol;\n"
				"void main() {\n"
				"    vec3 biasedWorldPos = fWorldPos - 0.0001;\n"
				"    fCol = vec4(mod(biasedWorldPos.x, 1.0), mod(biasedWorldPos.y, 1.0), mod(biasedWorldPos.z, 1.0), 1.0);\n"
				"}";

			auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
			auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
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

			// Uniform buffer object
			glCreateBuffers(1, &m_ubo);
			glNamedBufferStorage(m_ubo, sizeof(typename Base::UniformType), nullptr, GL_DYNAMIC_STORAGE_BIT);
			glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_ubo);
		}

		void terminate_impl() noexcept {
			if (Base::m_initialized)
				glDeleteProgram(m_shaderProgram);

			glfwTerminate();
		}

		std::unique_ptr<WindowManager> construct_window_impl(const vxg::core::rendering::WindowProperties& properties) const {
			return std::make_unique<vxg::core::rendering::WindowManager>(properties);
		}

		void clear_screen_impl() const noexcept {
			glClear(m_depthTestingEnabled ? GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT : GL_COLOR_BUFFER_BIT);
		}

		void set_clear_color_impl(const glm::vec4& color) noexcept {
			glClearColor(color.r, color.g, color.b, color.a);
		}

		[[nodiscard]]
		Base::ObjectAllocationIdentifier construct_object_impl(const std::vector<vxg::core::structs::Vertex>& verts, const vxg::core::structs::ObjectData& data)
			noexcept(std::is_nothrow_invocable_v<decltype(&Allocator::allocate_object), Allocator, size_t> &&
				std::is_nothrow_invocable_v<decltype(&Allocator::construct_vertices), Allocator, typename Allocator::AllocationIdentifier, decltype(verts)> &&
				std::is_nothrow_invocable_v<decltype(&Allocator::construct_data), Allocator, typename Allocator::AllocationIdentifier, decltype(data)>)
		{
			auto alloc = m_allocator.allocate_object(verts.size());
			m_allocator.construct_vertices(alloc.vertices, verts);
			m_allocator.construct_data(alloc.data, data);
			return alloc;
		}

		void destroy_object_impl(const Base::ObjectAllocationIdentifier& object)
			noexcept(noexcept(m_allocator.destroy(object)) &&
				noexcept(m_allocator.deallocate(object)))
		{
			m_allocator.destroy(object);
			m_allocator.deallocate(object);
		}

		void enqueue_draw_impl(const Base::ObjectAllocationIdentifier& object)
			noexcept(std::is_nothrow_invocable_v<decltype(&Allocator::construct_draw), Allocator, typename Allocator::AllocationIdentifier, typename Allocator::AllocationIdentifier, typename Allocator::AllocationIdentifier>)
		{
			m_allocator.construct_draw(object.draw, object.vertices, object.data);
		}

		void dequeue_draw_impl(const Base::ObjectAllocationIdentifier& object)
			noexcept(noexcept(m_allocator.destroy(object.draw)))
		{
			m_allocator.destroy(object.draw);
		}

		void draw_queued_impl() const noexcept {
			for (const auto& drawResources : m_allocator.get_draw_resources()) {
				glBindVertexArray(drawResources.vao);
				glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawResources.drawCommandBuffer);
				glMultiDrawArraysIndirect(GL_TRIANGLES, nullptr, drawResources.drawCommandCount, 0);
			}
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, NULL);
			glBindVertexArray(NULL);
		}

		void update_uniforms_impl(const Base::UniformType& data) noexcept {
			glNamedBufferSubData(m_ubo, 0, sizeof(typename Base::UniformType), &data);
		}

		void configure_viewport_impl(unsigned int x, unsigned int y, unsigned int width, unsigned int height) noexcept {
			glViewport(x, y, width, height);
		}

		void enable_depth_testing_impl() noexcept {
			glEnable(GL_DEPTH_TEST);
			m_depthTestingEnabled = true;
		}

		void enable_face_culling_impl() noexcept {
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CCW);
		}

		void enable_multisampling_impl() noexcept {
			glEnable(GL_MULTISAMPLE);
		}

		void enable_wireframe_impl() noexcept {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		void disable_wireframe_impl() noexcept {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

	public:
		using AllocatorType = Allocator;

		template <typename... Args>
			requires std::is_constructible_v<Allocator, Args&&...>
		OpenGLBackend(Args&&... allocatorArgs)
			: Base(), m_allocator(Allocator {std::forward<Args>(allocatorArgs)...}) {}

		// Copy constructor
		OpenGLBackend(const OpenGLBackend& ob) = delete;

		// Copy assignment
		OpenGLBackend& operator=(const OpenGLBackend& ob) = delete;

		// Move constructor
		OpenGLBackend(OpenGLBackend&& ob) noexcept
			: Base(std::move(ob)), m_allocator(std::move(ob.m_allocator)), m_shaderProgram(std::move(ob.m_shaderProgram)), m_ubo(std::move(ob.m_ubo)) {}

		// Move assignment
		OpenGLBackend& operator=(OpenGLBackend&& ob) noexcept {
			Base::operator=(std::move(ob));
			m_allocator = std::move(ob.m_allocator);
			m_shaderProgram = std::move(ob.m_shaderProgram);
			m_ubo = std::move(ob.m_ubo);
		}

		~OpenGLBackend() noexcept = default;
	};

}; // namespace vxg::core::rendering