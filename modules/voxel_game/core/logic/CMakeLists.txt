target_sources(modules PUBLIC
	FILE_SET allModules TYPE CXX_MODULES FILES
	"logic.ixx"
)

add_subdirectory("camera/")