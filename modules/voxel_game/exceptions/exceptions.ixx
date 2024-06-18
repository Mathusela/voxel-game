module;

#include <exception>
#include <string_view>

export module voxel_game.exceptions;

export namespace vxg::exceptions {

	struct LoadError : public std::exception {
		LoadError(std::string_view message) : std::exception(message.data()) {}
	};

	struct InitError : public std::exception {
		InitError(std::string_view message) : std::exception(message.data()) {}
	};

};	// namespae vxg::exceptions