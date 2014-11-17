#include "OsgXml.h"


OsgXml::OsgXml(std::list<animations> * a, const char* s, const char * d, const char * f ) :
anims(a), source(s), destination(d), fps(f)
{
	createXML();
}


OsgXml::~OsgXml()
{
}

void OsgXml::createXML()
{
	TiXmlDocument doc;

	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "IS0-8859-15", "");
	doc.LinkEndChild(decl);

	TiXmlElement * info = new TiXmlElement("OsgAnimationTools");
	info->SetAttribute("sourceFile", source);
	info->SetAttribute("destinationFile", destination);
	info->SetAttribute("fps", fps);
	doc.LinkEndChild(info);

	TiXmlElement * root = new TiXmlElement("AnimationSplit");
	root->SetAttribute("sourceAnimation", "Take 001");
	info->LinkEndChild(root);

	for each (animations a in *anims)
	{
		TiXmlElement * newAnimation = new TiXmlElement("NewAnimation");
		newAnimation->SetAttribute("name", a.name);
		newAnimation->SetAttribute("startFrame", a.start);
		newAnimation->SetAttribute("endFrame", a.finish);
		root->LinkEndChild(newAnimation);
	}


	doc.SaveFile("./xml/XmlParser.xml");
}
