#include <stdio.h>

#include "engine/app.hpp"
#include "game_layer.hpp"
#include "arkanoid/breakanoid_layer.hpp"

int main (int argc, char *argv[]) {
	engine::App app({
		.window = {
			.width = 1280,
			.height = 720,
			.title = "OpenGL Engine"
		}
		});

	// Use this to test the engine itself
#if 0
	GameLayer* gameLayer = new GameLayer(app.getDeviceManager(), app.getAssetManager());
	app.pushLayer(gameLayer);
#endif

	BreakanoidLayer* breakanoidLayer = new BreakanoidLayer(app.getDeviceManager(), app.getAssetManager());
	app.pushLayer(breakanoidLayer);

	app.run();
}