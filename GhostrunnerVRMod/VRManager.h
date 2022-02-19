#pragma once

#define XR_USE_PLATFORM_WIN32
#define XR_USE_GRAPHICS_API_D3D11

#include <vector>
#include <d3d11.h>
#include "openvr/openvr.h"
#include "GhostrunnerVR.h"
#pragma comment(lib, "d3d11.lib")

namespace GhostrunnerVR
{
	struct VRResolution
	{
		unsigned int Width;
		unsigned int Height;
	};

	class VRManager
	{
	public:
		void InitVR(VRResolution* resolution);
		vr::EVRCompositorError SubmitFrame(int eye, ID3D11Texture2D* texture2D);

		bool IsVRInitialized;
		vr::EVREye LastEyeRendered = vr::Eye_Right;

		static VRManager* Get();

	private:
		static VRManager* Ref;
	};
}