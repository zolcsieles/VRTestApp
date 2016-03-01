#include "stdafx.h"

#include "VR.h"
#include "ERR.h"

vr::IVRSystem* vrSys;
vr::IVRCompositor* vrComp;
vr::Texture_t eyeTextures[2];

bool initVR()
{
	vr::HmdError hmdErr;

	vrSys = vr::VR_Init(&hmdErr, vr::VRApplication_Scene);

	switch (hmdErr)
	{
	case vr::VRInitError_Driver_HmdDisplayNotFound:
		Warning("!!!HMD DISPLAY Not found!\n");
		break;
	case vr::VRInitError_Init_HmdNotFound:
		Warning("!!HMD Not Found!\n");
		break;
	default:
		Warning("!!!There is an error (%i)\n", hmdErr);
		break;
	case vr::VRInitError_None:
		break;
	}

	if (hmdErr != vr::VRInitError_None)
		return false;

	//	vrDisp = vr::VRExtendedDisplay();
	vrComp = vr::VRCompositor();

	return true;
}

void createWindow()
{
	/*
	int32_t pX, pY;
	uint32_t W, H;
	vrDisp->GetWindowBounds(&pX, &pY, &W, &H);
	*/
}

void initTextures()
{
	uint32_t w = 0;
	uint32_t h = 0;

	//vrDisp->GetEyeOutputViewport();
	vrSys->GetRecommendedRenderTargetSize(&w, &h);

	for (int i = 0; i < 2; ++i)
	{
		eyeTextures[vr::Eye_Left].eColorSpace;
	}
}

void renderFrame()
{
	if (vrComp->CanRenderScene())
	{

		vrComp->Submit(vr::Eye_Left, &eyeTextures[vr::Eye_Left], nullptr, vr::Submit_Default);

		vrComp->Submit(vr::Eye_Right, &eyeTextures[vr::Eye_Right], nullptr, vr::Submit_Default);
	}
}

void shutdownVR()
{
	vr::VR_Shutdown();
}
