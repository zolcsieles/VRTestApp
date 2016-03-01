#pragma once

#include "Config.h"

#ifdef USE_OPENVR
#include <openvr.h>

extern vr::IVRSystem* vrSys;
extern vr::IVRCompositor* vrComp;
extern vr::Texture_t eyeTextures[2];

bool initVR();
void initTextures();
void renderFrame();
void shutdownVR();

#else //USE_OSVR
#endif
