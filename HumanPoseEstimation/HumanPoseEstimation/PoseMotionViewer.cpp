#include "PoseMotionViewer.h"

#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>

#include <osg/Stencil>
#include <osg/ClipNode>
#include <osg/ClipPlane>
#include <osg/BlendFunc>
#include <osgGA/NodeTrackerManipulator>

#include <iostream>
#include <string>

const int ReceivesShadowTraversalMask = 0x1;
const int CastsShadowTraversalMask = 0x2;

PoseMotionViewer::PoseMotionViewer()
{
	m_root_node = new osg::Group;
	m_viewer = new osgViewer::Viewer();

	m_ground = new OSGGround;
	osg::ref_ptr<osg::MatrixTransform> ground_mat = new osg::MatrixTransform;
	osg::Geode* groundGeode = new osg::Geode;
	groundGeode->addDrawable(m_ground->getRootNode());
	ground_mat->addChild(groundGeode);

	//Create Skeleton
	m_skeleton = new OSGSkeleton;
	m_skeleton->createSkeleton("Data/MotionData/01/01_01.bvh");

	osg::ref_ptr<osg::MatrixTransform> skeleton_mat = new osg::MatrixTransform;
	skeleton_mat->addChild(m_skeleton->skeleton);
	skeleton_mat->setMatrix(osg::Matrix::rotate(osg::inDegrees(90.0f), 1.0f, 0.0f, 0.0f));

	m_root_node->addChild(createMirroredScene(skeleton_mat.get()));
	// m_root_node->addChild(m_ground->getRootNode());

	osgUtil::Optimizer optimizer;
	optimizer.optimize(m_root_node.get());

	m_viewer->setUpViewInWindow(120, 120, 1024, 1024);
	m_viewer->setSceneData(m_root_node.get());
	m_viewer->setCameraManipulator(new osgGA::TrackballManipulator);

	// Anti-aliasing
	osg::DisplaySettings* ds = osg::DisplaySettings::instance();
	ds->setNumMultiSamples(1);
	m_viewer->setDisplaySettings(ds);

	AnimtkViewerModelController& mc = AnimtkViewerModelController::instance();
	// mc.stop();
	mc.play();
}

PoseMotionViewer::~PoseMotionViewer()
{

}

