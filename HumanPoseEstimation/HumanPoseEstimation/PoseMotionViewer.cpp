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

PoseMotionViewer::PoseMotionViewer(int pos_x, int pos_y, int width, int height)
{
	m_root_node = new osg::Group;
	m_viewer = new osgViewer::Viewer();

	m_ground = new OSGGround;
	m_skeleton = new OSGSkeleton;

	// Visualization test for skeleton and ground.
	/*osg::ref_ptr<osg::MatrixTransform> ground_mat = new osg::MatrixTransform;
	osg::Geode* groundGeode = new osg::Geode;
	groundGeode->addDrawable(m_ground->getRootNode());
	ground_mat->addChild(groundGeode);

	m_skeleton->createSkeleton("Data/MotionData/01/01_01.bvh");
	osg::ref_ptr<osg::MatrixTransform> skeleton_mat = new osg::MatrixTransform;
	skeleton_mat->addChild(m_skeleton->skeleton);
	skeleton_mat->setMatrix(osg::Matrix::rotate(osg::inDegrees(90.0f), 1.0f, 0.0f, 0.0f));

	m_root_node->addChild(skeleton_mat);
	m_root_node->addChild(ground_mat);*/

	m_root_node->addChild(createScene());

	osgUtil::Optimizer optimizer;
	optimizer.optimize(m_root_node.get());

	m_viewer->setUpViewInWindow(pos_x, pos_y, width, height);
	m_viewer->setSceneData(m_root_node.get());
	m_viewer->getCamera()->setViewport(new osg::Viewport(0, 0, width, height));
	m_viewer->getCamera()->setClearColor(osg::Vec4(255.f / 255.f, 255.f / 255.f, 255.f / 255.f, 0.5f));
	m_viewer->getCamera()->setProjectionMatrixAsPerspective(45.0f, static_cast<double>(width) / static_cast<double>(height), 1.0f, 100.0f);
	m_viewer->setCameraManipulator(new osgGA::TrackballManipulator);

	// Anti-aliasing
	osg::DisplaySettings* disp_setting = osg::DisplaySettings::instance();
	disp_setting->setNumMultiSamples(10);
	m_viewer->setDisplaySettings(disp_setting);

	AnimtkViewerModelController& model_controller = AnimtkViewerModelController::instance();
	model_controller.play();
}

PoseMotionViewer::~PoseMotionViewer()
{

}

osg::Node* PoseMotionViewer::createScene()
{
	osg::MatrixTransform*  scene_node = new osg::MatrixTransform;

	// make sure that the global color mask exists.
	osg::ColorMask* root_color_mask = new osg::ColorMask;
	root_color_mask->setMask(true, true, true, true);

	// set up depth to be inherited by the rest of the scene unless
	// overrideen. this is overridden in bin 3.
	 osg::Depth* root_depth = new osg::Depth;
	 root_depth->setFunction(osg::Depth::LESS);
	 root_depth->setRange(0.0, 1.0);

	osg::StateSet* root_state_set = new osg::StateSet();
	root_state_set->setAttribute(root_color_mask);
	root_state_set->setAttribute(root_depth);

	scene_node->setStateSet(root_state_set);

	osg::Drawable* mirror_ground = m_ground->getRootNode();

	m_skeleton->createSkeleton("Data/MotionData/01/01_01.bvh");
	osg::ref_ptr<osg::MatrixTransform> human_model = new osg::MatrixTransform;
	human_model->addChild(m_skeleton->skeleton);
	human_model->setMatrix(osg::Matrix::rotate(osg::inDegrees(90.0f), 1.0f, 0.0f, 0.0f));

	// bin1  - draw the reflection.
	{
		osg::ClipPlane* clipplane = new osg::ClipPlane;
		clipplane->setClipPlane(0.0, 0.0, -1.0, 0.0f);
		clipplane->setClipPlaneNum(0);

		osg::ClipNode* clipNode = new osg::ClipNode;
		clipNode->addClipPlane(clipplane);

		osg::StateSet* dstate = clipNode->getOrCreateStateSet();
		dstate->setRenderBinDetails(1, "RenderBin");
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

		reverseMatrix->addChild(human_model);

		clipNode->addChild(reverseMatrix);

		scene_node->addChild(clipNode);
	}

	osg::StateSet* stateset = m_root_node->getOrCreateStateSet();

	// Light
	float light_hight = 100.0f;
	float light_x = -100.0f;
	float light_y = -100.0f;
	osg::Light* light_main = new osg::Light;
	light_main->setLightNum(0);
	light_main->setPosition(osg::Vec4(light_x, light_y, light_hight, 1.0f));
	light_main->setDirection(osg::Vec3(0.f, 0.f, 0.f));
	light_main->setAmbient(osg::Vec4(0.01f, 0.01f, 0.01f, 1.0f));
	light_main->setDiffuse(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	osg::LightSource* light_source_main = new osg::LightSource;
	light_source_main->setLight(light_main);
	light_source_main->setLocalStateSetModes(osg::StateAttribute::ON);
	stateset->setMode(GL_LIGHT0, osg::StateAttribute::ON);
	light_source_main->setStateSetModes(*stateset, osg::StateAttribute::ON);

	//Shadow
	osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene();
	shadowedScene->setReceivesShadowTraversalMask(ReceivesShadowTraversalMask);
	shadowedScene->setCastsShadowTraversalMask(CastsShadowTraversalMask);
	osg::ref_ptr<osgShadow::ShadowMap> sm = new osgShadow::ShadowMap;
	osg::ref_ptr<osgShadow::ShadowTexture> st = new osgShadow::ShadowTexture();
	shadowedScene->setShadowTechnique(sm.get());

	// bin2  - draw the textured mirror and blend it with the reflection.
	{
		osg::Stencil* stencil = new osg::Stencil;
		stencil->setFunction(osg::Stencil::EQUAL, 1, ~0u);
		stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::ZERO);

		// set up additive blending.
		osg::BlendFunc* trans = new osg::BlendFunc;
		trans->setFunction(osg::BlendFunc::SRC_COLOR, osg::BlendFunc::SRC_COLOR);

		osg::StateSet* statesetBin5 = new osg::StateSet();

		statesetBin5->setRenderBinDetails(2, "RenderBin");
		statesetBin5->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
		statesetBin5->setAttributeAndModes(stencil, osg::StateAttribute::ON);
		statesetBin5->setAttributeAndModes(trans, osg::StateAttribute::ON);

		// set up the mirror geode.
		osg::Geode* geode = new osg::Geode;
		geode->addDrawable(mirror_ground);
		geode->setStateSet(statesetBin5);
		
		human_model->setNodeMask(CastsShadowTraversalMask);
		geode->setNodeMask(ReceivesShadowTraversalMask);
		shadowedScene->addChild(light_source_main);
		shadowedScene->addChild(human_model);
		shadowedScene->addChild(geode);
		 
		scene_node->addChild(shadowedScene);
	}

	return scene_node;
}