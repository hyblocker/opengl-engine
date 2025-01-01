#include <stdio.h>

#include "engine/app.hpp"
#include "game_layer.hpp"

int main (int argc, char *argv[]) {
	engine::App app({
		.window = {
			.width = 1280,
			.height = 720,
			.title = "OpenGL Engine"
		}
		});

	GameLayer* gameLayer = new GameLayer(app.getDeviceManager());
	app.pushLayer(gameLayer);

	app.run();
}