#pragma once
#include <vector>
#include "SFML/Graphics/CircleShape.hpp"
#include "Navmesh.h"
#include "LineDrawer.h"

namespace sf
{
	class RenderWindow;
}

class CGame
{
public:
	CGame(bool& aIsRunning);
	void Init();
	void Update();
	void Render();
private:

	CNavmesh myNavmesh;

	bool& myShouldRun;
	sf::CircleShape myCursor;
	sf::RenderWindow* myWindow;
	CLineDrawer myLineDrawer;

	struct SPlacement
	{
		sf::Vector2f myPosition;
		bool myIsActive = false;
	} myPlacements[2];
};