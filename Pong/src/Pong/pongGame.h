#pragma once

#include <vector>
#include <memory>

#include "pongBall.h"
#include "pongPaddle.h"

#include <raylib.h>
#include <raymath.h>

enum class eSounds
{
	BOUNDS = 0,
	BOUNCE,
	SCORE,
	SOUNDS_MAX
};

class PongState
{
public:
	PongState();
	~PongState();

	void Init();
	void Shutdown();

	bool Update(float deltaTime);
	void Draw();

	void SetOnePlayer(const bool onePlayer) { m_onePlayer = onePlayer; }
		
private:
	void PlaySFX(eSounds sound);

private:
	std::unique_ptr<Ball> m_ball;
	std::unique_ptr<Paddle> m_firstPlayer;
	std::unique_ptr<Paddle> m_secondPlayer;

	bool m_onePlayer;

	std::vector<Sound> m_loadedSounds;

};
