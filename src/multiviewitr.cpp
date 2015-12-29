// ----------------------------------------------------------------------------
// multipleitr.cpp : a boost of multiple-window interactive DTI voxel model
//					render supporting box-selection ROI customization. It is
//					working in multiple process modality with an IPC of shared
//					memory
//
//					::This is a GLUT-based multiple view DTI render
//
// Creation : Mar. 4th 2011
// Reestablishment: Mar. 7th 2011
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#include "mitubeRenderView.h"

int main(int argc, char** argv)
{
	CMitubeRenderView mitview(argc, argv);
	mitview.setGlutInfo(" multiple-view DTI model visualization", 
						0, 0, 10, 10);
	mitview.setVerinfo("multiviewitr");
	mitview.run();
	return 0;
}

/* set ts=4 sts=4 tw=80 sw=4 */

