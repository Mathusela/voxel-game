target_sources(modules PUBLIC
	FILE_SET allModules TYPE CXX_MODULES FILES
	"core.ixx"
	"app.ixx"
)

add_subdirectory("rendering/")
add_subdirectory("memory/")
add_subdirectory("logic/")
add_subdirectory("structs/")