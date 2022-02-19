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
	}
}