#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <raylib.h>
#include <raymath.h>

#include <BCNet/IBCNetClient.h>
#include <BCNet/BCNetPacket.h>

#include "pongBall.h"
#include "pongPaddle.h"

enum class eSounds
{
	BOUNDS = 0,
	BOUNCE,
	SCORE,
	SOUNDS_MAX
};

class Game
{
public:
	Game();
	~Game();

	void Run();

private:
	void Init();
	void Shutdown();

	void Update(double deltaTime = 0.0f);
	void Draw();

	void PlaySFX(eSounds sound);

	void PacketReceived(const BCNet::Packet packet);

private:
	bool m_running = false;

	BCNet::IBCNetClient *m_netClient;

	std::vector<Sound> m_loadedSounds;

	std::unique_ptr<Ball> m_ball;
	std::unique_ptr<Paddle> m_firstPlayer;
	std::unique_ptr<Paddle> m_secondPlayer;

};
