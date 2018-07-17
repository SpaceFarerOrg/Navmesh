#include "Game.h"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Window/Event.hpp"
#include "LineDrawer.h"

CGame::CGame(bool & aIsRunning)
	:myShouldRun(aIsRunning)
{
	myWindow = new sf::RenderWindow();
	
	sf::VideoMode vm;
	vm.width = 1600;
	vm.height = 900;

	myWindow->create(vm, "Navmesh Labb", sf::Style::Close | sf::Style::Titlebar);
	myCursor.setRadius(8.f);
	myCursor.setFillColor(sf::Color::Red);
	myCursor.setOrigin(8, 8);

	CLineDrawer::SetWindow(myWindow);

	myNavmesh.Init();
}

void CGame::Init()
{
}

void CGame::Update()
{
	sf::Event e;
	myWindow->pollEvent(e);

	myCursor.setPosition((float)sf::Mouse::getPosition(*myWindow).x, (float)sf::Mouse::getPosition(*myWindow).y);

	if (e.type == sf::Event::Closed)
		myShouldRun = false;
	else if (e.type == sf::Event::MouseButtonPressed)
	{
		if (e.mouseButton.button == sf::Mouse::Button::Left)
		{
			if (myPlacements[0].myIsActive == false)
			{
				myPlacements[0].myIsActive = true;
				myPlacements[0].myPosition = myCursor.getPosition();
			}
			else if (myPlacements[0].myIsActive == true)
			{
				myPlacements[1].myIsActive = true;
				myPlacements[1].myPosition = myCursor.getPosition();

				myNavmesh.AddNewEdge(myPlacements[0].myPosition, myPlacements[1].myPosition);
				myPlacements[0].myIsActive = false;
				myPlacements[1].myIsActive = false;
			}
		}
		else if (e.mouseButton.button == sf::Mouse::Button::Right)
		{
			myPlacements[0].myIsActive = false;
			myPlacements[1].myIsActive = false;
		}
	}

}

void CGame::Render()
{
	myWindow->clear({ 50,50,50,255 });

	sf::Vector2f originalCursorPos = myCursor.getPosition();

	myNavmesh.Render(myWindow);

	if (myPlacements[0].myIsActive)
	{
		myCursor.setPosition(myPlacements[0].myPosition);
		myWindow->draw(myCursor);
		myLineDrawer.DrawLine(myCursor.getPosition(), originalCursorPos, {200, 230, 255, 255});
	}

	myCursor.setPosition(originalCursorPos);
	myWindow->draw(myCursor);

	myWindow->display();
}
