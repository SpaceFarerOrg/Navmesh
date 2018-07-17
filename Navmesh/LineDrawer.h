#pragma once
#include "SFML/System/Vector2.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/VertexArray.hpp"

namespace sf
{
	class RenderWindow;
}

class CLineDrawer
{
public:
	CLineDrawer();

	static void SetWindow(sf::RenderWindow* aWindow);

	void DrawLine(const sf::Vector2f& aFrom, const sf::Vector2f& aTo, sf::Color aColor = { 0,255,0,255 });
private:
	static sf::RenderWindow* ourWindow;

	sf::VertexArray myLineVisRep;
};