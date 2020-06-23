#include <windows.h>
#include <locale.h>
#include "detours\detours.h"
#include "log.h"
#include "main.h"
#include "ddrawhook.h"

_TrueGetAttachedSurface7 TrueGetAttachedSurface7;
_TrueCreateSurface7 TrueCreateSurface7;
_TrueDirectDrawCreateEx TrueDirectDrawCreateEx;
_TrueBlt7 TrueBlt7;

LPDIRECTDRAW7 g_lpDD7 = NULL;
LPDIRECTDRAWSURFACE7 g_lpDDSP7 = NULL;
LPDIRECTDRAWSURFACE7 g_lpDDSB7 = NULL;

extern BOOL bHackDesc;
extern BOOL bSpeedHack;
extern BOOL bView;
extern MAP* map;
extern int nSpeed;
extern int nWarpPosition;
extern int nMapIndex;

#define SCREEN_X 405
#define FONT_SIZE 13


HRESULT WINAPI HookGetAttachedSurface7(
	GUID FAR* lpGUID,
	LPDDSCAPS2 lpddscap,
	LPDIRECTDRAWSURFACE7 FAR* lpdds)
{
//	LOG(11, "HookGetAttachedSurface7\r\n");
	HRESULT h = TrueGetAttachedSurface7(lpGUID, lpddscap, lpdds);
	g_lpDDSB7 = *lpdds;
	return h;
}

HRESULT DrawText7(
	LPDIRECTDRAWSURFACE7 m_pdds, 
	TCHAR* strText,
	DWORD dwOriginX,
	DWORD dwOriginY,
	COLORREF crBackground,
	COLORREF crForeground,
	int nFontSize)
{
	HDC hDC = NULL;
	HRESULT hr;
	HFONT hFont = NULL;
	HFONT hOldFont = NULL;
	if (m_pdds == NULL || strText == NULL)
		return E_INVALIDARG;

	// Make sure this surface is restored.   
	if (FAILED(hr = m_pdds->Restore()))
		return hr;

	if (FAILED(hr = m_pdds->GetDC(&hDC)))
		return hr;

	// Set the background and foreground color   
	SetBkColor(hDC, crBackground);
	SetTextColor(hDC, crForeground);
	hFont = CreateFont(nFontSize, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0, VARIABLE_PITCH | FF_ROMAN, TEXT("����"));
	hOldFont = (HFONT)SelectObject(hDC, hFont);

	if (hFont)
		SelectObject(hDC, hFont);

	// Use GDI to draw the text on the surface   
	TextOut(hDC, dwOriginX, dwOriginY, strText, strlen(strText));
	SelectObject(hDC, hOldFont);
	DeleteObject(hFont);

	if (FAILED(hr = m_pdds->ReleaseDC(hDC)))
		return hr;

	return S_OK;
}


