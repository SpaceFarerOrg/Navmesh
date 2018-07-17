#include "Navmesh.h"
#include "SFML/Graphics/RenderWindow.hpp"
#include "Math.h"
#include "SFML/Graphics/Glsl.hpp"

void CNavmesh::Init()
{
	const float distanceFromEdge = 128;

	myVertices.reserve(1000*3);
	myEdges.reserve(1000*3);
	myTris.reserve(1000*3);

	myVertices.push_back({ 0 + distanceFromEdge, 0 + distanceFromEdge });
	myVertices.push_back({ 1600-distanceFromEdge, 0 + distanceFromEdge });
	myVertices.push_back({ 1600 - distanceFromEdge, 900 - distanceFromEdge });
	myVertices.push_back({ 0+ distanceFromEdge, 900 - distanceFromEdge });

	myEdges.push_back({ &myVertices[0],&myVertices[1] });
	myEdges.push_back({ &myVertices[1],&myVertices[2] });
	myEdges.push_back({ &myVertices[2],&myVertices[0] });
	myEdges.push_back({ &myVertices[0],&myVertices[3] });
	myEdges.push_back({ &myVertices[3],&myVertices[2] });

	myTris.push_back({ &myEdges[0], &myEdges[1], &myEdges[2] });
	myEdges[0].myOwnerTriangle[0] = &myTris[0];
	myEdges[1].myOwnerTriangle[0] = &myTris[0];
	myEdges[2].myOwnerTriangle[0] = &myTris[0];

	myTris.push_back({ &myEdges[3], &myEdges[4], &myEdges[2] });
	myEdges[3].myOwnerTriangle[0] = &myTris[1];
	myEdges[4].myOwnerTriangle[0] = &myTris[1];
	myEdges[2].myOwnerTriangle[1] = &myTris[1];


	myCircle.setFillColor(sf::Color::Red);
	myCircle.setRadius(8.f);
	myCircle.setOrigin(8.f, 8.f);

	myTriangleShader.loadFromFile("triangleShader.vfx", sf::Shader::Fragment);
}

void CNavmesh::AddNewEdge(const sf::Vector2f & aFrom, const sf::Vector2f & aTo)
{
	Math::SLineSegment line({ aFrom, aTo });
	std::vector<SEdge*> intersectedEdges = GetIntersectingEdgesWith(line);
	AddExtendedLineCollidingEdges(intersectedEdges, aTo, aFrom);

	for (SEdge*& edge : intersectedEdges)
	{
		SplitEdge(edge, edge->myIntersectionPoint);
	}

}

void CNavmesh::Render(sf::RenderWindow * aWindow)
{
	for (unsigned i = 0; i < myEdges.size(); ++i)
	{
		myLineDrawer.DrawLine(myEdges[i].myVertices[0]->myPosition, myEdges[i].myVertices[1]->myPosition);
		if (myEdges[i].myIsPlacedInFoundVector)
			myLineDrawer.DrawLine(myEdges[i].myVertices[0]->myPosition, myEdges[i].myVertices[1]->myPosition, {255, 0, 0, 255});
	}

	for (unsigned i = 0; i < myVertices.size(); ++i)
	{
		myCircle.setPosition(myVertices[i].myPosition);
		aWindow->draw(myCircle);
	}

	for (unsigned i = 0; i < myTris.size(); ++i)
	{
		sf::VertexArray triangle;
		triangle.append(myTris[i].myEdges[0]->myVertices[0]->myPosition);
		triangle.append(myTris[i].myEdges[1]->myVertices[0]->myPosition);
		triangle.append(myTris[i].myEdges[2]->myVertices[0]->myPosition);

		myTriangleShader.setUniform("color", sf::Glsl::Vec4(Math::RandomFloat(), Math::RandomFloat(), Math::RandomFloat(),1.f));
		aWindow->draw(&triangle[0], 3, sf::TrianglesStrip, &myTriangleShader);
	}
}

