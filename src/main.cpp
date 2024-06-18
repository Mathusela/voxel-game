import voxel_game.core;

int main() {
	constexpr vxg::core::App::WindowProperties windowProperties{ {700, 500}, "Voxel Game" };
	
	vxg::core::App app;
	return app.run(windowProperties);
}