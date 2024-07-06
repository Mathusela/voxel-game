module;

#include <exception>

export module voxel_game.exceptions;

export import :helpers;

export namespace vxg::exceptions {

	struct LoadError : public std::exception {
		LoadError(const char* message) : std::exception(message) {}
	};

	struct InitError : public std::exception {
		InitError(const char* message) : std::exception(message) {}
	};

	struct InvalidDataError : public std::exception {
		InvalidDataError(const char* message) : std::exception(message) {}
	};

};	// namespae vxg::exceptions