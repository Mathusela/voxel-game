target_sources(modules PUBLIC
	FILE_SET allModules TYPE CXX_MODULES FILES
	"memory.ixx"
	"gpu_allocator.ixx"
	"opengl_allocator.ixx"
)