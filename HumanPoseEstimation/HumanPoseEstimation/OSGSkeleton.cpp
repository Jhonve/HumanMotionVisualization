#include "OSGSkeleton.h"

osg::ref_ptr<osg::Group> createBody(osg::Vec3 offset)
{
	// blue color
	float bone_width = 0.3f;
	float ball_diameter = bone_width + 0.4;
	osg::Vec4 bone_color(85.0f / 255, 146.0f / 255, 237.0f / 255, 1.0f);
	osg::Vec4 joint_color(150.0f / 255, 210.0f / 255, 255.0f / 255, 1.0f);
	osg::ref_ptr<osg::Group> group = new osg::Group;

	osg::Vec3 start_point = osg::Vec3(0.0f, 0.0f, 0.0f);
	osg::Vec3 t = offset;
	float len = t.length();
	t /= 2;

	osg::Cylinder *unit_cylinder = new osg::Cylinder(osg::Vec3(0, 0, 0), bone_width, len);
	osg::ShapeDrawable* unit_cylinder_drawable = new osg::ShapeDrawable(unit_cylinder);

	osg::Sphere* start_sphere = new osg::Sphere(start_point, ball_diameter);
	osg::Sphere* end_sphere = new osg::Sphere(offset, ball_diameter);
	osg::ShapeDrawable* start_sphere_drawable = new osg::ShapeDrawable(start_sphere);
	osg::ShapeDrawable* end_sphere_drawable = new osg::ShapeDrawable(end_sphere);

	unit_cylinder_drawable->setColor(bone_color);
	start_sphere_drawable->setColor(joint_color);
	end_sphere_drawable->setColor(joint_color);

	osg::Geode* geode1 = new osg::Geode;
	osg::Geode* geode2 = new osg::Geode;

	osg::MatrixTransform* transform = new osg::MatrixTransform;
	transform->addChild(geode1);

	// the axis of rotation
	osg::Vec3 axis = osg::Vec3(0, 0, 1) ^ t;

	// the angle of rotationi
	float theta = acos(t.z() / t.length());

	osg::Matrixd mat1, mat2;
	mat1.makeRotate(osg::Quat(theta, axis));
	mat2.makeTranslate(t);
	transform->setMatrix(mat1*mat2);

	geode1->addDrawable(unit_cylinder_drawable);
	geode2->addDrawable(start_sphere_drawable);
	geode2->addDrawable(end_sphere_drawable);
	group->addChild(transform);
	group->addChild(geode2);
	return group;
}

struct AnimationManagerFinder : public osg::NodeVisitor
{
	osg::ref_ptr<osgAnimation::BasicAnimationManager> _am;
	AnimationManagerFinder() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
	void apply(osg::Node& node) {
		if (_am.valid())
			return;
		if (node.getUpdateCallback()) {
			osgAnimation::AnimationManagerBase* b = dynamic_cast<osgAnimation::AnimationManagerBase*>(node.getUpdateCallback());
			if (b) {
				_am = new osgAnimation::BasicAnimationManager(*b);
				return;
			}
		}
		traverse(node);
	}
};

struct AddHelperBone : public osg::NodeVisitor
{
	AddHelperBone() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
	void apply(osg::Transform& node) {
		osgAnimation::Bone* bone = dynamic_cast<osgAnimation::Bone*>(&node);
		if (bone) {
			unsigned int numChild = bone->getNumChildren();
			for (unsigned int i = 0; i < numChild; ++i)
			{
				osg::ref_ptr<osgAnimation::Bone> childBone = dynamic_cast<osgAnimation::Bone*>(bone->getChild(i));
				osg::Vec3 offset = childBone->getMatrixInSkeletonSpace().getTrans();
				if (bone->getName() != "SkelRoot" && bone->getName() != "Root")
				{
					bone->addChild(createBody(offset));
				}
			}
		}
		traverse(node);
	}
};

OSGSkeleton::OSGSkeleton()
{
}

void OSGSkeleton::createSkeleton(string str)
{
	skeleton = dynamic_cast<osg::Group*>(osgDB::readNodeFile(str));
	AnimationManagerFinder finder;
	skeleton->accept(finder);
	if (finder._am.valid()) {
		skeleton->setUpdateCallback(finder._am.get());
		AnimtkViewerModelController::setModel(finder._am.get());
	}
	AddHelperBone add_helper;
	skeleton->getChild(0)->accept(add_helper);
}