#include "VRManager.h"

namespace UnrealVR
{
	bool VRManager::InitVR()
	{
		vr::EVRInitError error;
		vr::IVRSystem* pSystem = vr::VR_Init(&error, vr::VRApplication_Scene);
		if (error != vr::VRInitError_None)
		{
			Log::Error(std::format("[UnrealVR] Failed to init VR; error ({})", error));
			return false;
		}

		uint32_t width;
		uint32_t height;
		pSystem->GetRecommendedRenderTargetSize(&width, &height);
		
		// FOV should be set automatically
		/*
		float tanLeft;
		float tanRight;
		float tanTop;
		float tanBottom;
		pSystem->GetProjectionRaw(vr::Eye_Left, &tanLeft, &tanRight, &tanTop, &tanBottom);
		float fov = (std::abs(std::atan(tanLeft)) + std::atan(tanRight)) * 180.0f / 3.14159f;
		*/

		UE4::UGameplayStatics::ExecuteConsoleCommand(std::format(L"r.SetRes {}x{}f", width, height).c_str(), nullptr);

		Log::Info("[UnrealVR] Initialized VR system and compositor");
		IsVRInitialized = true;
		return true;
	}

	bool VRManager::SubmitFrame(ID3D11Texture2D* texture2D)
	{
		vr::Texture_t texture{ texture2D, vr::TextureType_DirectX, vr::ColorSpace_Gamma };
		vr::EVREye eye;
		if (LastEyeRendered == vr::Eye_Left)
		{
			eye = vr::Eye_Right;
		}
		else
		{
			eye = vr::Eye_Left;
			vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];
			vr::VRCompositor()->WaitGetPoses(poses, vr::k_unMaxTrackedDeviceCount, NULL, 0);
		}
		vr::EVRCompositorError error = vr::VRCompositor()->Submit(eye, &texture, NULL, vr::Submit_Default);
		if (error != vr::VRCompositorError_None)
		{
			Log::Error("[UnrealVR] VR compositor error (%d)", error);
			return false;
		}
		UnrealVR::Get()->ModActor->CallFunctionByNameWithArguments(std::format(L"UnrealVRSetEye {}", LastEyeRendered).c_str(), NULL, nullptr, true);
		LastEyeRendered = eye;
		return true;
	}
}