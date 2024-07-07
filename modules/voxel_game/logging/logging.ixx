module;

#include "voxel_game/debug_macros.hpp"

#include <memory>

export module voxel_game.logging;

export import :logger;

export namespace vxg::logging {

	[[nodiscard]]
	Logger& std_debug_log() noexcept {
		#ifdef DEBUG
		static CombinedLogger stdDebugLog{
			std::make_unique<ConsoleDebugLogger>(),
			std::make_unique<FileDebugLogger>("debug.log")
		};
		#else
		// Potentially optimize in release mode by avoiding combined logger overhead
		static NullLogger stdDebugLog;
		#endif // DEBUG

		return stdDebugLog;
	}

}; // namespace vxg::logging