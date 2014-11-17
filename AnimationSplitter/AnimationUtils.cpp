#include "AnimationUtils.h"



AnimationUtils::AnimationUtils()
{
}


AnimationUtils::~AnimationUtils()
{
}

template < typename ContainerType > 
int AnimationUtils::GetNearestKeyFrameIndex(ContainerType* keyContainer, double time)
{
	if (!keyContainer){ return -1; }

	int closestFrame = -1;
	double closestDiff = 99999.99f;

	//loop all the keys
	for (unsigned int i = 0; i<keyContainer->size(); i++)
	{
		double diff = fabs(time - (*keyContainer)[i].getTime());
		if (diff < closestDiff){
			closestFrame = i;
			closestDiff = diff;
		}
	}
	return closestFrame;
}

template < typename ChannelType, typename ContainerType >
osg::ref_ptr<ChannelType> AnimationUtils::ResampleChannel(ChannelType* sourceChannel, unsigned int startFrame, unsigned int endFrame, int fps)
{
	osg::ref_ptr<ChannelType> newChannel = NULL;
	if (!sourceChannel){
		return newChannel;
	}

	//get the key frame container from the source channel
	ContainerType* sourceKeyCont = sourceChannel->getSamplerTyped()->getKeyframeContainerTyped();

	if (sourceKeyCont)
	{
		OSG_INFO << "OsgAnimationTools ResampleChannel INFO: Resampling source channel '" << sourceChannel->getName()
			<< "', from startFrame '" << startFrame << "' to endFrame '" << endFrame << ". Total frames in source channel '" << sourceKeyCont->size() << "'." << std::endl;

		//determine the copy direction, i.e. lets see if we can copy frames in reverse as could come in handy
		unsigned int from = startFrame;
		unsigned int to = endFrame;
		if (startFrame > endFrame){
			from = endFrame;
			to = startFrame;
		}

		//get our frames as a times 
		double fromTime = from == 0 ? 0.0f : (float)from / (float)fps;
		double toTime = to == 0 ? 0.0f : (float)to / (float)fps;

		//needs to be if time is too big
		float firstTime = (*sourceKeyCont)[0].getTime();
		float lastTime = (*sourceKeyCont)[sourceKeyCont->size() - 1].getTime();
		/*if(fromTime < firstTime || toTime > lastTime)
		{
		OSG_WARN << "OsgAnimationTools ResampleChannel ERROR: startTime or endTime is out of range," << std::endl
		<< "                                         startTime '" << fromTime << "', endFrame '" << toTime << ". Max time in channel '" << lastTime << "'." << std::endl;

		return newChannel;
		}*/

		//find the true key frame numbers as some animation channels contain more then the actual fps
		from = GetNearestKeyFrameIndex<ContainerType>(sourceKeyCont, fromTime);
		if (from == -1){ return newChannel; }
		to = GetNearestKeyFrameIndex<ContainerType>(sourceKeyCont, toTime);
		if (to == -1){ return newChannel; }

		//get the offset time form the keys
		double offsetToZero = (*sourceKeyCont)[from].getTime();

		//create our new channel with same type, name and target name
		newChannel = new ChannelType();
		newChannel->setName(sourceChannel->getName());
		newChannel->setTargetName(sourceChannel->getTargetName());

		//get the new channels key containter, also creating the sampler and container if required
		ContainerType* destKeyCont = newChannel->getOrCreateSampler()->getOrCreateKeyframeContainer();

		//now copy all the frames between fromTime and toTime
		for (unsigned int k = from; k <= to; k++)
		{
			//push source frame onto destination container
			destKeyCont->push_back((*sourceKeyCont)[k]);
			//adjust the new frames time so animation starts from 0.0
			double sourceTime = (*sourceKeyCont)[k].getTime();
			(*destKeyCont)[destKeyCont->size() - 1].setTime(sourceTime - offsetToZero);
		}

		return newChannel;

	}
	else{
		OSG_WARN << "OsgAnimationTools ResampleChannel ERROR: source channel contains no key frame container," << std::endl;
		return newChannel;
	}
	return newChannel;
}