osg::Node* PoseMotionViewer::createMirroredScene(osg::Node* model)
{
	osg::MatrixTransform*  mir_node = new osg::MatrixTransform;

	// make sure that the global color mask exists.
	osg::ColorMask* rootColorMask = new osg::ColorMask;
	rootColorMask->setMask(true, true, true, true);

	// set up depth to be inherited by the rest of the scene unless
	// overrideen. this is overridden in bin 3.
	osg::Depth* rootDepth = new osg::Depth;
	rootDepth->setFunction(osg::Depth::LESS);
	rootDepth->setRange(0.0, 1.0);

	osg::StateSet* rootStateSet = new osg::StateSet();
	rootStateSet->setAttribute(rootColorMask);
	rootStateSet->setAttribute(rootDepth);

	mir_node->setStateSet(rootStateSet);

	osg::Drawable* mirror = m_ground->getRootNode();

	// bin1  - set up the stencil values and depth for mirror.
	{
		// set up the stencil ops so that the stencil buffer get set at
		// the mirror plane
		osg::Stencil* stencil = new osg::Stencil;
		stencil->setFunction(osg::Stencil::ALWAYS, 1, ~0u);
		stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::REPLACE);

		// switch off the writing to the color bit planes.
		osg::ColorMask* colorMask = new osg::ColorMask;
		colorMask->setMask(false, false, false, false);
		// colorMask->setMask(true, true, true, true);

		osg::StateSet* statesetBin1 = new osg::StateSet();
		statesetBin1->setRenderBinDetails(1, "RenderBin");
		statesetBin1->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
		statesetBin1->setAttributeAndModes(stencil, osg::StateAttribute::ON);
		statesetBin1->setAttribute(colorMask);

		// set up the mirror geode.
		osg::Geode* geode = new osg::Geode;
		geode->addDrawable(mirror);
		geode->setStateSet(statesetBin1);
		mir_node->addChild(geode);
	}

	// bin 2 - draw scene without mirror or reflection, unset
	// stencil values where scene is infront of mirror and hence
	// occludes the mirror.
	{
		osg::Stencil* stencil = new osg::Stencil;
		stencil->setFunction(osg::Stencil::ALWAYS, 0, ~0u);
		stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::REPLACE);

		osg::StateSet* statesetBin2 = new osg::StateSet();
		statesetBin2->setRenderBinDetails(2, "RenderBin");
		statesetBin2->setAttributeAndModes(stencil, osg::StateAttribute::ON);

		osg::Group* groupBin2 = new osg::Group();
		groupBin2->setStateSet(statesetBin2);
		groupBin2->addChild(model);

		mir_node->addChild(groupBin2);
	}

	// bin3  - set up the depth to the furthest depth value
	{

		// set up the stencil ops so that only operator on this mirrors stencil value.
		osg::Stencil* stencil = new osg::Stencil;
		stencil->setFunction(osg::Stencil::EQUAL, 1, ~0u);
		stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);

		// switch off the writing to the color bit planes.
		osg::ColorMask* colorMask = new osg::ColorMask;
		colorMask->setMask(false, false, false, false);

		// set up depth so all writing to depth goes to maximum depth.
		osg::Depth* depth = new osg::Depth;
		depth->setFunction(osg::Depth::ALWAYS);
		depth->setRange(1.0, 1.0);

		osg::StateSet* statesetBin3 = new osg::StateSet();
		statesetBin3->setRenderBinDetails(3, "RenderBin");
		statesetBin3->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
		statesetBin3->setAttributeAndModes(stencil, osg::StateAttribute::ON);
		statesetBin3->setAttribute(colorMask);
		statesetBin3->setAttribute(depth);

		// set up the mirror geode.
		osg::Geode* geode = new osg::Geode;
		geode->addDrawable(mirror);
		geode->setStateSet(statesetBin3);

		mir_node->addChild(geode);

	}

	// bin4  - draw the reflection.
	{
		osg::ClipPlane* clipplane = new osg::ClipPlane;
		clipplane->setClipPlane(0.0, 0.0, -1.0, 0.0f);
		clipplane->setClipPlaneNum(0);

		osg::ClipNode* clipNode = new osg::ClipNode;
		clipNode->addClipPlane(clipplane);


		osg::StateSet* dstate = clipNode->getOrCreateStateSet();
		dstate->setRenderBinDetails(4, "RenderBin");
		dstate->setMode(GL_CULL_FACE, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF);

		osg::Stencil* stencil = new osg::Stencil;
		stencil->setFunction(osg::Stencil::EQUAL, 1, ~0u);
		stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);
		dstate->setAttributeAndModes(stencil, osg::StateAttribute::ON);

		osg::MatrixTransform* reverseMatrix = new osg::MatrixTransform;
		reverseMatrix->setStateSet(dstate);
		reverseMatrix->preMult(osg::Matrix::translate(0.0f, 0.0f, 0.0f)*
			osg::Matrix::scale(1.0f, 1.0f, -1.0f)*
			osg::Matrix::translate(0.0f, 0.0f, 0.0f));

		reverseMatrix->addChild(model);

		clipNode->addChild(reverseMatrix);

		mir_node->addChild(clipNode);
	}

	osg::StateSet* stateset = m_root_node->getOrCreateStateSet();

	// Light
	float light_hight = 100.0f;
	float light_hight_offset = 10.0f;
	float light_x = 100.0f;
	float light_y = 100.0f;
	osg::Light* light_main = new osg::Light;
	light_main->setLightNum(0);
	light_main->setPosition(osg::Vec4(light_x, light_y, light_hight, 1.0f));
	light_main->setDirection(osg::Vec3(0.f, 0.f, 0.f));
	osg::LightSource* light_source_main = new osg::LightSource;
	light_source_main->setLight(light_main);
	light_source_main->setLocalStateSetModes(osg::StateAttribute::ON);
	stateset->setMode(GL_LIGHT0, osg::StateAttribute::ON);
	light_source_main->setStateSetModes(*stateset, osg::StateAttribute::ON);

	osg::Light* light1 = new osg::Light;
	light1->setLightNum(1);
	light1->setPosition(osg::Vec4(-light_x, -light_y, light_hight + light_hight_offset, 1.0f));
	light1->setDirection(osg::Vec3(0.f, 0.f, 0.f));
	osg::LightSource* light_source_1 = new osg::LightSource;
	light_source_1->setLight(light1);
	light_source_1->setLocalStateSetModes(osg::StateAttribute::ON);
	stateset = m_root_node->getOrCreateStateSet();
	stateset->setMode(GL_LIGHT1, osg::StateAttribute::ON);
	light_source_1->setStateSetModes(*stateset, osg::StateAttribute::ON);

	mir_node->addChild(light_source_main);
	mir_node->addChild(light_source_1);

	//Shadow
	osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene();
	shadowedScene->setReceivesShadowTraversalMask(ReceivesShadowTraversalMask);
	shadowedScene->setCastsShadowTraversalMask(CastsShadowTraversalMask);
	osg::ref_ptr<osgShadow::ShadowMap> sm = new osgShadow::ShadowMap;
	osg::ref_ptr<osgShadow::ShadowTexture> st = new osgShadow::ShadowTexture();
	shadowedScene->setShadowTechnique(st.get());

	// bin5  - draw the textured mirror and blend it with the reflection.
	{
		// set up depth so all writing to depth goes to maximum depth.
		osg::Depth* depth = new osg::Depth;
		depth->setFunction(osg::Depth::ALWAYS);

		osg::Stencil* stencil = new osg::Stencil;
		stencil->setFunction(osg::Stencil::EQUAL, 1, ~0u);
		stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::ZERO);

		// set up additive blending.
		osg::BlendFunc* trans = new osg::BlendFunc;
		trans->setFunction(osg::BlendFunc::ONE, osg::BlendFunc::ONE);

		osg::StateSet* statesetBin5 = new osg::StateSet();

		statesetBin5->setRenderBinDetails(5, "RenderBin");
		statesetBin5->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
		statesetBin5->setAttributeAndModes(stencil, osg::StateAttribute::ON);
		statesetBin5->setAttributeAndModes(trans, osg::StateAttribute::ON);
		statesetBin5->setAttribute(depth);

		// set up the mirror geode.
		osg::Geode* geode = new osg::Geode;
		geode->addDrawable(mirror);
		geode->setStateSet(statesetBin5);

		 model->setNodeMask(CastsShadowTraversalMask);
		 geode->setNodeMask(ReceivesShadowTraversalMask);
		 shadowedScene->addChild(light_source_main);
		 shadowedScene->addChild(light_source_1);
		 shadowedScene->addChild(model);
		 shadowedScene->addChild(geode);
		 
		 mir_node->addChild(shadowedScene);
	}

	return mir_node;
}