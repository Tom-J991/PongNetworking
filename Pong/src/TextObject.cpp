#include "TextObject.h"

#include <iostream>

void TextObject::Init(std::string text, float x, float y, float lifeTime, int fontSize, Color color)
{
	this->text = text;
	this->xPosition = x;
	this->yPosition = y;
	this->lifeTime = lifeTime;
	this->fontSize = fontSize;
	this->color = color;

	startingLife = lifeTime;
}

void TextObject::Animate(double deltaTime)
{
	if (!InUse())
		return;

	color.a = (unsigned char)((lifeTime / startingLife) * 255.0f); // Set alpha over life time.

	lifeTime -= (float)deltaTime; // Decrease life.
	if (lifeTime < 0.0f)
		lifeTime = 0.0f;
}

void TextObject::Draw()
{
	if (!InUse())
		return;

	DrawText(text.c_str(), (int)xPosition, (int)yPosition, fontSize, color);
}

void TextObjectPool::Init(std::string text, float x, float y, float lifeTime, int fontSize, Color color)
{
	for (TextObject &object : m_pool)
	{
		if (object.InUse()) // Don't reinit an object whilst in use.
			continue;

		object.Init(text, x, y, lifeTime, fontSize, color);
		break; // Init the first available.
	}
}

void TextObjectPool::Animate(double deltaTime)
{
	for (TextObject &object : m_pool)
	{
		object.Animate(deltaTime);
	}
}

void TextObjectPool::Draw()
{
	for (TextObject &object : m_pool)
	{
		object.Draw();
	}
}
