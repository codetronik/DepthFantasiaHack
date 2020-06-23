#include <windows.h>
#include <locale.h>
#include "detours\detours.h"
#include "log.h"
#include "main.h"
#include "ddrawhook.h"

extern _TrueDirectDrawCreateEx TrueDirectDrawCreateEx;

MAP* map;

BOOL bSpeedHack;
BOOL bTeleportHack;
BOOL bView;
BOOL bHackDesc;

int nSpeed;
int nMapIndex;
int nMapCount;
int nWarpPosition;

void PrintLogo()
{
	LOG(14, "뎁스판타지아 핵\r\n");
	LOG(12, "https://github.com/codetronik\r\n");
}
void SetPoint(char* szFilePath);

void LoadFile()
{
	nMapCount = 0;
	map = (MAP*)malloc(sizeof(MAP) * 50);
	for (int i = 0; i < 50; i++)
	{
		map[i].warp = (WARP_POINT*)malloc(sizeof(WARP_POINT) * 500);
		ZeroMemory(map[i].warp, sizeof(WARP_POINT) * 500);
	}
		
	BOOL bResult = TRUE;
	WIN32_FIND_DATA w32FindData = { 0, };
	char szCurrentDirectory[MAX_PATH] = { 0, };
	GetCurrentDirectoryA(MAX_PATH, szCurrentDirectory);
	strcat(szCurrentDirectory, "\\warp\\*.txt");
	
	HANDLE hFile = FindFirstFile(szCurrentDirectory, &w32FindData);

	while (bResult)
	{
		// 디렉토리라면,  
		if (w32FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			
		}
		else
		{			
			char szFilePath[MAX_PATH] = { 0, };
			GetCurrentDirectoryA(MAX_PATH, szCurrentDirectory);
			strcat(szFilePath, szCurrentDirectory);
			strcat(szFilePath, "\\warp\\");
			strcat(szFilePath, w32FindData.cFileName);

			char imsi[255] = { 0, };
			strcpy_s(imsi, 255, w32FindData.cFileName);
			char* p;
			if (p = strstr(imsi, ".txt"))
			{			
				*p = 0x00;
			}
			
			strcpy_s(map[nMapCount].name, 255, imsi);
			
			SetPoint(szFilePath);					
		}

		bResult = FindNextFile(hFile, &w32FindData);
	}
	CloseHandle(hFile);
}

void SetPoint(char* szFilePath)
{
	int nWarpTotal = 0;

	FILE* fp = fopen(szFilePath, "r");
	if (!fp)
	{		
		return;
	}

	char buf[512] = { 0, };
	while (!feof(fp))
	{
		fgets(buf, sizeof(buf), fp);
		if (strstr(buf, "$$$$")) continue;
		if (strstr(buf, "%%%%")) break;
		if (strstr(buf, "#")) continue;
		// 주석처리
		char* comment = strstr(buf, " //");
		if (comment)
		{
			comment = 0x0;
		}

		char* context = NULL;
		char* token = strtok_s(buf, "=", &context);
		char* token2 = strtok_s(NULL, "=", &context);
		char* token3 = strtok_s(NULL, "=", &context);
		char* token4 = strtok_s(NULL, "=", &context);
		char* token5 = strtok_s(NULL, "=", &context);
		char* token6 = strtok_s(NULL, "=", &context);
	
		token6[strlen(token6) - 1] = 0; // \n 제거
		map[nMapCount].warp[nWarpTotal].a = strtoul(token, NULL, 16);
		map[nMapCount].warp[nWarpTotal].b = strtoul(token2, NULL, 16);
		map[nMapCount].warp[nWarpTotal].c = strtoul(token3, NULL, 16);
		map[nMapCount].warp[nWarpTotal].d = strtoul(token4, NULL, 16);
		map[nMapCount].warp[nWarpTotal].e = strtoul(token5, NULL, 16);
		
		strcpy_s(map[nMapCount].warp[nWarpTotal].dest, 255, token6);	

		/*
		LOG(13, "%d %d %d %d %d : %s\r\n", 
			map[nMapCount].warp[nWarpTotal].a, map[nMapCount].warp[nWarpTotal].b,
			map[nMapCount].warp[nWarpTotal].c, map[nMapCount].warp[nWarpTotal].d,
			map[nMapCount].warp[nWarpTotal].e, map[nMapCount].warp[nWarpTotal].dest
		);
		*/
		nWarpTotal++;
	}
	fclose(fp);
	
	map[nMapCount].count = nWarpTotal;

	nMapCount++;
}

void _stdcall Teleport(DWORD* a, DWORD* b)
{
	DWORD* c = (DWORD*)0x6409C0;
	DWORD* d = (DWORD*)0x6409C4;
	DWORD* e = (DWORD*)0x6409D0;
	if (bView == TRUE)
	{
		char temp[100] = { 0, };
		sprintf_s(temp, 100, "%x=%x=%x=%x=%x\r\n", *a, *b, *c, *d, *e);
		HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, strlen(temp) + 1);
		memcpy(GlobalLock(handle), temp, strlen(temp));
		GlobalUnlock(handle);
		OpenClipboard(NULL);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, handle);
		CloseClipboard();			
	}
	if (TRUE == bTeleportHack)
	{	
		/*
		LOG(12, "teleport to %x %x %x %x %x\r\n", map[nMapIndex].warp[nWarpPosition].a,
			map[nMapIndex].warp[nWarpPosition].b,
			map[nMapIndex].warp[nWarpPosition].c,
			map[nMapIndex].warp[nWarpPosition].d,
			map[nMapIndex].warp[nWarpPosition].e
		);
		*a = map[nMapIndex].warp[nWarpPosition].a;
		*b = map[nMapIndex].warp[nWarpPosition].b;
		*c = map[nMapIndex].warp[nWarpPosition].c;
		*d = map[nMapIndex].warp[nWarpPosition].d;
		*e = map[nMapIndex].warp[nWarpPosition].e;
		*/
		bTeleportHack = FALSE;
	}
}

