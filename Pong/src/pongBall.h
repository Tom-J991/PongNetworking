#pragma once

#include <raylib.h>
#include <raymath.h>

class Paddle; // Forward declare

class Ball
{
public:
	Ball();
	~Ball();

	void Init();

	void Move(double deltaTime);
	void Draw();

	bool CheckBounds(double deltaTime);
	bool CheckCollision(Paddle &paddle, double deltaTime);

	Vector2 Position() const { return m_pos; }
	void SetPosition(Vector2 pos) { m_pos = pos; }
	void SetPosition(float x, float y) { m_pos = { x, y }; }

	Vector2 Velocity() const { return m_vel; }
	void SetVelocity(Vector2 vel) { m_vel = vel; }
	void SetVelocity(float x, float y) { m_vel = { x, y }; }

	Vector2 Size() const { return m_size; }
	void SetSize(Vector2 size) { m_size = size; }
	void SetSize(float x, float y) { m_size = { x, y }; }

	float Speed() const { return m_speed; }
	void SetSpeed(float speed) { m_speed = speed; }

private:
	bool AABB(Vector2 pos, Vector2 size, Paddle &paddle);
	float CalculateStartingAngle();

private:
	Vector2 m_pos;
	Vector2 m_vel;
	Vector2 m_size;
	float m_speed;

};
