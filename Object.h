#pragma once

// HEADERS:
#include <TL-Engine.h>

using namespace tle;

// CLASS:
class Object
{
	// VARAIBLES:
	IMesh* mesh;
	IModel* model;
	float height;
	float width;
	float length;

	// PUBLIC FUNCTIONS:
	public:
	Object();
	~Object();
};