osg::ref_ptr<osgAnimation::Animation> AnimationUtils::ResampleAnimation(osgAnimation::Animation* source,
	int startFrame, int endFrame, int fps,
	std::string newName)
{
	osg::ref_ptr<osgAnimation::Animation> newAnimation = NULL;
	if (!source){
		return newAnimation;
	}

	newAnimation = new osgAnimation::Animation();
	newAnimation->setName(newName);

	//loop all the source channels
	osgAnimation::ChannelList sourceChannels = source->getChannels();
	for (unsigned int i = 0; i<sourceChannels.size(); i++)
	{
		//clone the channel type, name and target
		//this is a dumb method but I cant find a way to just clone the channel type,
		//so instead we copy the whole channel, keys and all, then delete the ones we don't want :(
		osgAnimation::Channel* pChannel = sourceChannels[i].get();
		osg::ref_ptr<osgAnimation::Channel> resampledChannel = NULL;

		//osgAnimation::Channel* pChannel = anim.getChannels()[i].get();

		osgAnimation::DoubleLinearChannel* pDlc = dynamic_cast<osgAnimation::DoubleLinearChannel*>(pChannel);
		if (pDlc)
		{
			resampledChannel = ResampleChannel<osgAnimation::DoubleLinearChannel, osgAnimation::DoubleKeyframeContainer>(pDlc, startFrame, endFrame, fps);
			if (resampledChannel){ newAnimation->addChannel(resampledChannel.get()); }
			continue;
		}
		osgAnimation::FloatLinearChannel* pFlc = dynamic_cast<osgAnimation::FloatLinearChannel*>(pChannel);
		if (pFlc)
		{
			resampledChannel = ResampleChannel<osgAnimation::FloatLinearChannel, osgAnimation::FloatKeyframeContainer>(pFlc, startFrame, endFrame, fps);
			if (resampledChannel){ newAnimation->addChannel(resampledChannel.get()); }
			continue;
		}
		osgAnimation::Vec2LinearChannel* pV2lc = dynamic_cast<osgAnimation::Vec2LinearChannel*>(pChannel);
		if (pV2lc)
		{
			resampledChannel = ResampleChannel<osgAnimation::Vec2LinearChannel, osgAnimation::Vec2KeyframeContainer>(pV2lc, startFrame, endFrame, fps);
			if (resampledChannel){ newAnimation->addChannel(resampledChannel.get()); }
			continue;
		}
		osgAnimation::Vec3LinearChannel* pV3lc = dynamic_cast<osgAnimation::Vec3LinearChannel*>(pChannel);
		if (pV3lc)
		{
			resampledChannel = ResampleChannel<osgAnimation::Vec3LinearChannel, osgAnimation::Vec3KeyframeContainer>(pV3lc, startFrame, endFrame, fps);
			if (resampledChannel){ newAnimation->addChannel(resampledChannel.get()); }
			continue;
		}
		osgAnimation::Vec4LinearChannel* pV4lc = dynamic_cast<osgAnimation::Vec4LinearChannel*>(pChannel);
		if (pV4lc)
		{
			resampledChannel = ResampleChannel<osgAnimation::Vec4LinearChannel, osgAnimation::Vec4KeyframeContainer>(pV4lc, startFrame, endFrame, fps);
			if (resampledChannel){ newAnimation->addChannel(resampledChannel.get()); }
			continue;
		}
		osgAnimation::QuatSphericalLinearChannel* pQslc = dynamic_cast<osgAnimation::QuatSphericalLinearChannel*>(pChannel);
		if (pQslc)
		{
			resampledChannel = ResampleChannel<osgAnimation::QuatSphericalLinearChannel, osgAnimation::QuatKeyframeContainer>(pQslc, startFrame, endFrame, fps);
			if (resampledChannel){ newAnimation->addChannel(resampledChannel.get()); }
			continue;
		}
		osgAnimation::FloatCubicBezierChannel* pFcbc = dynamic_cast<osgAnimation::FloatCubicBezierChannel*>(pChannel);
		if (pFcbc)
		{
			resampledChannel = ResampleChannel<osgAnimation::FloatCubicBezierChannel, osgAnimation::FloatCubicBezierKeyframeContainer>(pFcbc, startFrame, endFrame, fps);
			if (resampledChannel){ newAnimation->addChannel(resampledChannel.get()); }
			continue;
		}
		osgAnimation::DoubleCubicBezierChannel* pDcbc = dynamic_cast<osgAnimation::DoubleCubicBezierChannel*>(pChannel);
		if (pDcbc)
		{
			resampledChannel = ResampleChannel<osgAnimation::DoubleCubicBezierChannel, osgAnimation::DoubleCubicBezierKeyframeContainer>(pDcbc, startFrame, endFrame, fps);
			if (resampledChannel){ newAnimation->addChannel(resampledChannel.get()); }
			continue;
		}
		osgAnimation::Vec2CubicBezierChannel* pV2cbc = dynamic_cast<osgAnimation::Vec2CubicBezierChannel*>(pChannel);
		if (pV2cbc)
		{
			resampledChannel = ResampleChannel<osgAnimation::Vec2CubicBezierChannel, osgAnimation::Vec2CubicBezierKeyframeContainer>(pV2cbc, startFrame, endFrame, fps);
			if (resampledChannel){ newAnimation->addChannel(resampledChannel.get()); }
			continue;
		}
		osgAnimation::Vec3CubicBezierChannel* pV3cbc = dynamic_cast<osgAnimation::Vec3CubicBezierChannel*>(pChannel);
		if (pV3cbc)
		{
			resampledChannel = ResampleChannel<osgAnimation::Vec3CubicBezierChannel, osgAnimation::Vec3CubicBezierKeyframeContainer>(pV3cbc, startFrame, endFrame, fps);
			if (resampledChannel){ newAnimation->addChannel(resampledChannel.get()); }
			continue;
		}
		osgAnimation::Vec4CubicBezierChannel* pV4cbc = dynamic_cast<osgAnimation::Vec4CubicBezierChannel*>(pChannel);
		if (pV4cbc)
		{
			resampledChannel = ResampleChannel<osgAnimation::Vec4CubicBezierChannel, osgAnimation::Vec4CubicBezierKeyframeContainer>(pV4cbc, startFrame, endFrame, fps);
			if (resampledChannel){ newAnimation->addChannel(resampledChannel.get()); }
			continue;
		}
	}//loop channel

	//now the channels are in, compute the duration
	newAnimation->computeDuration();

	return newAnimation;
}

