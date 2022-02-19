#pragma once

#include "Mod/Mod.h"
#include "DXManager.h"
#include "Utilities/MinHook.h"

namespace GhostrunnerVR
{
	class GhostrunnerVR : public Mod
	{
	public:
		GhostrunnerVR()
		{
			ModName = "GhostrunnerVR"; 
			ModVersion = "0.1.0";
			ModDescription = "Adds support for virtual reality gameplay";
			ModAuthors = "Gabriel Pizarro (TheNewJavaman)";
			ModLoaderVersion = "2.2.0";

			ModRef = this;
			CompleteModCreation();
		}

		virtual void InitializeMod() override;

		virtual void PostBeginPlay(std::wstring ModActorName, UE4::AActor* Actor) override;

		static GhostrunnerVR* Get();

		UE4::AActor* ModActor;
	};
}