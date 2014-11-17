#include "AnimationSplitter.h"


AnimationSplitter::AnimationSplitter(std::string config)
	: _fps(30)
{

	_ready = ReadXMLConfigHeaderFromFile(config);
	if (!_ready){
		OSG_WARN << "OsgAnimationTools ERROR: an error occured initalizing OsgAnimationTools config. Check your config file" << std::endl;
	}
	OSG_INFO << "OsgAnimationTools INFO: Config found and loaded, sourceFile: '" << _sourceFile << "', destinationFile: '" << _destinationFile << "', FPS: '" << _fps << "'." << std::endl;

	//load the source file
	OSG_INFO << "OsgAnimationTools INFO: Loading sourceFile..." << std::endl;

	_sourceNode = osgDB::readNodeFile(_sourceFile);
	if (!_sourceNode.get())
	{
		OSG_WARN << "OsgAnimationTools ERROR: Failed to load sourceFile '" << _sourceFile << "'. OsgAnimationTools can not continue." << std::endl;
		return;
	}

	//export the original as osgt
	std::string importFolder = osgDB::getFilePath(_sourceFile);
	osgDB::writeNodeFile(*_sourceNode.get(), importFolder + "/originalExported.osgt");

	OSG_INFO << "OsgAnimationTools INFO: SourceFile loaded successfully" << std::endl;

	OSG_INFO << "OsgAnimationTools INFO: Checking for AnimationSplit info in config file..." << std::endl;
	bool foundSplit = ReadAniSplitFromConfigNode();
	if (!foundSplit || _animationSplits.size() == 0){
		OSG_WARN << "OsgAnimationTools ERROR: No AnimationSplit nodes found in Config, nothing to do." << std::endl;
		return;
	}

	//now do the split
	if (!FindAndSplitAnimations()){
		OSG_WARN << "OsgAnimationTools ERROR: Failed to Split animations." << std::endl;
		return;
	}

	//it worked so save the result
	osgDB::writeNodeFile(*_sourceNode.get(), _destinationFile);


}


bool AnimationSplitter::ReadXMLConfigHeaderFromFile(std::string configFile)
{
	//check the file exists
	if (!osgDB::fileExists(configFile))
	{
		OSG_WARN << "OsgAnimationTools XML Config ERROR: Failed to read xml file '" << configFile << "'," << std::endl
			<< "                                        The file does not exists." << std::endl;
		return false;
	}

	//allocate the document node
	osgDB::XmlNode* doc = new osgDB::XmlNode;
	osgDB::XmlNode* root = 0;

	//open the file with xmlinput
	osgDB::XmlNode::Input input;
	input.open(configFile);
	input.readAllDataIntoBuffer();

	//read the file into out document
	doc->read(input);

	//iterate over the document nodes and try and find a OsgAnimationSplitter node to
	//use as a root
	for (osgDB::XmlNode::Children::iterator itr = doc->children.begin();
		itr != doc->children.end() && !root;
		++itr)
	{
		if ((*itr)->name == "OsgAnimationTools") root = (*itr);
	}

	if (root == NULL)
	{
		OSG_WARN << "OsgAnimationTools XML Config ERROR: Failed to read xml file '" << configFile << "'," << std::endl
			<< "                                        OsgAnimationTools Config file must contain a valid <OsgAnimationTools> node." << std::endl;
		return false;
	}

	//we have our head node
	_configNode = root;

	//the head node should contain source and destination file properties
	if (_configNode->properties.count("sourceFile") > 0)
	{
		_sourceFile = _configNode->properties["sourceFile"];
	}
	else{
		OSG_WARN << "OsgAnimationTools XML Config ERROR: Failed to read xml file '" << configFile << "'," << std::endl
			<< "                                        OsgAnimationTools node must contain a 'sourceFile' property." << std::endl;
		return false;
	}

	if (_configNode->properties.count("destinationFile") > 0)
	{
		_destinationFile = _configNode->properties["destinationFile"];
	}
	else{
		OSG_WARN << "OsgAnimationTools XML Config ERROR: Failed to read xml file '" << configFile << "'," << std::endl
			<< "                                        OsgAnimationTools node must contain a 'destinationFile' property." << std::endl;
		return false;
	}

	if (_configNode->properties.count("fps") > 0)
	{
		std::string fpsStr = _configNode->properties["fps"];
		//not too safe
		_fps = atoi(fpsStr.c_str());

	}
	else{
		OSG_WARN << "OsgAnimationTools AnimationSplit XML Config WARNING: Your config does not specifiy a 'fps' property. Defaulting to 30 fps" << std::endl;
	}

	return true;
}

