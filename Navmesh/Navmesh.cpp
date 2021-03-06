#include "Navmesh.h"
#include "SFML/Graphics/RenderWindow.hpp"
#include "Math.h"
#include "SFML/Graphics/Glsl.hpp"

void CNavmesh::Init()
{
	const float distanceFromEdge = 128;

	myVertices.reserve(1000 * 3);
	myEdges.reserve(1000 * 3);
	myTris.reserve(1000 * 3);

	myVertices.push_back({ 0 + distanceFromEdge, 0 + distanceFromEdge });
	myVertices.push_back({ 1600 - distanceFromEdge, 0 + distanceFromEdge });
	myVertices.push_back({ 1600 - distanceFromEdge, 900 - distanceFromEdge });
	myVertices.push_back({ 0 + distanceFromEdge, 900 - distanceFromEdge });

	myEdges.push_back({ &myVertices[0],&myVertices[1] });
	myEdges.push_back({ &myVertices[1],&myVertices[2] });
	myEdges.push_back({ &myVertices[2],&myVertices[0] });
	myEdges.push_back({ &myVertices[0],&myVertices[3] });
	myEdges.push_back({ &myVertices[3],&myVertices[2] });

	myTris.push_back({ &myEdges[0], &myEdges[1], &myEdges[2] });
	myEdges[0].myOwnerTriangle[0] = &myTris[0];
	myEdges[1].myOwnerTriangle[0] = &myTris[0];
	myEdges[2].myOwnerTriangle[0] = &myTris[0];
	myTris.back().myColor = sf::Color(Math::RandomFloat(), Math::RandomFloat(), Math::RandomFloat(), 1.0f);

	myTris.push_back({ &myEdges[3], &myEdges[4], &myEdges[2] });
	myEdges[3].myOwnerTriangle[0] = &myTris[1];
	myEdges[4].myOwnerTriangle[0] = &myTris[1];
	myEdges[2].myOwnerTriangle[1] = &myTris[1];
	myTris.back().myColor = sf::Color(Math::RandomFloat(), Math::RandomFloat(), Math::RandomFloat(), 1.0f);

	myCircle.setFillColor(sf::Color::Red);
	myCircle.setRadius(8.f);
	myCircle.setOrigin(8.f, 8.f);

	myTriangleShader.loadFromFile("triangleShader.vfx", sf::Shader::Fragment);
	myFont.loadFromFile("debug.ttf");
	myText.setFont(myFont);
	myText.setCharacterSize(16.f);
}

void CNavmesh::AddNewEdge(const sf::Vector2f & aFrom, const sf::Vector2f & aTo)
{
	Math::SLineSegment line({ aFrom, aTo });
	std::vector<SEdge*> intersectedEdges = GetIntersectingEdgesWith(line);
	AddExtendedLineCollidingEdges(intersectedEdges, aTo, aFrom);

	for (SEdge*& edge : intersectedEdges)
	{
		if (edge != nullptr)
		{
			SplitEdge(edge, edge->myIntersectionPoint);
		}
	}

}

void CNavmesh::Render(sf::RenderWindow * aWindow)
{
	int triCount = 0, verCount = 0, edgCount = 0;
	for (unsigned i = 0; i < myTris.size(); ++i)
	{
		if (myTris[i].myIsActive == false || myTris[i].myEdges[0] == nullptr || myTris[i].myEdges[1] == nullptr || myTris[i].myEdges[2] == nullptr)
			continue;

		++triCount;

		sf::VertexArray triangle;
		triangle.append(myTris[i].myEdges[0]->myVertices[0]->myPosition);
		triangle.append(myTris[i].myEdges[1]->myVertices[0]->myPosition);
		triangle.append(myTris[i].myEdges[2]->myVertices[0]->myPosition);

		myTriangleShader.setUniform("color", sf::Glsl::Vec4(myTris[i].myColor.r / 255.f, myTris[i].myColor.g / 255.f, myTris[i].myColor.b / 255.f, 1.0f));
		aWindow->draw(&triangle[0], 3, sf::TrianglesStrip, &myTriangleShader);
	}

	for (unsigned i = 0; i < myEdges.size(); ++i)
	{
		if (myEdges[i].myIsActive)
		{
			++edgCount;
			myLineDrawer.DrawLine(myEdges[i].myVertices[0]->myPosition, myEdges[i].myVertices[1]->myPosition);
			if (myEdges[i].myIsPlacedInFoundVector)
				myLineDrawer.DrawLine(myEdges[i].myVertices[0]->myPosition, myEdges[i].myVertices[1]->myPosition, { 255, 0, 0, 255 });
		}
	}

	for (unsigned i = 0; i < myVertices.size(); ++i)
	{
		++verCount;
		myCircle.setPosition(myVertices[i].myPosition);
		aWindow->draw(myCircle);
	}

	myText.setPosition(10.f, 10.f);
	myText.setString("Tris: " + std::to_string(triCount));
	aWindow->draw(myText);

	myText.setPosition(10.f, 30.f);
	myText.setString("Vertices: " + std::to_string(verCount));
	aWindow->draw(myText);

	myText.setPosition(10.f, 50.f);
	myText.setString("Edges: " + std::to_string(edgCount));
	aWindow->draw(myText);
}

