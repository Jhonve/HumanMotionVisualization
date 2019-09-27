#include <stdio.h>
#include <iostream>

#include "PoseMotionViewer.h"

int main()
{
	PoseMotionViewer* pose_motion_viewer = new PoseMotionViewer();
	pose_motion_viewer->m_viewer->run();
	std::cout << "Test." << std::endl;
	system("pause");
	return 0;
}