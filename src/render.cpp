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

namespace render {
	dvar_s* r_buf_dynamicIndexBuffer_mult;
	dvar_s* r_buf_dynamicVertexBuffer_mult;
	dvar_s* r_aspectratio_fix;
	uintptr_t UI_DrawHandlePic_cursor_ptr;
	int UI_DrawHandlePic_cursor(float x, float y, float width, float height, int a5, int a6) {
		if (r_aspectratio_fix && r_aspectratio_fix->value.integer >= 2) {
			float scale = (4.f / 3.f) / *(float*)gfx(0x101D4CB8);
			float original_width = width;
			width *= scale;

			x += (original_width - width) / 2.0f;
		}
		return cdecl_call<int>(UI_DrawHandlePic_cursor_ptr, x, y, width, height, a5, a6);
	}
	void SafeAreaisModified() {
		auto safeArea_horizontal = dvars::Dvar_FindVar("safeArea_horizontal");
		if (safeArea_horizontal)
			safeArea_horizontal->modified = true;

		auto safeArea_vertical = dvars::Dvar_FindVar("safeArea_vertical");

		if(safeArea_vertical)
			safeArea_horizontal->modified = true;
	}
	class component final : public component_interface
	{
	public:

		void post_start() override {
			SetProcessDPIAware();
			Memory::VP::InterceptCall(0x4C3A6E, UI_DrawHandlePic_cursor_ptr, UI_DrawHandlePic_cursor);

		}
		void post_cg_init() override {
			SafeAreaisModified();
		}
		void post_gfx() override {
			SafeAreaisModified();
			if (!r_buf_dynamicIndexBuffer_mult) {
				r_buf_dynamicIndexBuffer_mult = dvars::Dvar_RegisterFloat("r_buf_dynamicIndexBuffer_mult", 4.f, 1.f, 16.f, DVAR_ARCHIVE);
				r_buf_dynamicVertexBuffer_mult = dvars::Dvar_RegisterFloat("r_buf_dynamicVertexBuffer_mult", 4.f, 1.f, 16.f, DVAR_ARCHIVE);

				r_aspectratio_fix = dvars::Dvar_RegisterInt("r_aspectRatio_fix", 2, 0, 2, DVAR_ARCHIVE);

			}
			CreateMidHook(gfx(0x10011B60), [](SafetyHookContext& ctx) {
				auto value = (float)ctx.ecx * r_buf_dynamicIndexBuffer_mult->value.decimal;
				ctx.ecx = (uint32_t)value;
				printf("dynamic index buffer return %p ecx %d\n", *(int*)ctx.esp, ctx.ecx);
				


				});

			CreateMidHook(gfx(0x10011AD0), [](SafetyHookContext& ctx) {
				auto value = (float)ctx.ecx * r_buf_dynamicVertexBuffer_mult->value.decimal;
				ctx.ecx = (uint32_t)value;
				printf("dynamic vertex buffer return %p ecx %d\n", *(int*)ctx.esp, ctx.ecx);



				});
			Memory::VP::Patch<float>(gfx(0x1000ACEF + 1), 0.f);

			CreateMidHook(gfx(0x10011710), [](SafetyHookContext& ctx) {
				if (r_aspectratio_fix && r_aspectratio_fix->value.integer) {

					uint32_t* res = (uint32_t*)gfx(0x101D4CA8);

					Memory::VP::Patch<float>(gfx(0x101D4CB8), (float)res[0] / (float)res[1]);
					}



				});

			auto windowstyle_addr = gfx(0x10012B16);

			static auto r_noborder = dvars::Dvar_RegisterInt("r_noborder", 0, 0, 1,DVAR_ARCHIVE);

			Memory::VP::Nop(windowstyle_addr, 5);
			CreateMidHook(windowstyle_addr, [](SafetyHookContext& ctx) {

				if (r_noborder->value.integer) {
					ctx.ebp = WS_VISIBLE | WS_POPUP;
				}
				else {
					ctx.ebp = WS_VISIBLE | WS_SYSMENU | WS_CAPTION;
				}

				});

			// r_lodScale
			Memory::VP::Patch<uint32_t>(gfx(0x1000AD0C + 1), DVAR_RENDERER | DVAR_ARCHIVE);
			// r_lodBias
			Memory::VP::Patch<uint32_t>(gfx(0x1000ACE5 + 1), DVAR_RENDERER | DVAR_ARCHIVE);

		}


	};

}
REGISTER_COMPONENT(render::component);

