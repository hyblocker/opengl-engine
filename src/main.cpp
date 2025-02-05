#include <stdio.h>

#include "engine/app.hpp"
#include "game_layer.hpp"
#include "arkanoid/arkanoid_layer.hpp"

int main (int argc, char *argv[]) {
	engine::App app({
		.window = {
			.width = 1920,
			.height = 1080,
			.title = "Breakanoid"
		}
		});

	// Use this to test the engine itself
#if 0
	GameLayer* gameLayer = new GameLayer(app.getDeviceManager(), app.getAssetManager());
	app.pushLayer(gameLayer);
#endif

	// actual game
	ArkanoidLayer* arkanoidLayer = new ArkanoidLayer(app.getDeviceManager(), app.getAssetManager());
	app.pushLayer(arkanoidLayer);

	app.run();
}