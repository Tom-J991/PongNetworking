#include "Game.h"

#include "shared.h"

Game* Game::m_instance = nullptr;
Game::Game()
{
	InitAudioDevice();

	const int monitor = GetCurrentMonitor();
	const int mWidth = GetMonitorWidth(monitor);
	const int mHeight = GetMonitorHeight(monitor);
	SetWindowPosition(mWidth/2 - GetScreenWidth()/2, mHeight/2 - GetScreenHeight()/2);

	m_pong = std::unique_ptr<PongState>(new PongState());
	m_pong->Init();
}
Game::~Game()
{
	m_pong->Shutdown();
	CloseAudioDevice();
}

void Game::Update(float deltaTime)
{
	m_pong->Update(deltaTime);
}

void Game::Draw()
{
	m_pong->Draw();
}
