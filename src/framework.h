#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#define LIBRARY "DDRAW.dll"
#include "GMath.h"
#define LIBRARYW TEXT(LIBRARY)
// cdecl
template<typename Ret = void, typename... Args>
inline Ret cdecl_call(uintptr_t addr, Args... args) {
    return reinterpret_cast<Ret(__cdecl*)(Args...)>(addr)(args...);
}

// stdcall
template<typename Ret = void, typename... Args>
inline Ret stdcall_call(uintptr_t addr, Args... args) {
    return reinterpret_cast<Ret(__stdcall*)(Args...)>(addr)(args...);
}

// fastcall
template<typename Ret = void, typename... Args>
inline Ret fastcall_call(uintptr_t addr, Args... args) {
    return reinterpret_cast<Ret(__fastcall*)(Args...)>(addr)(args...);
}

// thiscall
template<typename Ret = void, typename... Args>
inline Ret thiscall_call(uintptr_t addr, Args... args) {
    return reinterpret_cast<Ret(__thiscall*)(Args...)>(addr)(args...);
}
#define BASE_GFX 0x10000000
uintptr_t gfx(uintptr_t sp, uintptr_t mp = 0);
uintptr_t exe(uintptr_t sp, uintptr_t mp = 0);

#ifndef COD2_DEFINITIONS_H
#define COD2_DEFINITIONS_H

#define qboolean int
#define qtrue 1
#define qfalse 0

typedef unsigned char byte;
typedef float vec_t;
typedef vector2 vec2_t;
typedef vector3 vec3_t;
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];

#define PITCH               0       // up / down
#define YAW                 1       // left / right
#define ROLL                2       // fall over

#endif

#define MOD_NAME "CoD2-YAP"

#define ADDR(addr_win, addr_linux) (addr_win)
// Choose between Windows (W) or Linux (L) code
#define WL(win, linux) win


union itemDefData_t
{
	uint32_t listBox;
	uint32_t editField;
	uint32_t multi;
	uint32_t enumDvarName;
	uint32_t data;
};


struct rectDef_s
{
	float x;
	float y;
	float w;
	float h;
	uint32_t horzAlign = 0;
	uint32_t vertAlign = 0;


	bool allFloatsSame(float epsilon = 1e-6f) const {
		return (std::abs(x - y) < epsilon &&
			std::abs(x - w) < epsilon &&
			std::abs(x - h) < epsilon);
	}

	bool floatsEqual(const rectDef_s& other, float epsilon = 1e-6f) const {
		return (std::abs(x - other.x) < epsilon &&
			std::abs(y - other.y) < epsilon &&
			std::abs(w - other.w) < epsilon &&
			std::abs(h - other.h) < epsilon);
	}
};

struct windowDef_t
{
	rectDef_s rect[4];
	rectDef_s rectClient[4];
	const char* name;
/*	char rect[96];
	char rectClient[96]*/;
	const char* group;
	const char* cinematicName;
	const char* cinematic;
	uint32_t style;
	uint32_t border;
	uint32_t ownerDraw;
	uint32_t ownerDrawFlags;
	uint32_t borderSize;
	uint32_t staticFlags;
	char dynamicFlags[16];
	char rectEffects0[96];
	char rectEffects1[96];
	char offsetTime[16];
	uint32_t nextTime;
	char foreColor[16];
	char backColor[16];
	char borderColor[16];
	char outlineColor[16];
	uint32_t background;
};

struct menuDef_t;

struct itemDef_s
{
	//char window[528];
	windowDef_t window;
	//char textRect[96];
	rectDef_s textRect[4];
	uint32_t type;
	uint32_t dataType;
	uint32_t alignment;
	uint32_t fontEnum;
	uint32_t textalignment;
	uint32_t textalignx;
	uint32_t textaligny;
	uint32_t textscale;
	uint32_t textStyle;
	const char* text;
	uint32_t textSavegameInfo;
	menuDef_t *parent;
	const char* mouseEnterText;
	const char* mouseExitText;
	const char* mouseEnter;
	const char* mouseExit;
	const char* action;
	const char* onAccept;
	const char* onFocus;
	const char* leaveFocus;
	const char* dvar;
	const char* dvarTest;
	const char* onKey;
	const char* enableDvar;
	uint32_t dvarFlags;
	uint32_t focusSound;
	uint32_t special;
	char cursorPos[16];
	itemDefData_t typeData;
	uint32_t imageTrack;
};

struct menuDef_t
{
	windowDef_t window;
	int32_t font;
	int32_t fullScreen;
	uint32_t itemCount;
	uint32_t fontIndex;
	uint8_t cursorItem[16];
	int32_t fadeCycle;
	int32_t fadeClamp;
	int32_t fadeAmount;
	int32_t fadeInAmount;
	int32_t blurRadius;
	const char* onOpen;
	const char* onClose;
	const char* onESC;
	const char* onKey;
	char soundName[4];
	int32_t imageTrack;
	vec4_t focusColor;
	vec4_t disableColor;
	itemDef_s** items;
};

