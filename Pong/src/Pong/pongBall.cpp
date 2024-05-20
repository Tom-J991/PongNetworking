#include "pongBall.h"

#include "pongPaddle.h"

template <typename T> int sgn(T val) { // Sign function. Returns -1 for negative numbers or 1 for positive numbers.
	return (T(0) < val) - (val < T(0));
}

Ball::Ball()
	: m_pos{ 0, 0 }
	, m_vel{ 0, 0 }
	, m_size{ 0, 0 }
	, m_speed{ 0 }
{ }
Ball::~Ball()
{ }

void Ball::Init()
{
	m_pos = { (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 };
	m_vel = { -1, -1 };
	m_size = { 10, 10 };
	m_speed = 256;

	float angle = CalculateStartingAngle();
	m_vel.x = m_speed * cosf(angle);
	m_vel.y = m_speed * sinf(angle);
}

void Ball::Move(float deltaTime)
{
	// Move.
	m_pos.x += m_vel.x * deltaTime;
	m_pos.y += m_vel.y * deltaTime;
}

void Ball::Draw()
{
	const Vector2 center = { m_pos.x-(m_size.x/2), m_pos.y-(m_size.y/2) };
	DrawRectangle((int)center.x, (int)center.y, (int)m_size.x, (int)m_size.y, WHITE);
}

bool Ball::CheckBounds(float deltaTime)
{
	if (m_pos.y < 0 || m_pos.y > GetScreenHeight()) // Flip y direction if hit ceiling or floor.
	{
		m_vel.y *= -1;
		return true;
	}
	return false;
}

bool Ball::CheckCollision(Paddle &paddle, float deltaTime)
{
	bool collided = false;
	// X Axis
	if (AABB({m_pos.x + m_vel.x * deltaTime, m_pos.y}, m_size, paddle))
	{
		while (!AABB({m_pos.x + sgn(m_vel.x) * deltaTime, m_pos.y}, m_size, paddle))
			m_pos.x += sgn(m_vel.x) * deltaTime; // Push back out
		collided = true;
	}
	// Y Axis
	if (AABB({ m_pos.x, m_pos.y + m_vel.y * deltaTime }, m_size, paddle))
	{
		while (!AABB({ m_pos.x, m_pos.y + sgn(m_vel.y) * deltaTime }, m_size, paddle))
			m_pos.y += sgn(m_vel.y) * deltaTime; // Push back out
		collided = true;
	}
	// Bounce
	if (collided)
	{
		// Calculate bounce angle
		float intersectY = paddle.Position().y - m_pos.y; // How far from the middle of the paddle did the ball hit?
		float normalizedIntersect = intersectY / (paddle.Size().y/2); // -1 <-> 1
		float bounceAngle = normalizedIntersect * (45 * DEG2RAD);
		m_speed = 256 + abs(normalizedIntersect) * 256; // 256 min speed, 512 max speed
			
		if (paddle.Player() == 1) // Left Paddle
		{
			m_vel.x = m_speed * cosf(bounceAngle);
			m_vel.y = m_speed * -sinf(bounceAngle);
		}
		else if (paddle.Player() == 2) // Right Paddle
		{
			m_vel.x = m_speed * -cosf(bounceAngle);
			m_vel.y = m_speed * sinf(bounceAngle);
		}
	}

	return collided;
}

bool Ball::AABB(Vector2 pos, Vector2 size, Paddle &paddle)
{
	bool collX = pos.x - (size.x/2) <= paddle.Position().x + (paddle.Size().x/2) &&
				pos.x + (size.x/2) >= paddle.Position().x - (paddle.Size().x/2);
	bool collY = pos.y - (size.y/2) <= paddle.Position().y + (paddle.Size().y/2) &&
				pos.y + (size.y/2) >= paddle.Position().y - (paddle.Size().y/2);
	m_speed *= 1.1f;
	return collX && collY;
}

float Ball::CalculateStartingAngle()
{
	float angle = (float)GetRandomValue(0, (int)(360 * DEG2RAD));
	if (angle == 0 || angle == (90 * DEG2RAD) || angle == (180 * DEG2RAD) || angle == (270 * DEG2RAD) || angle == (360 * DEG2RAD))
		return CalculateStartingAngle();
	//float angle = (PI);
	return angle;
}
