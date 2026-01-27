#include <helper.hpp>
#include "component_loader.h"
#include <Hooking.Patterns.h>

#include <filesystem>
#include <fstream>
#include "nlohmann/json.hpp"

#include <optional>
#include <buildnumber.h>
#include "framework.h"

#include <MemoryMgr.h>

#include "dvars.h"
#include "hooking.h"
#include "GMath.h"
#include "cod2_player.h"
namespace shared {
#define va_info_size 32000
#define MAX_COD_THREADS 4
	struct va_info_t_extended
	{
		char va_string[2][va_info_size];
		int index;
	};
	va_info_t_extended new_va_info[MAX_COD_THREADS];

	char* va(char* Format, ...)
	{
		va_info_t_extended* v1; // eax
		char* v2; // esi
		unsigned int v3; // eax
		va_list ArgList; // [esp+Ch] [ebp+8h] BYREF

		va_start(ArgList, Format);
		v1 = (va_info_t_extended*)*((DWORD*)TlsGetValue(*(DWORD*)exe(0x825A6C)) + 1);
		v2 = v1->va_string[v1->index];
		v1->index = (v1->index + 1) % 2;
		v3 = vsnprintf(v2, va_info_size, Format, ArgList);
		v2[va_info_size - 1] = 0;
		if (v3 >= va_info_size)
			Com_Error(1, "Attempted to overrun string by %d in call to va()",v3);
		return v2;
	}

	void Init() {
		if (!exe(1))
			return;
		Memory::VP::Patch<void*>(exe(0x43FA19 + 2), new_va_info);
		Memory::VP::Patch<uint32_t>(exe(0x43FA0B + 2), sizeof(va_info_t_extended));

		Memory::VP::InjectHook(exe(0x43F990), va, Memory::VP::HookType::Jump);
	}

}