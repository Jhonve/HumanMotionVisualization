#pragma once

#include <windows.h>

#include <osg/Geometry>
#include <osg/Notify>
#include <osg/ShapeDrawable>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>
#include <string.h>

#include <osgDB/ReadFile>
#include <osgAnimation/Bone>
#include <osgAnimation/Skeleton>
#include <osgAnimation/UpdateBone>
#include <osgAnimation/StackedTransform>
#include <osgAnimation/StackedTranslateElement>
#include <osgAnimation/StackedQuaternionElement>
#include <osgAnimation/BasicAnimationManager>

#include "AnimtkViewerModelController.h"

using namespace std;

class OSGSkeleton
{
public:
	OSGSkeleton();
	osg::ref_ptr<osg::Group> skeleton;


	void createSkeleton(string str);
};