module;

#include <cstdint>
#include <string_view>

export module voxel_game.utilities;

export import :deferred_function;
export import :glm_helpers;
export import voxel_game.utilities.tmp;

export namespace vxg::utilities {
	
	// FNV-1a hash
	// https://softwareengineering.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed
	// Compile-time capable
	[[nodiscard]]
	constexpr std::uint64_t hash_string(std::string_view str) noexcept {
		std::uint64_t hash = 14695981039346656037ULL;

		for (const char c : str) {
			hash ^= c;
			hash *= 1099511628211ULL;
		}

		return hash;
	}

} // namespace vxg::utilities