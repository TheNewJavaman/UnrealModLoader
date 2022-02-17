#include "Utilities/Globals.h"

namespace UE4Helpers
{
	static void TriggerEvent(std::wstring FunctionName, std::vector<std::wstring> FunctionArgs)
	{
		std::wstring command = FunctionName;
		for (int i = 0; i < FunctionArgs.size(); i++)
		{
			command += L" " + FunctionArgs.at(i);
		}
		for (int i = 0; i < Global::GetGlobals()->ModInfoList.size(); i++)
		{
			if (Global::GetGlobals()->ModInfoList[i].CurrentModActor)
			{
				Global::GetGlobals()->ModInfoList[i].CurrentModActor->CallFunctionByNameWithArguments(command.c_str(), nullptr, NULL, true);
			}
		}
	}
}