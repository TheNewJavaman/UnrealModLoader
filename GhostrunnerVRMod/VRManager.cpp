#include "VRManager.h"

namespace GhostrunnerVR
{
	VRManager* VRManager::Ref;

	VRManager* VRManager::Get()
	{
		if (!Ref)
		{
			Ref = new VRManager();
		}
		return Ref;
	}

	void VRManager::InitVR()
	{
		vr::HmdError error;
		vr::IVRSystem* pSystem = vr::VR_Init(&error, vr::VRApplication_Scene);

		uint32_t width;
		uint32_t height;
		pSystem->GetRecommendedRenderTargetSize(&width, &height);
		float tanLeft;
		float tanRight;
		float tanTop;
		float tanBottom;
		pSystem->GetProjectionRaw(vr::Eye_Left, &tanLeft, &tanRight, &tanTop, &tanBottom);
		float fov = (std::abs(std::atan(tanLeft)) + std::atan(tanRight)) * 180.0f / 3.14159f;
		UEHelper::TriggerEvent(L"GhostrunnerVRSettings", { std::to_wstring(width), std::to_wstring(height), std::to_wstring(fov), std::to_wstring(6.35f) });

		Log::Info("[GhostrunnerVR] Initialized VR system and compositor");
		IsVRInitialized = true;
	}

	vr::EVRCompositorError VRManager::SubmitFrame(ID3D11Texture2D* texture2D)
	{
		vr::Texture_t texture = { (void*)texture2D, vr::TextureType_DirectX, vr::ColorSpace_Gamma };
		vr::VRTextureBounds_t bounds;
		bounds.uMin = 0.0f;
		bounds.uMax = 1.0f;
		bounds.vMin = 0.0f;
		bounds.vMax = 1.0f;

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
		vr::EVRCompositorError error = vr::VRCompositor()->Submit(eye, &texture, &bounds, vr::Submit_Default);
		if (error != vr::VRCompositorError_None)
		{
			return error;
		}
		UEHelper::TriggerEvent(L"GhostrunnerVRSetEye", { std::to_wstring(LastEyeRendered) });
		LastEyeRendered = eye;
		return vr::VRCompositorError_None;
	}
}