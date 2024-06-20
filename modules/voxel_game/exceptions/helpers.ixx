module;

#include <iostream>
#include <variant>
#include <type_traits>
#include <utility>

export module voxel_game.exceptions:helpers;

import voxel_game.typedefs;

namespace vxg::exceptions {

	template <typename Instance, typename ErrorResult>
	struct InstanceOrErrorResult {
		std::variant<Instance, ErrorResult> result;
		bool encounteredException;

		constexpr Instance& get_instance() {
			if (encounteredException)
				throw std::bad_variant_access{};
			return std::get<0>(result);
		}

		constexpr ErrorResult& get_error_result() {
			if (!encounteredException)
				throw std::bad_variant_access{};
			return std::get<1>(result);
		}
	};

}; // namespace vxg::exceptions

export namespace vxg::exceptions {

	template <typename T>
	[[nodiscard]]
	vxg::ExitCode handle_unrecoverable_error(const T& error) noexcept {
		std::cerr << error.what() << "\n";
		return EXIT_FAILURE;
	}

	template <typename T, typename Exception, typename Handler, typename... Args>
		requires std::is_invocable_v<Handler, const Exception&> && std::is_constructible_v<T, Args...>
	[[nodiscard]]
	constexpr InstanceOrErrorResult<T, std::invoke_result_t<Handler, const Exception&>> construct_and_catch(Handler handler, Args&&... args)
		noexcept(std::is_nothrow_invocable_v<Handler, const Exception&>)
	{
		try {
			return {
				T {std::forward<Args>(args)...},
				false
			};
		}
		catch (const Exception& e) {
			return {
				std::invoke(handler, e),
				true
			};
		}
	}

};	// namespace vxg::exceptions