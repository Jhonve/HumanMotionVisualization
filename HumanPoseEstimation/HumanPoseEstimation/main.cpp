#include <stdio.h>
#include <iostream>

#include "PoseMotionViewer.h"

int main()
{
	int win_x = 120;	int win_y = 20;
	int width = 1024;	int height = 1024;
	PoseMotionViewer* pose_motion_viewer = new PoseMotionViewer(win_x, win_y, width, height);
	pose_motion_viewer->m_viewer->run();
	return 0;
}