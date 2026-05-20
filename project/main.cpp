#include "Game.h"
#include "Framework.h"

// Windowsアプリでのエントリーポイント
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	std::unique_ptr<Framework> game = std::make_unique<Game>();

	game->Run();

	game.reset();

	return 0;
}


