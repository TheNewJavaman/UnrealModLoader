#pragma once

#include "Utilities/MinHook.h"
#include "Mod/Mod.h"

namespace UnrealVR
{
	class UnrealVR : public Mod
	{
	public:
		UnrealVR()
		{
			ModName = "UnrealVR";
			ModVersion = "0.1.0";
			ModDescription = "Adds virtual reality support to Unreal Engine 4 games";
			ModAuthors = "TheNewJavaman";
			ModLoaderVersion = "2.2.0";

			ModRef = this;
			Ref = this;
			CompleteModCreation();
		}

		virtual void InitializeMod() override;

		virtual void PostBeginPlay(std::wstring ModActorName, UE4::AActor* Actor) override;

		UE4::AActor* ModActor;

		static UnrealVR* Get();

	private:
		static UnrealVR* Ref;
	};
}