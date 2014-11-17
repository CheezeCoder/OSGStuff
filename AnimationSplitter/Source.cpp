//
//  Takes a given FBX file and converts it to an osgb file with the given animation information.
//

#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include "AnimationSplitter.h"
#include "tinystr.h"
#include "tinyxml.h"
#include "OsgXml.h"

/**
* Builds the animation property
*
* @param (int k) = Assign a unique key to locate the specific animation
* @param (int s) = The starting frame of this particular animation
* @param (int f) = The end frame of this particular animation
* @param (const char * n) = The name of the animation you wish to refer to in your OSG application.
*
* @return A new animation property to add to the list of animations to bake into your new .osgb file.
*/
animations newAnim(int k, int s, int f, const char* n)
{
	animations * a	= new animations();
	a->key			= k;
	a->start		= s;
	a->finish		= f;
	a->name			= n;

	return *a;
}

/**
* Creates a new .osgb file with your animations baked into it.
* In order to add an animation just push a new animation object into the anims list.  
* You can either do this by directly instanciating the object in the paramater of the
* push_back() method or by creating individual animation objects and adding them to the
* anims list.  You must give a unique ID, a start frame, and end frame and the name you
* wish to find the animation by in your OSG application as arguments for the newAnim method,
* in the given order.  Then instanciate OsgXML which builds the xml file that the animationSplitter
* (@courtesy of tomhog), class will use to create your new .osgb file.  
*
* For example a new Animation could look like: anims.push_back(newAnim(0, 0, 25, "Jump");
* This animation would have a unique id of 0, The animation begins on frame 0 and ends on
* frame 25.  The name for the animation is jump and will now be available through the 
* animation manager in your OSG application via getAnimationsList() and getName().
*
*
* Pass in the list of animations and your fbx file path, the new path and name for your new .osgb file
* and the fps.  FPS defaults to 30 if not provided.  
*
* Finally instanciate the AnimationSplitter class with the xml file as a paramater and the program
* will create a new .osgb file with your proper animations. 
*/
int main(int argc, char** argv)
{

	std::list<animations> anims;

	//Example: 

	//anims.push_back(newAnim(0, 38, 76, "Idle"));
	//anims.push_back(newAnim(1, 0, 36, "Run"));

	//or

	//animations run = newAnim(0, 80, 102, "Walk");
	//anims.push_back(run);


	//NOTE:  I haven't written checks in so if you give a false path name or file name 
	//it won't work properly. Make sure your file paths and names are good. 
	OsgXml xml(&anims, "ExportToOSG.fbx.", "TutorialModel.osgb", "30");

	AnimationSplitter spiltter("./xml/XmlParser.xml");
	return 0;
}