module;

#include <iostream>

export module voxel_game.exceptions:helpers;

import voxel_game.typedefs;

export namespace vxg::exceptions {

	template <typename T>
	vxg::ExitCode handle_unrecoverable_error(const T& error) noexcept {
		std::cerr << error.what() << "\n";
		return EXIT_FAILURE;
	}

};	// namespae vxg::exceptions