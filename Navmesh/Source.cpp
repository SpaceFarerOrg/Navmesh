#include "Game.h"

#ifdef _DEBUG
#pragma comment(lib, "sfml-system-d")
#pragma comment(lib, "sfml-window-d")
#pragma comment(lib, "sfml-graphics-d")
#else
#pragma comment(lib, "sfml-system")
#pragma comment(lib, "sfml-window")
#pragma comment(lib, "sfml-graphics")
#endif

int main()
{
	bool isRunning = true;

	CGame game(isRunning);
	game.Init();

	while (isRunning)
	{
		game.Update();
		game.Render();
	}

	return 0;
}