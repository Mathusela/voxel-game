target_sources(modules PUBLIC
	FILE_SET allModules TYPE CXX_MODULES FILES
	"rendering.ixx"
	"rendering_backend.ixx"
	"opengl_backend.ixx"
	"rendering_context.ixx"
	"window_manager.ixx"
	"structs.ixx"
)