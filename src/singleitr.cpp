// ----------------------------------------------------------------------------
// singleitr.cpp : instantiate a single CTuberRender instance and boost the
//					interactive box-selection based DTI visualization render
//
// Creation : Mar. 2nd 2011
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#include "situbeRender.h"

int main(int argc, char** argv)
{
	CSitubeRender sitr(argc, argv);
	sitr.setGlutInfo(" single DTI model render ");
	sitr.setVerinfo("singleitr");
	sitr.setFullWindow();
	sitr.run();
	return 0;
}

/* set ts=4 sts=4 tw=80 sw=4 */

