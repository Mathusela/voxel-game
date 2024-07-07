module;

#include "voxel_game/debug_macros.hpp"

#include <memory>

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

}; // namespace vxg::logging