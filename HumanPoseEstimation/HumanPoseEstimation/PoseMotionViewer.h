#pragma once

#include "OSGSkeleton.h"
#include "OSGGround.h"

#include <osg/Group>
#include <osg/MatrixTransform>
#include <osgGA/TrackballManipulator>
#include <osgFX/SpecularHighlights>
#include <osgUtil/Optimizer>
#include <osgDB/ReaderWriter>
#include <osgDB/ReadFile>
#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowTexture>
#include <osgShadow/ShadowMap>
#include <osgShadow/SoftShadowMap>
#include <osgShadow/ParallelSplitShadowMap>
#include <osgShadow/LightSpacePerspectiveShadowMap>
#include <osgShadow/StandardShadowMap>

class PoseMotionViewer
{
public:
	PoseMotionViewer(int pos_x, int pos_y, int width, int height);
	~PoseMotionViewer();

	osg::ref_ptr<osgViewer::Viewer> m_viewer;

private:
	OSGGround* m_ground;
	OSGSkeleton* m_skeleton;

	osg::ref_ptr<osg::Group> m_root_node;

	osg::Node* createScene();
};