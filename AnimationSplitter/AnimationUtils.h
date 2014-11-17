#include <osgAnimation/Animation>



class AnimationUtils
{
	
public:
	AnimationUtils();
	~AnimationUtils();
	template < typename ContainerType >
	//returns the index of the keyframe closest to the passed time
	//returns -1 if the time is out of range of the key container
	
	static int GetNearestKeyFrameIndex(ContainerType* keyContainer, double time);

	//// Helper method for resampling channels
	template < typename ChannelType, typename ContainerType > 
	static osg::ref_ptr<ChannelType> ResampleChannel(ChannelType* sourceChannel, unsigned int startFrame, unsigned int endFrame, int fps);

	//
	//Create and return a new animation based on the start end frames of the source animation
	//creating all the relevant channels etc
	static osg::ref_ptr<osgAnimation::Animation> ResampleAnimation(osgAnimation::Animation* source,
		int startFrame, int endFrame, int fps,
		std::string newName);
};
