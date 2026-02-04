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
#include "..\hooking.h"
#include "game.h"
#include "dvars.h"
namespace gsc {

	std::unordered_map<std::string, game::scr_function_t> CustomScrFunctions;
	std::unordered_map<const char*, const char*> ReplacedFunctions;
	const char* ReplacedPos = nullptr;


	game::scrVmPub_t* gScrVmPub = (game::scrVmPub_t*)0xA01880;

	void AddFunction(const char* name, game::xfunction_t func, int type = 0)
	{
		game::scr_function_t toAdd;
		toAdd.function = func;
		toAdd.developer = type;

		CustomScrFunctions.insert_or_assign(name, toAdd);
	}
	SafetyHookInline Scr_GetFunctionD;
	game::xfunction_t Scr_GetFunction_Stub(const char** name, int* isDev)
	{
		auto got = CustomScrFunctions.find(*name);

		if (got == CustomScrFunctions.end())
			return Scr_GetFunctionD.unsafe_ccall<game::xfunction_t>(name, isDev);

		*isDev = got->second.developer;
		return got->second.function;
	}

	const char* GetCodePosForParam(int index)
	{
		if (static_cast<unsigned int>(index) >= gScrVmPub->outparamcount)
		{
			printf("errpr");
			return "";
		}

		const auto* value = &gScrVmPub->top[-index];

		if (value->type != game::VAR_FUNCTION)
		{
			printf("errpr2");
			return "";
		}

		return value->u.codePosValue;
	}

	void SetReplacedPos(const char* what, const char* with)
	{
		if (!*what || !*with)
		{
			Com_Printf(0, "Invalid parameters passed to ReplacedFunctions\n");
			return;
		}

		if (ReplacedFunctions.contains(what))
		{
			Com_Printf(0, "ReplacedFunctions already contains codePosValue for a function\n");
		}

		ReplacedFunctions[what] = with;
	}

	void GetReplacedPos(const char* pos)
	{
		if (!pos)
		{
			// This seems to happen often and there should not be pointers to NULL in our map
			return;
		}



		if (const auto itr = ReplacedFunctions.find(pos); itr != ReplacedFunctions.end())
		{
			ReplacedPos = itr->second;

			//auto lastCall = (gScrVmPub->function_frame[-(int)gScrVmPub->function_count].fs.pos) - 8;
			//if (ReplacedPos == lastCall)
			//	ReplacedPos = 0;
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override {
			if (!exe(1))
				return;

		}
		void post_start() override {
			if (!exe(1))
				return;

			Scr_GetFunctionD = safetyhook::create_inline(exe(0x4F8B60), Scr_GetFunction_Stub);
			static auto SV_ShutdownGameVM = safetyhook::create_mid(exe(0x4489A0), [](SafetyHookContext& ctx) {

				int clearScripts = ctx.edi;

				if (clearScripts) {
					ReplacedFunctions.clear();
				}

				});

			gsc::AddFunction("replacefunc", [] // gsc: ReplaceFunc(<function>, <function>)
				{


					const auto what = GetCodePosForParam(0);
					const auto with = GetCodePosForParam(1);

					SetReplacedPos(what, with);
				});

			static auto VM_ExecuteInternal_midhook = safetyhook::create_mid(exe(0x469451), [](SafetyHookContext& ctx) {

				const char* pos = (const char*)(ctx.esi);

				//auto pos = gScrVmPub->function_frame->fs.pos;

				GetReplacedPos(pos);
				if (ReplacedPos) {

					printf("pos ptr1 %p ptr2 %p replaced %p og: %p %d\n", gScrVmPub->function_frame[0].fs.pos, gScrVmPub->function_frame[-(int)gScrVmPub->function_count].fs.pos, ReplacedPos, ctx.esi,gScrVmPub->function_count);
					//printf("found a ReplcaedPos!!!\n");
					ctx.esi = (uintptr_t)ReplacedPos;
					ReplacedPos = nullptr;
				}

				});

		}


	};
}
REGISTER_COMPONENT(gsc::component);