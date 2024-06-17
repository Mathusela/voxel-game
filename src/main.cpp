#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <utility>
#include <string_view>

#include <iostream>
#include <functional>
#include <type_traits>

template <typename T>
class ScopedResource {
	std::function<void(std::remove_reference_t<T>&)> m_destructor;
	T m_resource;

public:
	// Construct with constructor function
	template <typename Destructor>
	ScopedResource(std::function<std::remove_reference_t<T>()> constructor, Destructor destructor):
		m_destructor(destructor)
	{
		static_assert(std::is_nothrow_invocable_v<Destructor, std::remove_reference_t<T>&>);
		m_resource = constructor();
	}

	// Construct by capturing resource
	template <typename Destructor>
	ScopedResource(T&& resource, Destructor destructor) noexcept :
		m_resource(std::move(resource)), m_destructor(destructor)
	{
		static_assert(std::is_nothrow_invocable_v<Destructor, std::remove_reference_t<T>&>);
	}

	// Copy constructor
	ScopedResource(const ScopedResource& sr) noexcept {
		m_resource = sr.m_resource;
		m_destructor = sr.m_destructor;
	}

	// Copy assignment
	ScopedResource& operator=(const ScopedResource& sr) noexcept {
		m_destructor(m_resource);
		m_resource = sr.m_resource;
		m_destructor = sr.m_destructor;
		return *this;
	}

	// Move constructor
	ScopedResource(ScopedResource&& sr) noexcept {
		m_resource = std::move(sr.m_resource);
		m_destructor = std::move(sr.m_destructor);
		sr.m_destructor = nullptr;
	}

	// Move assignment
	ScopedResource& operator=(ScopedResource&& sr) {
		m_destructor(m_resource);
		m_resource = std::move(sr.m_resource);
		m_destructor = std::move(sr.m_destructor);
		sr.m_destructor = nullptr;
		return *this;
	}

	~ScopedResource() noexcept {
		if (m_destructor != nullptr) m_destructor(m_resource);
	}

	T& get() noexcept {
		return m_resource;
	}
};

template <>
class ScopedResource<void> {
	std::function<void()> m_destructor;

public:
	// Construct with constructor function
	template <typename Destructor>
	ScopedResource(std::function<void()> constructor, Destructor destructor) :
		m_destructor(destructor)
	{
		static_assert(std::is_nothrow_invocable_v<Destructor>);
		constructor();
	}

	// Construct only passing destructor
	template <typename Destructor>
	ScopedResource(Destructor destructor) noexcept :
		m_destructor(destructor)
	{
		static_assert(std::is_nothrow_invocable_v<Destructor>);
	}

	// Copy constructor
	ScopedResource(const ScopedResource& sr) noexcept = delete;

	// Copy assignment
	ScopedResource& operator=(const ScopedResource& sr) noexcept = delete;

	// Move constructor
	ScopedResource(ScopedResource&& sr) noexcept = delete;

	// Move assignment
	ScopedResource& operator=(ScopedResource&& sr) = delete;

	~ScopedResource() noexcept {
		m_destructor();
	}
};

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
	ScopedResource<void> deferredGLFWInstanceTermination(glfwTerminate);
	
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