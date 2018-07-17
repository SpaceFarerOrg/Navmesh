#pragma once
#include "Navmesh.h"
#include "SFML/System/Vector2.hpp"

namespace Math
{
	struct SLineSegment
	{
		sf::Vector2f myFrom;
		sf::Vector2f myTo;
	};

	struct SLine
	{
		sf::Vector2f myPoint;
		sf::Vector2f myDirection;
	};


	struct SAABB
	{
		sf::Vector2f myTopLeft;
		sf::Vector2f myBottomRight;
	};


	static float Length(const sf::Vector2f aVector)
	{
		return sqrt(aVector.x * aVector.x + aVector.y * aVector.y);
	}

	static float Length2(const sf::Vector2f aVector)
	{
		return aVector.x * aVector.x + aVector.y * aVector.y;
	}

	static void Normalize(sf::Vector2f& aVectorToNormalize)
	{
		float length = Length(aVectorToNormalize);
		aVectorToNormalize.x /= length;
		aVectorToNormalize.y /= length;
	}

	static float Dot(const sf::Vector2f& aVector1, const sf::Vector2f& aVector2)
	{
		return aVector1.x * aVector2.x + aVector1.y * aVector2.y;
	}

	static bool CheckCollisionBetweenPointAndTriangle(const sf::Vector2f& aPoint, const CNavmesh::STriangle& aTriangle)
	{
		sf::Vector2f A = aTriangle.myEdges[0]->myVertices[0]->myPosition;
		sf::Vector2f B = aTriangle.myEdges[1]->myVertices[0]->myPosition;
		sf::Vector2f C = aTriangle.myEdges[2]->myVertices[0]->myPosition;
		sf::Vector2f P = aPoint;

		sf::Vector2f v0 = C - A;
		sf::Vector2f v1 = B - A;
		sf::Vector2f v2 = P - A;

		float dot00 = Dot(v0, v0);
		float dot01 = Dot(v0, v1);
		float dot02 = Dot(v0, v2);
		float dot11 = Dot(v1, v1);
		float dot12 = Dot(v1, v2);

		float invDenom = 1.f / (dot00 * dot11 - dot01 * dot01);
		float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
		float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

		return (u >= 0) && (v >= 0) && (u + v < 1);
	}

	static bool CheckCollisionBetweenAABBs(SAABB box1, SAABB box2)
	{
		return (box1.myTopLeft.x > box2.myBottomRight.x ||
			box1.myBottomRight.x < box2.myTopLeft.x ||
			box1.myTopLeft.y > box2.myBottomRight.y ||
			box1.myBottomRight.y < box2.myTopLeft.y);
	}

	static bool CheckCollisionBetweenLines(SLineSegment aLine1, SLineSegment aLine2)
	{
		const float& argument0 = aLine1.myFrom.x;
		const float& argument1 = aLine1.myFrom.y;
		const float& argument2 = aLine1.myTo.x;
		const float& argument3 = aLine1.myTo.y;
		const float& argument4 = aLine2.myFrom.x;
		const float& argument5 = aLine2.myFrom.y;
		const float& argument6 = aLine2.myTo.x;
		const float& argument7 = aLine2.myTo.y;

		float denominator = ((argument2 - argument0) * (argument7 - argument5)) - ((argument3 - argument1) * (argument6 - argument4));
		float numerator1 = ((argument1 - argument5) * (argument6 - argument4)) - ((argument0 - argument4) * (argument7 - argument5));
		float numerator2 = ((argument1 - argument5) * (argument2 - argument0)) - ((argument0 - argument4) * (argument3 - argument1));

		if (denominator == 0) { return (numerator1 == 0 && numerator2 == 0); }

		float r = numerator1 / denominator;
		float s = numerator2 / denominator;

		return ((r >= 0 && r <= 1) && (s >= 0 && s <= 1));
	}


}
