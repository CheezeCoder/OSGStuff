#include <osgAnimation\AnimationManagerBase>
#include <osgAnimation\BasicAnimationManager>

struct AnimationManagerFinder : public osg::NodeVisitor
{
	osg::ref_ptr<osgAnimation::BasicAnimationManager> _am;
	AnimationManagerFinder();
	void apply(osg::Node& node);
};