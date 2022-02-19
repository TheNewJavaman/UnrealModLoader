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

	void VRManager::InitVR(VRResolution* resolution)
	{
		vr::HmdError error;
		vr::IVRSystem* pSystem = vr::VR_Init(&error, vr::VRApplication_Scene);
		pSystem->GetRecommendedRenderTargetSize(&resolution->Width, &resolution->Height);
		Log::Info("[GhostrunnerVR] Initialized VR system and compositor");
		IsVRInitialized = true;
	}

	vr::EVRCompositorError VRManager::SubmitFrame(int eye, ID3D11Texture2D* texture2D)
	{
		vr::Texture_t texture = { (void*)texture2D, vr::TextureType_DirectX, vr::ColorSpace_Gamma };
		vr::VRTextureBounds_t bounds;
		bounds.uMin = 0.0f;
		bounds.uMax = 1.0f;
		bounds.vMin = 0.0f;
		bounds.vMax = 1.0f;

		vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];
		vr::VRCompositor()->WaitGetPoses(poses, vr::k_unMaxTrackedDeviceCount, NULL, 0);
		vr::EVRCompositorError error = vr::VRCompositor()->Submit(vr::Eye_Left, &texture, &bounds, vr::Submit_Default);
		if (error != vr::VRCompositorError_None)
		{
			return error;
		}
		error = vr::VRCompositor()->Submit(vr::Eye_Right, &texture, &bounds, vr::Submit_Default);
		if (error != vr::VRCompositorError_None)
		{
			return error;
		}
		return vr::VRCompositorError_None;
	}
}