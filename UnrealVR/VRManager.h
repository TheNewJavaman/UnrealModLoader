#pragma once

#include <format>
#include <d3d11.h>
#include "openvr.h"
#include "Ue4.hpp"
#include "UnrealVR.h"

namespace UnrealVR
{
	class VRManager
	{
	public:
		static bool InitVR();
		static bool SubmitFrame(ID3D11Texture2D* texture2D);

		static bool IsVRInitialized;
		static vr::EVREye LastEyeRendered;
	};
}