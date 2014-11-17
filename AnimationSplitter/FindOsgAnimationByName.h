#include <osgAnimation\AnimationManagerBase>


//
//Finds an Animation by name, returning a pointer to the animation
//and a pointer to the manager it was found in
struct FindOsgAnimationByName : public osg::NodeVisitor
{
	//the name of the source animation we are looking for
	std::string _sourceAnimationName;

	//used to return the animation and the manager it was stored in
	osgAnimation::Animation* p_ani;
	osgAnimation::AnimationManagerBase* p_manager;

	FindOsgAnimationByName(std::string sourceAnimationName);

	void apply(osg::Node& node);
};
