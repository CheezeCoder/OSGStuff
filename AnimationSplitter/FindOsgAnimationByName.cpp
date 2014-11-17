#include "FindOsgAnimationByName.h"


FindOsgAnimationByName::FindOsgAnimationByName(std::string sourceAnimationName)
	: osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
	p_ani(NULL),
	p_manager(NULL),
	_sourceAnimationName(sourceAnimationName)
{
}


void FindOsgAnimationByName::apply(osg::Node& node) {

	if (node.getUpdateCallback()) {
		osgAnimation::AnimationManagerBase* b = dynamic_cast<osgAnimation::AnimationManagerBase*>(node.getUpdateCallback());
		if (b) {
			//we have found a valid osgAnimation manager, now look for the single long animation inside with the desired name
			osgAnimation::AnimationList aniList = b->getAnimationList();
			for (unsigned int i = 0; i<aniList.size(); i++){
				if (aniList[i]->getName() == _sourceAnimationName){
					p_manager = b;
					p_ani = aniList[i].get();
					return;
				}
			}
		}
	}
	traverse(node);
}
