#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <utility>
#include <string_view>
#include <concepts>
#include <tuple>

#include <iostream>
#include <functional>
#include <type_traits>

template <typename Function, typename... Args>
	requires std::invocable<Function, Args&&...>
class DeferredFunction {
	const Function m_function;
	const std::tuple<Args...> m_args;

public:
	// Main constructor
	template <typename... CtorArgs>
	constexpr DeferredFunction(Function function, CtorArgs&&... args) noexcept
		: m_function(std::forward<Function>(function)), m_args(std::forward<CtorArgs>(args)...) {}

	// Copy constructor
	DeferredFunction(const DeferredFunction& df) = delete;

	// Copy assignment
	DeferredFunction& operator=(const DeferredFunction& df) = delete;

	// Move constructor
	DeferredFunction(DeferredFunction&& df) = delete;

	// Move assignment
	DeferredFunction& operator=(DeferredFunction&& df) = delete;
	
	~DeferredFunction() noexcept(std::is_nothrow_invocable_v<Function, Args&&...>) {
		std::apply(m_function, m_args);
	}
};

template<typename Function, typename... CtorArgs>
DeferredFunction(Function, CtorArgs&&...) -> DeferredFunction<Function, CtorArgs...>;

struct LoadError : public std::exception {
	LoadError(std::string_view message) : std::exception(message.data()) {}
};

struct InitError : public std::exception {
	InitError(std::string_view message) : std::exception(message.data()) {}
};

struct WindowProperties {
	std::pair<int, int> resolution;
	std::string_view title;
};

GLFWwindow* create_window(const WindowProperties& properties) {
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(properties.resolution.first, properties.resolution.second, properties.title.data(), nullptr, nullptr);

	glfwMakeContextCurrent(window);

	return window;
}

void load_opengl() {
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
		throw LoadError("Failed to load OpenGL symbols.");
}

void init_glfw() {
	if (!glfwInit())
		throw InitError("Failed to initialise GLFW.");
}

void set_opengl_rendering_state() {
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

void render_loop(GLFWwindow* window) {
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

int main() {
	try { init_glfw(); }
	catch (const InitError& e) {
		std::cerr << e.what();
		return EXIT_FAILURE;
	}
	DeferredFunction deferredGLFWTerminate(glfwTerminate);
	
	constexpr WindowProperties windowProperties {{700, 500}, "Voxel Game"};
	auto window = create_window(windowProperties);

	// TODO: Add resizing callback
	
	try { load_opengl(); }
	catch (const LoadError& e) {
		std::cerr << e.what();
		return EXIT_FAILURE;
	}
	
	glViewport(0, 0, windowProperties.resolution.first, windowProperties.resolution.second);


	set_opengl_rendering_state();
	render_loop(window);

	return EXIT_SUCCESS;
}