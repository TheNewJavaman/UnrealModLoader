#pragma once

#include "Ue4.hpp"
#include "GhostrunnerVR.h"

namespace GhostrunnerVR
{
	class UEHelper
	{
	public:
		static void TriggerEvent(std::wstring FunctionName, std::vector<std::wstring> FunctionArgs);
	};
}