bool AnimationSplitter::ReadAniSplitFromConfigNode(){

	//config must have already been read from file
	if (!_configNode.get()){
		OSG_WARN << "OsgAnimationTools AnimationSplit XML Config ERROR: Config file must have been loaded first via call to 'ReadXMLConfigHeaderFromFile'. Can not continue." << std::endl;
		return false;
	}

	//search root children for any <AnimationSplit> nodes
	for (unsigned int i = 0; i<_configNode->children.size(); i++)
	{
		//if its name is animation split then we have a vslid split to load
		if (_configNode->children[i]->name == "AnimationSplit")
		{
			NameToStartEndTimeMap splitAni;
			osgDB::XmlNode* splitNode = _configNode->children[i];
			std::string sourceAniName = "";

			//an animation split head node contains a sourceAnimation name and a number of splits contained within it's children
			if (splitNode->properties.count("sourceAnimation") > 0)
			{
				sourceAniName = splitNode->properties["sourceAnimation"];
			}
			else{
				OSG_WARN << "OsgAnimationTools AnimationSplit XML Config ERROR: AnimationSplit nodes must contain a 'sourceAnimation' property." << std::endl;
				return false;
			}

			//we have the target animation and number of expected splits, now try each child as a NEW_ANIMATION
			for (unsigned int t = 0; t<splitNode->children.size(); t++)
			{
				//check name matches
				if (splitNode->children[t]->name == "NewAnimation")
				{
					NameToStartEndTimePair newAnimation;
					if (!ReadNewAnimationFromNode(splitNode->children[t], newAnimation)){
						OSG_WARN << "OsgAnimationTools AnimationSplit XML Config ERROR: Failed to read <NewAnimation> node, can not continue." << std::endl;
						return false;
					}
					//add the new animation to this split 
					splitAni.insert(newAnimation);
				}
				else{
					OSG_WARN << "OsgAnimationTools AnimationSplit XML Config ERROR: Children of <AnimationSplit> should be of type <NewAnimation>. The config can not be passed." << std::endl;
					return false;
				}
			}

			//add the new split to our list of splits
			_animationSplits.insert(SourceToSplitPair(sourceAniName, splitAni));
		}//end of split
	}//loop next child

	return true;
}

bool AnimationSplitter::ReadNewAnimationFromNode(osgDB::XmlNode* node, NameToStartEndTimePair& newAnimationParams)
{
	//name should be new animation
	if (!node){
		OSG_WARN << "OsgAnimationTools ReadNewAnimationFromNode ERROR: Null node passed." << std::endl;
		return false;
	}
	if (node->name != "NewAnimation"){
		OSG_WARN << "OsgAnimationTools ReadNewAnimationFromNode ERROR: Node is not a <NewAnimation> Node." << std::endl;
		return false;
	}

	std::string newName = "";
	int start, end = -1;
	//contains 3 properties, start/end frame, new animation name
	if (node->properties.count("name") > 0)
	{
		newName = node->properties["name"];
	}
	else{
		OSG_WARN << "OsgAnimationTools ReadNewAnimationFromNode ERROR: <NewAnimation> nodes must contain a 'name' property." << std::endl;
		return false;
	}
	if (node->properties.count("startFrame") > 0)
	{
		std::string startFrameStr = node->properties["startFrame"];
		//convert to frame
		start = atoi(startFrameStr.c_str());
	}
	else{
		OSG_WARN << "OsgAnimationTools ReadNewAnimationFromNode ERROR: <NewAnimation> nodes must contain a 'startFrame' property." << std::endl;
		return false;
	}
	if (node->properties.count("endFrame") > 0)
	{
		std::string endFrameStr = node->properties["endFrame"];
		//convert to frame
		end = atoi(endFrameStr.c_str());
	}
	else{
		OSG_WARN << "OsgAnimationTools ReadNewAnimationFromNode ERROR: <NewAnimation> nodes must contain a 'startFrame' property." << std::endl;
		return false;
	}

	//store info in the passed NameToStartEndTimePair
	newAnimationParams.first = newName;
	newAnimationParams.second = osg::Vec2(start, end);
	return true;
}

bool AnimationSplitter::FindAndSplitAnimations()
{
	if (_animationSplits.size() == 0){
		return false;
	}
	SourceToSplitMap::iterator splitItr = _animationSplits.begin();
	for (; splitItr != _animationSplits.end(); splitItr++)
	{
		//get the name of the source animation
		FindOsgAnimationByName findAniVis((*splitItr).first);
		_sourceNode->accept(findAniVis);
		if (findAniVis.p_ani != NULL)
		{
			//the animation was found, we need to copy it so it can be replaced with the new split animations
			osg::ref_ptr<osgAnimation::Animation> originalAnimation = osg::clone(findAniVis.p_ani, osg::CopyOp::DEEP_COPY_ALL);
			//delete the original (at this point findAniVis.p_ani will probably become invalid, which is ok as we copied it above)
			findAniVis.p_manager->unregisterAnimation(findAniVis.p_ani);

			//now iterate over this splits new aniations
			NameToStartEndTimeMap::iterator newAnimsItr = (*splitItr).second.begin();
			for (; newAnimsItr != (*splitItr).second.end(); newAnimsItr++)
			{
				//get the split info
				std::string newName = (*newAnimsItr).first;
				osg::Vec2 startEndVec = (*newAnimsItr).second;
				osg::ref_ptr<osgAnimation::Animation> newAnimation = AnimationUtils::ResampleAnimation(originalAnimation.get(), (int)startEndVec.x(), (int)startEndVec.y(), _fps, newName);
				if (newAnimation.get()){
					findAniVis.p_manager->registerAnimation(newAnimation.get());
				}
				else{
					return false;
				}
			}
		}
		else{
			OSG_WARN << "OsgAnimationTools FindAndSplitAnimations ERROR: Failed to find animation named '" << (*splitItr).first << "', can not perform split." << std::endl;
			return false;
		}
	}
	return true;
}
