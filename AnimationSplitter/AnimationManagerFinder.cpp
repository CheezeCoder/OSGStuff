#include "AnimationManagerFinder.h"


AnimationManagerFinder::AnimationManagerFinder() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) 
{
}


void AnimationManagerFinder::apply(osg::Node& node) {
	if (_am.valid())
		return;
	if (node.getUpdateCallback()) {
		osgAnimation::AnimationManagerBase* b = dynamic_cast<osgAnimation::AnimationManagerBase*>(node.getUpdateCallback());
		if (b) {
			_am = new osgAnimation::BasicAnimationManager(*b);
			node.setUpdateCallback(_am.get());
			return;
		}
	}
	traverse(node);
}
