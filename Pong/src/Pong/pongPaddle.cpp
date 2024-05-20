#include "pongPaddle.h"

#include "pongBall.h"

Paddle::Paddle(int player)
	: m_player{ player }
	, m_pos{ 0, 0 }
	, m_size{ 0, 0 }
	, m_speed{ 0, 0 }
	, m_score{ 0 }
{ }
Paddle::~Paddle()
{ }

void Paddle::Init()
{
	if (m_player < 1 || m_player > 2) // Is neither player 1 or 2.
		return;

	m_size = { 12, 96 };
	m_speed.y = 256;

	m_score = 0;

	const int paddleOffset = 64;
	if (m_player == 1)
	{
		m_pos = { (float)paddleOffset, (float)GetScreenHeight() / 2 };
		m_upKey = KEY_W;
		m_downKey = KEY_S;
	}
	else if (m_player == 2)
	{
		m_pos = { (float)GetScreenWidth() - paddleOffset, (float)GetScreenHeight() / 2 };
		m_upKey = KEY_UP;
		m_downKey = KEY_DOWN;
	}
}

void Paddle::Move(Ball &ball, float deltaTime)
{
	// Get Input.
	int move = IsKeyDown(m_downKey) - IsKeyDown(m_upKey);

	// Move.
	m_pos.y += move * m_speed.y * deltaTime;

	// Clamp Paddle to bounds.
	if (m_pos.y - (m_size.y / 2) < 0)
		m_pos.y = (m_size.y / 2);
	if (m_pos.y + (m_size.y / 2) > GetScreenHeight())
		m_pos.y = GetScreenHeight() - (m_size.y / 2);
}

void Paddle::Draw()
{
	const Vector2 center = { m_pos.x-(m_size.x/2), m_pos.y-(m_size.y/2) };
	DrawRectangle((int)center.x, (int)center.y, (int)m_size.x, (int)m_size.y, WHITE);
}
