#include "UnrealVR.h"

namespace UnrealVR
{
	UnrealVR* UnrealVR::Get()
	{
		return Ref;
	}

	void UnrealVR::InitializeMod()
	{
		UE4::InitSDK();
		SetupHooks();
		MinHook::Init();
	}

	void UnrealVR::PostBeginPlay(std::wstring ModActorName, UE4::AActor* Actor)
	{
		std::wstring modName(ModName.begin(), ModName.end());
		if (ModActorName == modName)
		{
			ModActor = Actor;
		}
	}
}