std::vector<CNavmesh::SEdge*> CNavmesh::GetIntersectingEdgesWith(Math::SLineSegment& aLine)
{
	std::vector<SEdge*> rv;
	for (int i = 0; i < myEdges.size(); ++i)
	{
		if (myEdges[i].myIsActive)
		{
			Math::SLineSegment line({ myEdges[i].myVertices[0]->myPosition, myEdges[i].myVertices[1]->myPosition });
			if (Math::CheckCollisionBetweenLines(line, aLine, myEdges[i].myIntersectionPoint))
			{
				rv.push_back(&myEdges[i]);
				myEdges[i].myShouldSplit = true;
				myEdges[i].myIsPlacedInFoundVector = true;
			}
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
				startTriangle->myEdges[i]->myShouldSplit = true;
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
				endTriangle->myEdges[i]->myShouldSplit = true;
				aCurrentEdgesGotten.back()->myIsPlacedInFoundVector = true;
				break;
			}
		}
	}
}

void CNavmesh::SplitEdge(SEdge * aEdge, const sf::Vector2f & aSplitPos)
{
	aEdge->myIsActive = false;
	sf::Vector2f previousStartPos = aEdge->myVertices[0]->myPosition;
	//aEdge->myVertices[0]->myPosition = aSplitPos;

	//myVertices.push_back(SVertex(previousStartPos.x, previousStartPos.y));
	myVertices.push_back(SVertex(aSplitPos.x, aSplitPos.y));
	//myEdges.push_back(SEdge(&myVertices[myVertices.size() - 1], &myVertices[myVertices.size() - 2]));

	myEdges.push_back(SEdge(&myVertices.back(), aEdge->myVertices[0]));
	myEdges.push_back(SEdge(&myVertices.back(), aEdge->myVertices[1]));

	for (STriangle* tri : aEdge->myOwnerTriangle)
	{
		if (tri == nullptr)
		{
			continue;
		}

		SVertex* vertex = nullptr;
		for (SEdge* edge : tri->myEdges)
		{
			if (edge == nullptr)
				continue;

			if (edge->myVertices[0]->myPosition != aEdge->myVertices[0]->myPosition && edge->myVertices[0]->myPosition != aEdge->myVertices[1]->myPosition)
			{
				vertex = edge->myVertices[0];
			}
		}
		
		if (vertex != nullptr)
		{
			myEdges.push_back(SEdge(vertex, &myVertices.back()));

			for (int i = 0; i < 2; ++i)
			{
				myTris.push_back(STriangle(nullptr, nullptr, nullptr));
				STriangle& newTri = myTris.back();

				newTri.myEdges[0] = &myEdges[myEdges.size() - 2 - i];
				newTri.myEdges[1] = &myEdges.back();
				for (SEdge* edge : tri->myEdges)
				{
					if (edge->myVertices[0] == myEdges.back().myVertices[0] || edge->myVertices[1] == myEdges.back().myVertices[0] || edge->myVertices[0] == myEdges.back().myVertices[1] || edge->myVertices[1] == myEdges.back().myVertices[1])
					{
						if (edge->myVertices[0] == myEdges[myEdges.size() - 2 - i].myVertices[0] || edge->myVertices[1] == myEdges[myEdges.size() - 2 - i].myVertices[0] || edge->myVertices[0] == myEdges[myEdges.size() - 2 - i].myVertices[1] || edge->myVertices[1] == myEdges[myEdges.size() - 2 - i].myVertices[1])
						{
							newTri.myEdges[2] = edge;

							newTri.myEdges[0]->myOwnerTriangle[0] = &myTris.back();
							newTri.myEdges[1]->myOwnerTriangle[0] = &myTris.back();
							newTri.myEdges[2]->myOwnerTriangle[0] = &myTris.back();
						}
					}
				}
				
				myTris.back().myColor = sf::Color(Math::RandomFloat(), Math::RandomFloat(), Math::RandomFloat(), 255);

			}
			
			//tri->myEdges[0] = nullptr;
			//tri->myEdges[1] = nullptr;
			//tri->myEdges[2] = nullptr;
			tri->myIsActive = false;
		}
	}

	//Find triangle that has no should split




}

void CNavmesh::RebindEdgesToVertices(SEdge * aOldEdge, std::array<SEdge*, 2>& aNewEdges)
{
	for (size_t newEdge = 0; newEdge < aNewEdges.size(); ++newEdge)
	{
		for (size_t triangleInEdge = 0; triangleInEdge < 2; ++triangleInEdge)
		{
			STriangle* tri = aOldEdge->myOwnerTriangle[triangleInEdge];

			for (size_t edgeInTri = 0; edgeInTri < tri->myEdges.size(); ++edgeInTri)
			{
			}
		}
	}
}


