module;

#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>

export module voxel_game.utilities:deferred_function;

export namespace vxg::utilities {

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

};	// namespace vxg::utilities

