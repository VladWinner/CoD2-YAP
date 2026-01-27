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
namespace xanim {
	dvar_s* yap_xanim_iw3_float;
	bool do_transitionTime = false;
	SAFETYHOOK_NOINLINE void ehf(SafetyHookContext& ctx) {
		auto ps = *(playerState_t**)(ctx.esp + 0x14);
		//printf("weapon anim 0x%X\n", ps->weapAnim);
		//if ((ps->weapAnim & 1) == 0) {
		//	return;
		//}
		if(yap_xanim_iw3_float->value.decimal != 0.f)
			do_transitionTime = true;





	}
	class component final : public component_interface
	{
	public:
		void post_unpack() override {
			yap_xanim_iw3_float = dvars::Dvar_RegisterFloat("yap_xanim_iw3_transitionTime", 0.5f, 0.f, 1.f,DVAR_ARCHIVE,"Time for transition time in WeaponRunXModelAnims for when xanim hasn't finished\nThis does the smoothed out weapon anim cancel in IW3+");
		}
		void post_start() override {
			static auto eh = safetyhook::create_mid(exe(0x4B29AD,0x4D3A1D), ehf);

			static auto XAnimSetGoalWeight1 = safetyhook::create_mid(exe(0x4B2588,0x4D36BC), [](SafetyHookContext& ctx) {
				if (do_transitionTime) {
					*(float*)(ctx.esp) = yap_xanim_iw3_float->value.decimal;
				}
				});

			static auto XAnimSetGoalWeight2 = safetyhook::create_mid(exe(0x4B25B7,0x4D36EA), [](SafetyHookContext& ctx) {
				if (do_transitionTime) {
					*(float*)(ctx.esp) = yap_xanim_iw3_float->value.decimal;
				}
				});

			static auto StartWeaponAnim_done = safetyhook::create_mid(exe(0x4B25DA,0x4D3713), [](SafetyHookContext& ctx) {

				do_transitionTime = false;

				});

		}


	};


}
REGISTER_COMPONENT(xanim::component);