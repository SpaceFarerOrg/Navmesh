#pragma once
#include "SFML/System/Vector2.hpp"
#include <vector>
#include <array>
#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/VertexArray.hpp"
#include "LineDrawer.h"
#include "SFML/Graphics/Shader.hpp"

#include "SFML/Graphics/Text.hpp"

#define VERTEX_COUNT 2
#define EDGE_COUNT 3

namespace Math
{
	struct SLineSegment;
}

namespace sf
{
	class RenderWindow;
}

class CNavmesh
{
public:
	struct STriangle;

	struct SVertex
	{
		SVertex(float x, float y) : myPosition(x, y) {};
		sf::Vector2f myPosition;
	};

	struct SEdge
	{
		SEdge(SVertex* aOne, SVertex* aTwo) 
		{
			myVertices[0] = aOne;
			myVertices[1] = aTwo;
		}
		std::array<SVertex*, VERTEX_COUNT> myVertices;
		std::array<STriangle*, 2> myOwnerTriangle = { nullptr, nullptr };

		sf::Vector2f myIntersectionPoint;

		bool myIsPlacedInFoundVector = false;
	};

	struct STriangle
	{
		STriangle(SEdge* aOne, SEdge* aTwo, SEdge* aThree) 
		{ 
			myEdges[0] = aOne;
			myEdges[1] = aTwo;
			myEdges[2] = aThree;
		}

		bool operator==(const STriangle& aOther)
		{
			return myEdges[0] == aOther.myEdges[0] && myEdges[1] == aOther.myEdges[1] && myEdges[2] == aOther.myEdges[2];
		}

		std::array<SEdge*, EDGE_COUNT> myEdges;
		sf::Color myColor;
	};

public:
	void Init();
	void AddNewEdge(const sf::Vector2f& aFrom,const sf::Vector2f& aTo);
	void Render(sf::RenderWindow* aWindow);
private:
	std::vector<SEdge*> GetIntersectingEdgesWith(Math::SLineSegment& aLine);
	void AddExtendedLineCollidingEdges(std::vector<SEdge*>& aCurrentEdgesGotten, const sf::Vector2f& aTo, const sf::Vector2f& aFrom);

	void SplitEdge(SEdge* aEdge, const sf::Vector2f& aSplitPos);
	void RebindEdgesToVertices(SEdge* aOldEdge, std::array<SEdge*, 2>& aNewEdges);

	CLineDrawer myLineDrawer;

	std::vector<STriangle> myTris;
	std::vector<SEdge> myEdges;
	std::vector<SVertex> myVertices;

	sf::CircleShape myCircle;

	sf::Shader myTriangleShader;
	sf::Font myFont;
	sf::Text myText;
	
};