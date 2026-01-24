// --------------------------------------+
// System Dynamic Link Library Proxy
// by momo5502
// --------------------------------------+
//#include "StdInc.h"

#include "framework.h"
#include <helper.hpp>
#include <map>


// Static class
// --------------------------------------+
class SDLLP
{
private:
	static std::map<std::string, HINSTANCE> mLibraries;
	static const char* mTargetLibrary;
	static void	Log(const char* message, ...);
	static void	LoadLibrary(const char* library);
	static bool	IsLibraryLoaded(const char* library);
	static const char* DetermineLibrary();
	static void GetModuleDirectory(CHAR* path, size_t size);
public:
	static FARPROC GetExport(const char* function);
	static const char* GetTargetLibrary();
};

//	Class variable declarations
// --------------------------------------+
std::map<std::string, HINSTANCE> SDLLP::mLibraries;
const char* SDLLP::mTargetLibrary = nullptr;

// Get the directory of the current module
// --------------------------------------+
void SDLLP::GetModuleDirectory(CHAR* path, size_t size)
{
	GetModuleFileNameA(NULL, path, size);
	// Remove the executable filename to get just the directory
	char* lastSlash = strrchr(path, '\\');
	if (lastSlash) *(lastSlash + 1) = '\0';
}

// Determine which library to proxy
// --------------------------------------+
const char* SDLLP::DetermineLibrary()
{
	if (mTargetLibrary) return mTargetLibrary;

	Log("[SDLLP] Determining target library.");

	// Get the name of the current DLL
	CHAR currentDllPath[MAX_PATH];
	HMODULE hModule = NULL;

	// Get the handle to THIS DLL
	GetModuleHandleExA(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCSTR)&DetermineLibrary,
		&hModule
	);

	GetModuleFileNameA(hModule, currentDllPath, MAX_PATH);

	// Extract just the filename
	char* fileName = strrchr(currentDllPath, '\\');
	if (fileName) fileName++; // Move past the backslash
	else fileName = currentDllPath;

	// Remove the .dll extension to get base name
	char baseName[MAX_PATH];
	strcpy_s(baseName, fileName);
	char* dotPos = strrchr(baseName, '.');
	if (dotPos) *dotPos = '\0';

	// Construct the "Hooked" variant name
	char hookedName[MAX_PATH];
	sprintf_s(hookedName, "%sHooked.dll", baseName);

	Log("[SDLLP] Current DLL: %s, checking for hooked variant: %s", fileName, hookedName);

	// Check for the hooked version in the game directory
	CHAR hookedPath[MAX_PATH];
	GetModuleDirectory(hookedPath, MAX_PATH);
	strcat_s(hookedPath, hookedName);

	if (GetFileAttributesA(hookedPath) != INVALID_FILE_ATTRIBUTES)
	{
		Log("[SDLLP] %s found in game directory, using hooked variant.", hookedName);
		mTargetLibrary = _strdup(hookedName);
		return mTargetLibrary;
	}

	Log("[SDLLP] %s not found, falling back to %s.", hookedName, fileName);

	mTargetLibrary = _strdup(fileName);
	return mTargetLibrary;
}

// Load necessary library
// --------------------------------------+
void SDLLP::LoadLibrary(const char* library)
{
	Log("[SDLLP] Loading library '%s'.", library);
	CHAR mPath[MAX_PATH];

	// Check if this is a "Hooked" variant (ends with "Hooked.dll") - case insensitive
	size_t len = strlen(library);
	bool isHooked = (len > 10 && _stricmp(library + len - 10, "Hooked.dll") == 0);

	if (isHooked)
	{
		// For hooked variants, load from game directory
		GetModuleDirectory(mPath, MAX_PATH);
		strcat_s(mPath, library);
	}
	else
	{
		// For standard system libraries, load from system directory
		GetSystemDirectoryA(mPath, MAX_PATH);
		strcat_s(mPath, "\\");
		strcat_s(mPath, library);
	}

	mLibraries[library] = ::LoadLibraryA(mPath);
	if (!IsLibraryLoaded(library)) Log("[SDLLP] Unable to load library '%s'.", library);
}

// Check if export already loaded
// --------------------------------------+
bool SDLLP::IsLibraryLoaded(const char* library)
{
	return (mLibraries.find(library) != mLibraries.end() && mLibraries[library]);
}

const char* SDLLP::GetTargetLibrary()
{
	return DetermineLibrary();
}

// Get export address
// --------------------------------------+
FARPROC SDLLP::GetExport(const char* function)
{
	const char* library = GetTargetLibrary();
	Log("[SDLLP] Export '%s' requested from %s.", function, library);
	if (!IsLibraryLoaded(library)) LoadLibrary(library);
	FARPROC address = GetProcAddress(mLibraries[library], function);
	if (!address) Log("[SDLLP] Unable to load export '%s' from library '%s'.", function, library);
	return address;
}

// Write debug string
// --------------------------------------+
void SDLLP::Log(const char* message, ...)
{
	CHAR buffer[1024];
	va_list ap;
	va_start(ap, message);
	vsprintf_s(buffer, message, ap);
	va_end(ap);
	OutputDebugStringA(buffer);
}

// Macro to declare an export
// --------------------------------------+
#define EXPORT(_export) extern "C" __declspec(naked) __declspec(dllexport) void _export() { static FARPROC function = 0; if(!function) function = SDLLP::GetExport(__FUNCTION__); __asm { jmp function } }  

// --------------------------------------+
//	Export functions
// --------------------------------------+
EXPORT(D3DPERF_BeginEvent)
EXPORT(D3DPERF_EndEvent)
EXPORT(Direct3DCreate9)

EXPORT(WSAGetLastError)
EXPORT(inet_addr)
EXPORT(gethostbyname)

EXPORT(timeBeginPeriod)
EXPORT(timeGetTime)
EXPORT(timeEndPeriod)