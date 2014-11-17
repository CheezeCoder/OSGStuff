#include <map>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/XmlParser>
#include <osgAnimation\Animation>
#include "FindOsgAnimationByName.h"
#include "AnimationUtils.h"

//
//Loads an xml config file used to
//Specify a source and destination model file
//Specifiy a target animation used to create new resampled animations based on start end frames
class AnimationSplitter
{
public:
	//map new animation names to start end times in the original animation
	//these are loaded in from the config file
	typedef std::map<std::string, osg::Vec2> NameToStartEndTimeMap;
	typedef std::pair<std::string, osg::Vec2> NameToStartEndTimePair;
	//map an original animation name to its NameToStartEndTimeMap split version
	typedef std::map<std::string, NameToStartEndTimeMap> SourceToSplitMap;
	typedef std::pair<std::string, NameToStartEndTimeMap> SourceToSplitPair;

	AnimationSplitter(std::string config);

protected:

	//Finds the root <OsgAnimationTools> node and reads the source and destination properties from it
	bool ReadXMLConfigHeaderFromFile(std::string configFile);

	//
	//Reads in any AnimationSplit nodes from our configNode,
	bool ReadAniSplitFromConfigNode();

	//
	//reads in a single newAnimation as part of a SplitAnimation
	bool ReadNewAnimationFromNode(osgDB::XmlNode* node, NameToStartEndTimePair& newAnimationParams);

	//
	//Find the source animation in our source node, then use it to create the new split versions
	//we should have already read the config and filled our _animationSplits list
	bool FindAndSplitAnimations();

	//was the config found and loaded
	bool _ready;

	//root xml node for the OsgAnimation tools config
	osg::ref_ptr<osgDB::XmlNode> _configNode;

	//the head node of the xml config should give a source and destination file for the conversion
	std::string _sourceFile;
	std::string _destinationFile;

	//the node loaded from source
	osg::ref_ptr<osg::Node> _sourceNode;

	//the fps of the source file, this is needed to convert the passed frames to a time
	int _fps;

	//animation splits
	SourceToSplitMap _animationSplits;
};