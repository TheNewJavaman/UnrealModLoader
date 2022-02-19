#include "GhostrunnerVR.h"

namespace GhostrunnerVR 
{
	void GhostrunnerVR::InitializeMod()
	{
		UE4::InitSDK();
		SetupHooks();
		MinHook::Init();

		DXManager::Get()->HookDX();

		Log::Info("[GhostrunnerVR] Initialized mod");
	}

	void GhostrunnerVR::PostBeginPlay(std::wstring ModActorName, UE4::AActor* Actor)
	{
		std::wstring tmpModName(ModName.begin(), ModName.end());
		if (ModActorName == tmpModName)
		{
			ModActor = Actor;
		}
	}

	GhostrunnerVR* GhostrunnerVR::Get()
	{
		return static_cast<GhostrunnerVR*>(ModRef);
	}
}