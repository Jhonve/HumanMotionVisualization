#pragma once

#include <windows.h>

#include <osgViewer/Viewer>
#include <osg/Group>
#include <osgFX/SpecularHighlights>

class OSGGround
{
public:
	OSGGround();
	~OSGGround();

	osg::ref_ptr<osg::Geometry> createGround();
	osg::ref_ptr<osg::Geometry> getRootNode();

private:
	osg::ref_ptr<osg::Geometry> rootNode;
};