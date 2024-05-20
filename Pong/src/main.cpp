#include <sstream>

#include "Game.h"
#include <raylib.h>

int main()
{
	// Create Window
	InitWindow(800, 600, "Pong Networking");
	SetTargetFPS(60);
	SetExitKey(NULL);

	// Game Loop
	auto& game = Game::Get();
	std::stringstream* ss = new std::stringstream;
	while (!(WindowShouldClose() || game.IsQuitting()))
	{
		game.Update(GetFrameTime());
		game.Draw();
	}

	// Close
	game.Destroy();
	CloseWindow();

	return 0;
}