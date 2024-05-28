#pragma once

#include <string>

#include <raylib.h>

struct TextObject
{
	std::string text;
	Color color = WHITE;
	int fontSize = 24;

	float xPosition = 0.5f;
	float yPosition = 0.5f;
	float lifeTime = 0.0f;

	float startingLife = 0.0f;

	inline bool InUse() { return lifeTime > 0.0f; }

	void Init(std::string text, float x, float y, float lifeTime, int fontSize, Color color);
	void Animate(double deltaTime);
	void Draw();
};

class TextObjectPool
{
public:
	TextObjectPool() = default;
	~TextObjectPool() = default;

	void Init(std::string text, float x, float y, float lifeTime, int fontSize = 24, Color color = WHITE);
	void Animate(double deltaTime);
	void Draw();

private:
	constexpr static unsigned int MAX_SIZE = 8;
	TextObject m_pool[MAX_SIZE];

};