std::vector<CNavmesh::SEdge*> CNavmesh::GetIntersectingEdgesWith(Math::SLineSegment& aLine)
{
	std::vector<SEdge*> rv;
	for (int i = 0; i < myEdges.size(); ++i)
	{
		Math::SLineSegment line({ myEdges[i].myVertices[0]->myPosition, myEdges[i].myVertices[1]->myPosition });
		if (Math::CheckCollisionBetweenLines(line, aLine, myEdges[i].myIntersectionPoint))
		{
			rv.push_back(&myEdges[i]);
			myEdges[i].myIsPlacedInFoundVector = true;
		}
	}

	return rv;
}

void CNavmesh::AddExtendedLineCollidingEdges(std::vector<SEdge*>& aCurrentEdgesGotten, const sf::Vector2f& aTo, const sf::Vector2f& aFrom)
{
	STriangle* startTriangle = nullptr;
	STriangle* endTriangle = nullptr;
	
	//Loop through all edges collected (intersecting with line segment)
	for (unsigned i = 0; i < aCurrentEdgesGotten.size(); ++i)
	{
		//Check if owner triangle intersects with to or from.
		//If intersecting with to it is end triangle
		//Opposite for start triangle
		for (unsigned t = 0; t < 2; ++t)
		{
			if (aCurrentEdgesGotten[i]->myOwnerTriangle[t] == nullptr)
				continue;

			if (Math::CheckCollisionBetweenPointAndTriangle(aTo, *(aCurrentEdgesGotten[i]->myOwnerTriangle[t])))
			{
				endTriangle = aCurrentEdgesGotten[i]->myOwnerTriangle[t];
			}
			if (Math::CheckCollisionBetweenPointAndTriangle(aFrom, *(aCurrentEdgesGotten[i]->myOwnerTriangle[t])))
			{
				startTriangle = aCurrentEdgesGotten[i]->myOwnerTriangle[t];
			}
		}

		if (startTriangle != nullptr && endTriangle != nullptr)
		{
			break;
		}
	}

	Math::SLineSegment fakeRay;
	sf::Vector2f direction = aTo - aFrom;
	direction.x *= 1000.f;
	direction.y *= 1000.f;
	fakeRay.myTo = aTo + direction;
	fakeRay.myFrom = aFrom - direction;

	float k = (fakeRay.myTo.y - fakeRay.myFrom.y) / (fakeRay.myTo.x - fakeRay.myFrom.x);


	//Both triangulurings
	aCurrentEdgesGotten.push_back(nullptr);
	for (unsigned i = 0; i < 3; ++i)
	{
		if (startTriangle == nullptr)
			break;
	
		if (startTriangle->myEdges[i]->myIsPlacedInFoundVector == false)
		{
			if (Math::CheckCollisionBetweenLines(fakeRay, { startTriangle->myEdges[i]->myVertices[0]->myPosition, startTriangle->myEdges[i]->myVertices[1]->myPosition }, startTriangle->myEdges[i]->myIntersectionPoint))
			{
				aCurrentEdgesGotten.back() = startTriangle->myEdges[i];
				aCurrentEdgesGotten.back()->myIsPlacedInFoundVector = true;
				break;
			}
		}

	}

	aCurrentEdgesGotten.push_back(nullptr);
	for (unsigned i = 0; i < 3; ++i)
	{
		if (endTriangle == nullptr)
			break;

		if (endTriangle->myEdges[i]->myIsPlacedInFoundVector == false)
		{
			if (Math::CheckCollisionBetweenLines(fakeRay, { endTriangle->myEdges[i]->myVertices[0]->myPosition, endTriangle->myEdges[i]->myVertices[1]->myPosition }, endTriangle->myEdges[i]->myIntersectionPoint))
			{
				aCurrentEdgesGotten.back() = endTriangle->myEdges[i];
				aCurrentEdgesGotten.back()->myIsPlacedInFoundVector = true;
				break;
			}
		}
	}
}

void CNavmesh::SplitEdge(SEdge * aEdge, const sf::Vector2f & aSplitPos)
{
	sf::Vector2f previousStartPos = aEdge->myVertices[0]->myPosition;
	//aEdge->myVertices[0]->myPosition = aSplitPos;
	myVertices.push_back(SVertex(previousStartPos.x, previousStartPos.y));
	myVertices.push_back(SVertex(aSplitPos.x, aSplitPos.y));
	//myEdges.push_back(SEdge(&myVertices[myVertices.size() - 1], &myVertices[myVertices.size() - 2]));
}


