#pragma once
#include "tinystr.h"
#include "tinyxml.h"
#include <list>

struct
{
	int start;
	int key;
	int finish;
	const char * name;
} typedef animations;

class OsgXml
{
public:
	OsgXml(std::list<animations> * a, const char* s = "MyModel.fbx", const char * d = "MyModel.osgb", const char * f = "30");
	~OsgXml();
private:
	void createXML();
	std::list<animations> * anims;
	const char* source;
	const char* destination;
	const char* fps;
};