DWORD* g_x;
DWORD* g_y;
DWORD g_esi;
DWORD g_ebx;

PDETOUR_TRAMPOLINE pTrampoline = NULL;
__declspec(naked) void HOOK_Teleport()
{
	__asm {
		pushad
		pushfd
			
		mov g_esi, esi;
		mov g_ebx, ebx;
	}
	g_x = (DWORD*)((DWORD)g_esi + 0x218);
	g_y = (DWORD*)g_ebx;

	__asm
	{
		push g_x
		push g_y
		call Teleport
		popfd
		popad
		jmp[pTrampoline]
	}
}

// 스피드핵
typedef int(__cdecl* _S_55910)(int a1, int a2, int a3, float a4);
_S_55910 S_55910;
int __cdecl Hook_S_55910(int a1, int a2, int a3, float a4)
{
	if (TRUE == bSpeedHack)
	{
		a4 = a4 + nSpeed; // 스핵 이동거리
	}
	return S_55910(a1, a2, a3, a4);
}

void Key()
{
	while (1)
	{
		if (GetAsyncKeyState(VK_F1) & 0x0001)
		{
			if (bHackDesc == TRUE)
			{
				bHackDesc = FALSE;
			}
			else
			{
				bHackDesc = TRUE;
			}
		}
		if (GetAsyncKeyState(VK_F2) & 0x0001)
		{
			nSpeed = 0; // 속도 초기화
			if (bSpeedHack == TRUE)
			{				
				bSpeedHack = FALSE;
			}
			else
			{				
				bSpeedHack = TRUE;
			}
		}
		if (GetAsyncKeyState(VK_F7) & 0x0001)
		{
			if (bSpeedHack == TRUE)
			{
				nSpeed = nSpeed + 20; // 스피드핵 속도 증가
			}
		}	

		if (GetAsyncKeyState(VK_F11) & 0x0001)
		{
			if (nMapIndex == nMapCount -1)
			{
				nMapIndex = 0;
			}
			else
			{
				nMapIndex++;
			}
			nWarpPosition = -1;
		}

		if (GetAsyncKeyState(VK_DELETE) & 0x0001)
		{
			bTeleportHack = FALSE;
		}
		if (GetAsyncKeyState(VK_F3) & 0x0001)
		{
			if (bView == TRUE)
			{				
				bView = FALSE;
			}
			else
			{
			
				bView = TRUE;
			}
		}
		
		if (GetAsyncKeyState(VK_PRIOR) & 0x0001)
		{				
			if (nWarpPosition < map[nMapIndex].count - 1)
			{
				nWarpPosition++;
			}
		
			bTeleportHack = TRUE;
			
		}
		if (GetAsyncKeyState(VK_NEXT) & 0x0001)
		{		
			if (nWarpPosition >= 0)
			{
				nWarpPosition--;
				
				if (nWarpPosition == -1)
					bTeleportHack = FALSE;
				else
				{
					bTeleportHack = TRUE;
				}
			}		
		}

	}
}

void Hooking()
{
	if (DetourIsHelperProcess())
	{
		return;
	}

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	HMODULE hMod = LoadLibrary("ddraw.dll");
	TrueDirectDrawCreateEx = (_TrueDirectDrawCreateEx)GetProcAddress(hMod, "DirectDrawCreateEx");
	DetourAttach(&(PVOID&)TrueDirectDrawCreateEx, HookDirectDrawCreateEx);
	
	S_55910 = (_S_55910)(DWORD)0x455910;
	DetourAttach(&(PVOID&)S_55910, Hook_S_55910); // 스피드핵

	DWORD dwTeleport = 0x418F56;
	DetourAttachEx(&(PVOID&)dwTeleport, HOOK_Teleport, &pTrampoline, NULL, NULL); // 순간이동 핵

	DetourTransactionCommit();
}
void Start()
{
	HMODULE hMod = NULL;

	/*
	if (AllocConsole())
	{
		FILE* stream = NULL;
		freopen_s(&stream, "CONOUT$", "w", stdout);

		SetConsoleTitle("Hack Log Console");
		setlocale(LC_ALL, "korean");
	}
	*/
	LOG(7, "process id - %d(%x)\n", GetCurrentProcessId(), GetCurrentProcessId());
	PrintLogo();
	Hooking();
	DWORD dwThread = 0;
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Key, 0, 0, &dwThread);

	bSpeedHack = FALSE;
	bTeleportHack = FALSE;
	bView = FALSE;
	bHackDesc = TRUE;
	nWarpPosition = -1;

	LoadFile();

}


BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		DWORD dwThread;
		HANDLE hDlgThread = 0;
		hDlgThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Start, 0, 0, &dwThread);

		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}