HRESULT __stdcall HookBlt7(GUID FAR* lpGUID,
	LPRECT lprc,
	LPDIRECTDRAWSURFACE7 lpdds,
	LPRECT lprc1,
	DWORD n,
	LPDDBLTFX n1)
{		
	if (bHackDesc)
	{
		char szSpeed[100] = { 0, };
		char szSpeed2[100] = { 0, };
		char szPoint[100] = { 0, };
		if (bSpeedHack)
		{
			sprintf_s(szSpeed, 100, "F2 : ���ǵ��� [ON]");
			sprintf_s(szSpeed2, 100, "F7 : �ӵ� ���� (%d)", nSpeed);
			DrawText7(lpdds, szSpeed2, SCREEN_X, 30, RGB(0, 0, 0), RGB(255, 255, 0), FONT_SIZE);
		}
		else
		{
			sprintf_s(szSpeed, 100, "F2 : ���ǵ��� [OFF]");
		}
	
		if (bView)
		{
			sprintf_s(szPoint, 100, "F3 : ��ǥ [ON] (Ŭ������ ���� ��)");
		}
		else
		{
			sprintf_s(szPoint, 100, "F3 : ��ǥ [OFF]");
		}
	
		DrawText7(lpdds, (char*)"F1 : ���� ON/OFF", SCREEN_X, 0, RGB(0, 0, 0), RGB(255, 216, 0), FONT_SIZE);
		DrawText7(lpdds, szSpeed, SCREEN_X, 15, RGB(0, 0, 0), RGB(255, 216, 0), FONT_SIZE);
		DrawText7(lpdds, szPoint, SCREEN_X, 45, RGB(0, 0, 0), RGB(255, 216, 0), FONT_SIZE);
		DrawText7(lpdds, (char*)"F11 : ��ǥ ���� ����", SCREEN_X, 60, RGB(0, 0, 0), RGB(255, 216, 0), FONT_SIZE);
		DrawText7(lpdds, (char*)"PAGEUP / PAGEDN : �����̵� ����", SCREEN_X, 75, RGB(0, 0, 0), RGB(255, 216, 0), FONT_SIZE);
		DrawText7(lpdds, (char*)"DEL : �����̵� ���� ĵ��", SCREEN_X, 90, RGB(0, 0, 0), RGB(255, 216, 0), FONT_SIZE);
		if (nWarpPosition >= 0)
		{
			DrawText7(lpdds, map[nMapIndex].warp[nWarpPosition].dest, SCREEN_X, 110, RGB(0, 0, 0), RGB(145, 220, 183), 25);
		}
		else
		{
			DrawText7(lpdds, map[nMapIndex].name, SCREEN_X, 110, RGB(0, 0, 0), RGB(145, 220, 183), 25);

		}
	}
	DrawText7(lpdds, (char*)"����: r (Pink) : �� ���̴ٸ� ������ ��� ��Ź�����!", 280, 410, RGB(0, 0, 0), RGB(255, 240, 211), 11);
	

	HRESULT h = TrueBlt7(lpGUID, lprc, lpdds, lprc1, n, n1);

	return h;
}

BOOL bOnce = FALSE;

HRESULT __stdcall HookCreateSurface7(
	GUID FAR* lpGUID,
	LPDDSURFACEDESC2 lpddsd,
	LPDIRECTDRAWSURFACE7 FAR* lpdds,
	IUnknown FAR* pUnkOuter)
{
	//LOG(11, "HookCreateSurface7\r\n");
	HRESULT h = TrueCreateSurface7(lpGUID, lpddsd, lpdds, pUnkOuter);
	
	if (bOnce == FALSE)
	{
		if (lpddsd->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
		{
			g_lpDDSP7 = *lpdds;
								
			TrueGetAttachedSurface7 = *(_TrueGetAttachedSurface7*)(*((DWORD*)g_lpDDSP7) + 0x30);
			TrueBlt7 = *(_TrueBlt7*)(*((DWORD*)g_lpDDSP7) + 0x14);
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&(PVOID&)TrueGetAttachedSurface7, HookGetAttachedSurface7);
			DetourAttach(&(PVOID&)TrueBlt7, HookBlt7);
			LONG error = DetourTransactionCommit();			
			bOnce = TRUE;
		}
	}

	return h;
}

HRESULT WINAPI HookDirectDrawCreateEx(GUID FAR* lpGuid, LPVOID* lplpDD, REFIID  iid, IUnknown FAR* pUnkOuter)
{
	LOG(11, "HookDirectDrawCreateEx %x\r\n", lplpDD);
	HRESULT h = TrueDirectDrawCreateEx(lpGuid, lplpDD, iid, pUnkOuter);
	
	if ((*((LPDIRECTDRAW7*)lplpDD)) != NULL && (DWORD)lplpDD == 0x8fa7d4)
	{
		// CreateSurface ��ŷ
		PROC p = NULL;
		g_lpDD7 = (LPDIRECTDRAW7)*lplpDD;
		p = *(PROC*)(*((DWORD*)g_lpDD7) + 0x18);
		TrueCreateSurface7 = (_TrueCreateSurface7)p;
			
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)TrueCreateSurface7, HookCreateSurface7);
		DetourTransactionCommit();
	}

	return h;
}