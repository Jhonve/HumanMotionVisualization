#include "OSGGround.h"

#include <osg/Vec3>

#include <osg/Geometry>
#include <osgGA/TrackballManipulator>
#include <osg/MatrixTransform>
#include <osg/Sequence>
#include <vector>

#include <iostream>

OSGGround::OSGGround(void)
{
	rootNode = createGround();
}

OSGGround::~OSGGround(void)
{
}
osg::ref_ptr<osg::Geometry> OSGGround::createGround()
{
	osg::Vec3 center(0.0f, 0.0f, -1.0f);
	float radius = 100.0f;
	int numTilesX = 50;
	int numTilesY = 50;
	float width = 2 * radius;
	float height = 2 * radius;
	osg::Vec3 v000(center - osg::Vec3(width*0.5f, height*0.5f, 0.0f));
	osg::Vec3 dx(osg::Vec3(width / ((float)numTilesX), 0.0, 0.0f));
	osg::Vec3 dy(osg::Vec3(0.0f, height / ((float)numTilesY), 0.0f));

	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;

	osg::ref_ptr<osg::Vec3Array> coords = new osg::Vec3Array;
	geom->setVertexArray(coords);
	int iy;
	for (iy = 0; iy <= numTilesY; ++iy)
	{
		for (int ix = 0; ix <= numTilesX; ++ix)
		{
			coords->push_back(v000 + dx * (float)ix + dy * (float)iy);
			coords->push_back(v000 + dx * (float)(ix + 1) + dy * (float)iy);
			coords->push_back(v000 + dx * (float)(ix + 1) + dy * (float)(iy + 1));
			coords->push_back(v000 + dx * (float)ix + dy * (float)(iy + 1));
		}
	}

	osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
	//colors->push_back(osg::Vec4(97.0f / 255, 97.0f / 255, 97.0f / 255, 1.0f)); // white
	//colors->push_back(osg::Vec4(0.0f / 255, 0.0f / 255, 0.0f / 255, 1.0f));
	geom->setColorArray(colors);
	for (iy = 0; iy <= numTilesY; ++iy)
	{
		for (int ix = 0; ix <= numTilesX; ++ix)
		{
			if ((iy + ix + 1) % 2 == 0)
			{
				for (int ic = 0; ic < 4; ic++)
				{
					colors->push_back(osg::Vec4(0.0f / 255, 0.0f / 255, 0.0f / 255, 1.0f));
				}
			}
			else
			{
				for (int ic = 0; ic < 4; ic++)
				{
					colors->push_back(osg::Vec4(97.0f / 255, 97.0f / 255, 97.0f / 255, 1.0f));
					// colors->push_back(osg::Vec4(255.0f / 255, 255.0f / 255, 255.0f / 255, 1.0f));
				}
			}
		}
	}

	geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
	geom->setNormalArray(normals);
	normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
	geom->setNormalBinding(osg::Geometry::BIND_OVERALL);
	geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, numTilesX * numTilesY * 4));

	// osg::Geode* geode = new osg::Geode;
	// geode->addDrawable(geom);
	// rootNode->addChild(geode);
	// rootNode->setDataVariance(osg::Object::DYNAMIC);

	return geom;
}

osg::ref_ptr<osg::Geometry> OSGGround::getRootNode()
{
	return rootNode;
}