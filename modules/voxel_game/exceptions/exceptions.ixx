module;

#include <stdexcept>
#include <string_view>

export module voxel_game.exceptions;

export import :helpers;

export namespace vxg::exceptions {

	struct LoadError : public std::runtime_error {
		LoadError(std::string_view message) : std::runtime_error(message.data()) {}
	};

	struct InitError : public std::runtime_error {
		InitError(std::string_view message) : std::runtime_error(message.data()) {}
	};

	struct InvalidDataError : public std::logic_error {
		InvalidDataError(std::string_view message) : std::logic_error(message.data()) {}
	};

	struct MemorySafetyError : public std::runtime_error {
		MemorySafetyError(std::string_view message) : std::runtime_error(message.data()) {}
	};

}; // namespcae vxg::exceptions