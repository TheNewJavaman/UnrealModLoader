#include "UEHelper.h"

namespace GhostrunnerVR
{
	void UEHelper::TriggerEvent(std::wstring FunctionName, std::vector<std::wstring> FunctionArgs)
	{
		std::wstring command = FunctionName;
		for (int i = 0; i < FunctionArgs.size(); i++)
		{
			command += L" " + FunctionArgs.at(i);
		}
		if (GhostrunnerVR::Get()->ModActor)
		{
			GhostrunnerVR::Get()->ModActor->CallFunctionByNameWithArguments(command.c_str(), nullptr, NULL, true);
		}
		// Old method, which would query _every_ mod -- this is bad!
		/*
		for (int i = 0; i < Global::GetGlobals()->ModInfoList.size(); i++)
		{
			if (Global::GetGlobals()->ModInfoList[i].CurrentModActor)
			{
				Global::GetGlobals()->ModInfoList[i].CurrentModActor->CallFunctionByNameWithArguments(command.c_str(), nullptr, NULL, true);
			}
		}
		*/
	}
}