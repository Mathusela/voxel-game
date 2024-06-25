module;

#include "voxel_game/debug_macros.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <memory>
#include <vector>
#include <iostream>
#include <type_traits>

export module voxel_game.core.rendering:rendering_backend;

import :window_manager;
import :structs;
import voxel_game.exceptions;

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

export namespace vxg::core::rendering {

	template <typename Derived>
	class RenderingBackend {
		bool m_terminated = false;
		
		RenderingBackend() {
			if (!glfwInit())
				throw vxg::exceptions::InitError("Failed to initialise GLFW.");
		}
		friend Derived;


		const Derived* derived_instance() const noexcept {
			return static_cast<const Derived*>(this);
		}

		Derived* derived_instance() noexcept {
			return static_cast<Derived*>(this);
		}

	public:
		void terminate() noexcept {
			m_terminated = true;
			derived_instance()->terminate_impl();
		}
		
		virtual ~RenderingBackend() noexcept {
			if (!m_terminated)
				terminate();
		}

		// Copy constructor
		RenderingBackend(const RenderingBackend& rb) = delete;

		// Copy assignment
		RenderingBackend& operator=(const RenderingBackend& rb) = delete;

		// Move constructor
		RenderingBackend(RenderingBackend&& rb) noexcept {
			rb.m_terminated = true;
		}

		// Move assignment
		RenderingBackend& operator=(RenderingBackend&& rb) noexcept {
			rb.m_terminated = true;
		}

		void initialize_api()
			noexcept(noexcept(derived_instance()->initialize_api_impl()))
		{
			derived_instance()->initialize_api_impl();
		}

		std::unique_ptr<WindowManager> construct_window(const vxg::core::rendering::WindowProperties& properties) const
			noexcept(noexcept(derived_instance()->construct_window_impl(properties)))
		{
			return derived_instance()->construct_window_impl(properties);
		}

		void clear_screen() const
			noexcept(noexcept(derived_instance()->clear_screen_impl()))
		{
			derived_instance()->clear_screen_impl();
		}

		void set_clear_color(const glm::vec4& color)
			noexcept(noexcept(derived_instance()->set_clear_color_impl(color)))
		{
			derived_instance()->set_clear_color_impl(color);
		}

		void enqueue_draw(const std::vector<vxg::core::rendering::Vertex>& verts)
			noexcept(noexcept(derived_instance()->enqueue_draw_impl(verts)))
		{
			derived_instance()->enqueue_draw_impl(verts);
		}

		void draw_queued() const
			noexcept(noexcept(derived_instance()->draw_queued_impl()))
		{
			derived_instance()->draw_queued_impl();
		}

		void configure_viewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
			noexcept(noexcept(derived_instance()->configure_viewport_impl(x, y, width, height)))
		{
			derived_instance()->configure_viewport_impl(x, y, width, height);
		}
	};

	template <typename T>
	concept rendering_backend_implementation = std::is_base_of_v<RenderingBackend<T>, T>;

	class OpenGLBackend final : public RenderingBackend<OpenGLBackend> {
		using Base = RenderingBackend<OpenGLBackend>;
		friend Base;

		std::vector<GLuint> m_vaoArray;
		std::vector<GLuint> m_vboArray;
		GLsizei m_queuedVertsCount = 0;
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
			// VAO + VBOs
			this->m_vaoArray.push_back(0);
			glCreateVertexArrays(1, &m_vaoArray.back());

			m_vboArray.push_back({});
			glCreateBuffers(1, &m_vboArray.back());

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

			glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
			glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

			int success;
			char infoLog[512];

			glCompileShader(vertexShader);
			glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
				throw vxg::exceptions::InitError("Vertex shader compilation failed.\n" + std::string(infoLog));
			}

			glCompileShader(fragmentShader);
			glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
				throw vxg::exceptions::InitError("Fragment shader compilation failed.\n" + std::string(infoLog));
			}

			m_shaderProgram = glCreateProgram();
			glAttachShader(m_shaderProgram, vertexShader);
			glAttachShader(m_shaderProgram, fragmentShader);
			glLinkProgram(m_shaderProgram);

			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			// Setup attribute bindings
			auto vao = m_vaoArray[0];
			auto vbo = m_vboArray[0];

			// Bind VBO to binding index
			glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));
			// Bind attributes to attribute indicies
			glEnableVertexArrayAttrib(vao, 0);
			glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));	// TODO: Write helper to get number of components from glm::vec{x}
			// Bind attribute indices to binding indices
			glVertexArrayAttribBinding(vao, 0, 0);
		}

		void terminate_impl() noexcept {
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

		void enqueue_draw_impl(const std::vector<vxg::core::rendering::Vertex>& verts) noexcept {
			// Somehow make it so you dont have to replace the data each frame?
			glNamedBufferData(m_vboArray[0], verts.size()*sizeof(Vertex), verts.data(), GL_DYNAMIC_DRAW);
			m_queuedVertsCount = static_cast<GLsizei>(verts.size());
		}

		void draw_queued_impl() const noexcept {
			glUseProgram(m_shaderProgram);
			
			glBindVertexArray(m_vaoArray[0]);
			glDrawArrays(GL_TRIANGLES, 0, m_queuedVertsCount);
			glBindVertexArray(NULL);
		}

		void configure_viewport_impl(unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
			glViewport(x, y, width, height);
		}

	public:
		OpenGLBackend(): Base() {}
	};

}; // namespace vxg::core::rendering