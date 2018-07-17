#include "LineDrawer.h"
#include "SFML/Graphics/RenderWindow.hpp"

sf::RenderWindow* CLineDrawer::ourWindow;


CLineDrawer::CLineDrawer()
{
	myLineVisRep = sf::VertexArray(sf::LineStrip, 2);
}

void CLineDrawer::SetWindow(sf::RenderWindow * aWindow)
{
	ourWindow = aWindow;
}

void CLineDrawer::DrawLine(const sf::Vector2f & aFrom, const sf::Vector2f & aTo, sf::Color aColor)
{
	myLineVisRep[0].color = aColor;
	myLineVisRep[0].position = aFrom;
	myLineVisRep[1].color = aColor;
	myLineVisRep[1].position = aTo;

	ourWindow->draw(myLineVisRep);
}
