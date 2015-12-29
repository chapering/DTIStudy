// ----------------------------------------------------------------------------
// multipleitr.cpp : a boost of multiple-window interactive DTI voxel model
//					render supporting box-selection ROI customization. It is
//					working in multiple process modality with an IPC of shared
//					memory
//
//					::This is a GLUT-based multiple window DTI render
//
// Creation : Mar. 4th 2011
// Reestablishment : Mar. 7th 2011
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#include "mitubeRenderWindow.h"

int main(int argc, char** argv)
{
	CMitubeRenderWindow mitwindow(argc, argv);
	mitwindow.setGlutInfo(" multiple-window DTI model visualization", 
						0, 0, 10, 10);
	mitwindow.setVerinfo("multiwindowitr");
	mitwindow.run();
	return 0;
}

/* set ts=4 sts=4 tw=80 sw=4 */

