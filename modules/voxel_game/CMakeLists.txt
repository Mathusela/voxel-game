target_sources(modules PUBLIC
	FILE_SET allModules TYPE CXX_MODULES FILES
	"voxel_game.ixx"
)

add_subdirectory("utilities/")
add_subdirectory("exceptions/")
add_subdirectory("core/")
add_subdirectory("typedefs/")
add_subdirectory("logging/")