module;

#include "voxel_game/debug_macros.hpp"

export module voxel_game.logging;

export import :logger;

export namespace vxg::logging {

	[[nodiscard]]
	auto& std_debug_log() noexcept {
		static CombinedLogger logger{
			ConsoleLogger<DebugLogger>{},
			FileLogger<DebugLogger>{"debug.log"}
		};

		return logger;
	}

	[[nodiscard]]
	auto& std_release_log() noexcept {
		static CombinedLogger logger{
			ConsoleLogger<ReleaseLogger>{},
			FileLogger<ReleaseLogger>{"release.log"}
		};

		return logger;
	}

}; // namespace vxg::logging