struct snd_volume_info_t
{
	float volume;
	float goalvolume;
	float goalrate;
};

struct __declspec(align(4)) snd_channelvolgroup
{
	snd_volume_info_t channelvol[11];
	uint8_t active;
};

struct snd_background_info_t
{
	float goalvolume;
	float goalrate;
};

struct __declspec(align(4)) snd_enveffect
{
	int roomtype;
	float drylevel;
	float drygoal;
	float dryrate;
	float wetlevel;
	float wetgoal;
	float wetrate;
	bool active;
};

struct orientation_t
{
	float origin[3];
	float axis[3][3];
};

struct __declspec(align(4)) snd_listener
{
	orientation_t orient;
	int clientNum;
	bool active;
};

enum snd_alias_system_t : __int32
{
	SASYS_UI = 0x0,
	SASYS_CGAME = 0x1,
	SASYS_GAME = 0x2,
	SASYS_COUNT = 0x3,
};

struct snd_alias_t
{
	const char* pszAliasName;
	const char* pszSubtitle;
	const char* pszSecondaryAliasName;
	uint32_t soundFile;
	uint32_t iSequence;
	float fVolMin;
	float fVolMax;
	float fPitchMin;
	float fPitchMax;
	float fDistMin;
	float fDistMax;
	uint32_t flags;
	float fSlavePercentage;
	float fProbability;
	float fLfePercentage;
	uint32_t startDelay;
	uint32_t volumeFalloffCurve;
};

struct snd_channel_info_t
{
	uint32_t entnum;
	uint32_t entchannel;
	uint32_t startDelay;
	uint32_t looptime;
	uint32_t endtime;
	float basevolume;
	uint32_t baserate;
	float pitch;
	uint32_t srcChannelCount;
	snd_alias_t* pAlias0;
	snd_alias_t* pAlias1;
	float lerp;
	float org[3];
	float offset[3];
	uint8_t paused;
	uint8_t master;
	char pad0[2];
	snd_alias_system_t system;
};


struct snd_local_t
{
	bool Initialized2d;
	bool Initialized3d;
	bool paused;
	char pad0[1];
	uint32_t playback_rate;
	uint32_t playback_bits;
	uint32_t playback_channels;
	uint32_t timescale;
	int pausetime;
	int cpu;
	char pad1[8];
	float volume;
	snd_volume_info_t mastervol;
	snd_channelvolgroup channelVolGroups[5];
	snd_channelvolgroup* channelvol;
	//char background[24];
	snd_background_info_t background[3]; // dont make sense but ok 
	uint32_t ambient_track;
	uint32_t slaveLerp;
	//char envEffects[96];
	snd_enveffect envEffects[3];
	snd_enveffect* effect;
	bool defaultPauseSettings[11];
	bool pauseSettings[11];
	snd_listener listeners[1]; // only one ahh cause no splitscreen ?
	uint32_t time;
	uint32_t looptime;
	snd_channel_info_t chaninfo[53];
	uint32_t max_2D_channels;
	uint32_t max_3D_channels;
	uint32_t max_stream_channels;
};

typedef enum
{
	SAT_UNKNOWN = 0,
	SAT_LOADED = 1,
	SAT_STREAMED = 2,
	SAT_PRIMED = 3,
	SAT_COUNT = 4
} snd_alias_type_t;

struct snd_alias_build_s
{
	char szSourceFile[64];
	char szAliasName[64];
	char szSecondaryAliasName[64];
	uint32_t subtitleText;
	uint32_t iSequence;
	char szSoundFile[64];
	uint32_t permSoundFile;
	float fVolMin;
	float fVolMax;
	float fVolMod;
	float fPitchMin;
	float fPitchMax;
	float fDistMin;
	float fDistMax;
	uint32_t iChannel;
	snd_alias_type_t eType;
	uint32_t volumeFalloffCurve;
	float fSlavePercentage;
	float fProbability;
	float fLfePercentage;
	uint32_t startDelay;
	uint8_t bLooping;
	uint8_t bMaster;
	uint8_t bSlave;
	uint8_t bFullDryLevel;
	uint8_t bNoWetLevel;
	uint8_t error;
	uint8_t keep;
	char pad0[1];
	uint32_t pSameSoundFile;
	uint32_t pNext;
};

inline bool matchesAny(const char* str, std::initializer_list<const char*> options) {
	for (const char* option : options) {
		if (strcmp(str, option) == 0) {
			return true;
		}
	}
	return false;
}