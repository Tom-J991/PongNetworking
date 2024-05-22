#include "Game.h"

int main()
{
	Game *game = new Game();
	game->Run();

	if (game)
		delete game;
	game = nullptr;

	return 0;
}