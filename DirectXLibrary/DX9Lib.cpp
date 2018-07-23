#pragma once

#include <windows.h>
#include <stdio.h>
#include<d3dx9.h>
#include <dinput.h>
#include <memory.h>
#include "DX9Lib.h"

#pragma comment(lib,"d3dx9d.lib")
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"winmm.lib")

LPDIRECT3D9 g_pDirect3D = NULL;
LPDIRECT3DDEVICE9 g_pDirect3DDevice = NULL;
LPDIRECTINPUT8 g_pDirectInput = NULL;
LPDIRECTINPUTDEVICE8 g_pDirectInputDevice[KEY_AND_MOUSE] = { NULL,NULL };
LPD3DXFONT g_pDirect3DXFont = NULL;
D3DPRESENT_PARAMETERS g_Direct3DPresentParameters;

KeyState g_keyState;
MouseState g_mouseState;

INT CreateWindowAndRepeatToControlAndRender(HINSTANCE hInst, const CHAR *appName, VOID(*func)(VOID), INT displayWidth, INT displayHeight,BOOL cullPolygon)
{
	HWND hWnd = NULL;
	MSG	msg;

	CreateWindowOverall(&hWnd, &msg, hInst, appName, displayWidth, displayHeight);

	if(FAILED(InitDirect3D()))
	{ 
		return FALSE;
	}

	if (FAILED(InitDirect3DDevice(hWnd, cullPolygon)))
	{
		return FALSE;
	}

	if (FAILED(InitDinput(hWnd)))
	{
		return FALSE;
	}

	while (msg.message != WM_QUIT)
	{
		if (ConfirmMessage(&msg))
		{
			CoordinateFPS(UPDATE_TIME);

			if (CoordinateFPS(CHECK_FPS))
			{
				GetKeyInfo();
				GetMouseInfo();
				PrepareRender();
				(*func)();
				CleanUpRender();
				UpdatePrevKeyInfo();
				UpdatePrevMouseInfo();
			}
		}
	}

	return(INT)msg.wParam;
}

VOID CreateWindowOverall(HWND *hWnd, MSG *msg, HINSTANCE hInst, const CHAR *appName, INT displayWidth, INT displayHeight)
{
	WNDCLASSEX wndclass;

	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInst;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = appName;
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&wndclass);

	(*hWnd) = CreateWindow(appName, appName, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, displayWidth, displayHeight, NULL, NULL, hInst, NULL);

	ShowWindow((*hWnd), SW_SHOW);
	UpdateWindow((*hWnd));

	ZeroMemory(msg, sizeof(*msg));

	return;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)//////////////////////////////////////////////////////////////////////////////////
{
	switch (iMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

INT ConfirmMessage(MSG *msg)
{
	if (PeekMessage(msg, NULL, 0U, 0U, PM_REMOVE))
	{
		TranslateMessage(msg);

		DispatchMessage(msg);

		return FALSE;
	}

	return TRUE;
}

INT CoordinateFPS(INT action, UINT frame)
{
	timeBeginPeriod(1);

	static DWORD prevTime = timeGetTime();
	static DWORD cuurentTime;

	switch (action)
	{
	case CHECK_FPS:

		if (cuurentTime - prevTime >= 1000 / frame)
		{
			prevTime = timeGetTime();

			timeEndPeriod(1);

			return TRUE;
		}

		break;

	case UPDATE_TIME:

		cuurentTime = timeGetTime();

		break;
	}

	timeEndPeriod(1);

	return FALSE;
}

HRESULT InitDirect3D(VOID)
{
	if (NULL == (g_pDirect3D = Direct3DCreate9(D3D_SDK_VERSION)))
	{
		MessageBox(0, "Direct3Dの作成に失敗しました", "", MB_OK);
		return E_FAIL;
	}

	ZeroMemory(&g_Direct3DPresentParameters, sizeof(g_Direct3DPresentParameters));

	SetBuckBufferOverall();

	g_Direct3DPresentParameters.Windowed = TRUE;

	return S_OK;
}

VOID SetBuckBufferOverall(VOID)
{
	g_Direct3DPresentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	g_Direct3DPresentParameters.BackBufferCount = 1;
	g_Direct3DPresentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;

	return;
}

HRESULT InitDirect3DDevice(HWND hWnd,BOOL cullPolygon)
{
	if (FAILED(g_pDirect3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_MIXED_VERTEXPROCESSING,&g_Direct3DPresentParameters, &g_pDirect3DDevice)))
	{
		MessageBox(0, "HALモードでDIRECT3Dデバイスを作成できません\nREFモードで再試行します", NULL, MB_OK);
		if (FAILED((g_pDirect3D)->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd,
			D3DCREATE_MIXED_VERTEXPROCESSING,
			&g_Direct3DPresentParameters, &g_pDirect3DDevice)))
		{
			MessageBox(0, "DIRECT3Dデバイスの作成に失敗しました", NULL, MB_OK);
			return E_FAIL;
		}
	}

	SetRenderStateOverall(cullPolygon);

	SetTextureStageStateOverall();

	g_pDirect3DDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

	return S_OK;
}

VOID SetRenderStateOverall(BOOL cullPolygon)
{
	g_pDirect3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	g_pDirect3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_pDirect3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	
	if (!cullPolygon)
	{
		g_pDirect3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	}

	return;
}

VOID SetTextureStageStateOverall(VOID)
{
	g_pDirect3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	g_pDirect3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	g_pDirect3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	g_pDirect3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	return;
}

HRESULT InitDinput(HWND hWnd)
{
	HRESULT hr;

	if (FAILED(hr = DirectInput8Create(GetModuleHandle(NULL),
		DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&g_pDirectInput, NULL)))
	{
		return hr;
	}

	if (FAILED(hr = g_pDirectInput->CreateDevice(GUID_SysKeyboard,
		&(g_pDirectInputDevice[KEY]), NULL)))
	{
		return hr;
	}

	if (FAILED(hr = (g_pDirectInputDevice[KEY])->SetDataFormat(&c_dfDIKeyboard)))
	{
		return hr;
	}

	if (FAILED(hr = (g_pDirectInputDevice[KEY])->SetCooperativeLevel(
		hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)))
	{
		return hr;
	}

	memset(&g_keyState,NULL,sizeof(KeyState));

	if (FAILED(hr = g_pDirectInput->CreateDevice(GUID_SysMouse,
		&(g_pDirectInputDevice[MOUSE]), NULL)))
	{
		return hr;
	}

	if (FAILED(hr = (g_pDirectInputDevice[MOUSE])->SetDataFormat(&c_dfDIMouse)))
	{
		return hr;
	}

	if (FAILED(hr = (g_pDirectInputDevice[MOUSE])->SetCooperativeLevel(
		hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)))
	{
		return hr;
	}

	DIPROPDWORD diprop;
	diprop.diph.dwSize = sizeof(diprop);
	diprop.diph.dwHeaderSize = sizeof(diprop.diph);
	diprop.diph.dwObj = 0;
	diprop.diph.dwHow = DIPH_DEVICE;
	diprop.dwData = DIPROPAXISMODE_REL;

	if (FAILED(hr = (g_pDirectInputDevice[MOUSE])->SetProperty(DIPROP_AXISMODE, &diprop.diph)))
	{
		return hr;
	}

	memset(&g_mouseState, 0, sizeof(MouseState));

	return S_OK;
}

VOID CustomImageVerticies(CustomVertex *pCustomVertex, FLOAT posX, FLOAT posY, FLOAT scaleX, FLOAT scaleY,
	DWORD color, FLOAT startPosTu, FLOAT startPosTv, FLOAT scaleTu, FLOAT scaleTv, FLOAT scaleImageX ,FLOAT scaleImageY)
{
	pCustomVertex[0] = { posX - scaleX ,posY - scaleY,0,1,color,startPosTu / scaleImageX,startPosTv / scaleImageY };
	pCustomVertex[1] = { posX + scaleX ,posY - scaleY,0,1,color,startPosTu+ scaleTu / scaleImageX,startPosTv / scaleImageY };
	pCustomVertex[2] = { posX + scaleX ,posY + scaleY,0,1,color,startPosTu+ scaleTu / scaleImageX,startPosTv+ scaleTv / scaleImageY };
	pCustomVertex[3] = { posX - scaleX ,posY + scaleY,0,1,color,startPosTu / scaleImageX,startPosTv+ scaleTv / scaleImageY };

	return;
}

VOID SetImageTuTv(CustomVertex *dest, CustomVertex *src, FLOAT startPosTu, FLOAT startPosTv, FLOAT scaleTu, FLOAT scaleTv, FLOAT scaleImageX, FLOAT scaleImageY)
{
	memcpy(dest, src, sizeof(CustomVertex) * 4);

	dest[0].m_tu = { startPosTu / scaleImageX };
	dest[1].m_tu = { startPosTu + scaleTu / scaleImageX };
	dest[2].m_tu = { startPosTu + scaleTu / scaleImageX };
	dest[3].m_tu = { startPosTu / scaleImageX };

	dest[0].m_tv = { startPosTv / scaleImageY };
	dest[1].m_tv = { startPosTv / scaleImageY };
	dest[2].m_tv = { startPosTv + scaleTv / scaleImageY };
	dest[3].m_tv = { startPosTv + scaleTv / scaleImageY };

	return;
}

DWORD GetColor(UCHAR alpha, UCHAR red, UCHAR blue, UCHAR green)
{
	DWORD color = (alpha << 24) + (red << 16) + (blue << 8) + (green);

	return color;
}

VOID RotateImageDeg(CustomVertex *dest, CustomVertex *src, FLOAT degree, INT axis, FLOAT relativeCenterPosX, FLOAT relativeCenterPosY)
{
	memcpy(dest, src, sizeof(CustomVertex) * 4);

	typedef struct
	{
		FLOAT m_x;
		FLOAT m_y;
	}XY;

	XY srcCenter = { (src[0].m_x + src[2].m_x) / 2.f ,(src[0].m_y + src[2].m_y) / 2.f };
	
	const XY srcDistancesCenterToVertex[4] = 
	{
		{ src[0].m_x - srcCenter.m_x,src[0].m_y - srcCenter.m_y },
		{ src[1].m_x - srcCenter.m_x,src[1].m_y - srcCenter.m_y },
		{ src[2].m_x - srcCenter.m_x,src[2].m_y - srcCenter.m_y },
		{ src[3].m_x - srcCenter.m_x,src[3].m_y - srcCenter.m_y }
	};

	XY movSrcToRotationPoses[4] =
	{
		{srcDistancesCenterToVertex[0].m_x- relativeCenterPosX, srcDistancesCenterToVertex[0].m_y- relativeCenterPosY },
		{srcDistancesCenterToVertex[1].m_x- relativeCenterPosX, srcDistancesCenterToVertex[1].m_y- relativeCenterPosY },
		{srcDistancesCenterToVertex[2].m_x- relativeCenterPosX, srcDistancesCenterToVertex[2].m_y- relativeCenterPosY },
		{srcDistancesCenterToVertex[3].m_x- relativeCenterPosX, srcDistancesCenterToVertex[3].m_y- relativeCenterPosY }
	};

	const double PI = 3.1415926;

	double rad = (double)(degree * PI / 180);

	switch (axis)
	{
	case X_AXIS:

		for (INT vertex = 0; vertex < 4; vertex++)
		{
			dest[vertex].m_y = movSrcToRotationPoses[vertex].m_y*(FLOAT)cos(rad) - src[vertex].m_z*(FLOAT)sin(rad);
			dest[vertex].m_z = movSrcToRotationPoses[vertex].m_y*(FLOAT)sin(rad) + src[vertex].m_z*(FLOAT)cos(rad);
			dest[vertex].m_y += srcCenter.m_y + relativeCenterPosY;
		}

		break;

	case Y_AXIS:

		for (INT vertex = 0; vertex < 4; vertex++)

		{
			dest[vertex].m_z = src[vertex].m_z*(FLOAT)cos(rad) - movSrcToRotationPoses[vertex].m_x*(FLOAT)sin(rad);
			dest[vertex].m_x = src[vertex].m_z*(FLOAT)sin(rad) + movSrcToRotationPoses[vertex].m_x*(FLOAT)cos(rad);
			dest[vertex].m_x += srcCenter.m_x + relativeCenterPosX;
		}

		break;

	case Z_AXIS:

		for (INT vertex = 0; vertex < 4; vertex++)
		{
			dest[vertex].m_x = movSrcToRotationPoses[vertex].m_x*(FLOAT)cos(rad) - movSrcToRotationPoses[vertex].m_y*(FLOAT)sin(rad);
			dest[vertex].m_y = movSrcToRotationPoses[vertex].m_x*(FLOAT)sin(rad) + movSrcToRotationPoses[vertex].m_y*(FLOAT)cos(rad);
			dest[vertex].m_x += srcCenter.m_x + relativeCenterPosX;
			dest[vertex].m_y += srcCenter.m_y + relativeCenterPosY;
		}

		break;
	}

	return;
}

VOID MoveImage(CustomVertex *dest, CustomVertex *src, FLOAT moveX, FLOAT moveY)
{
	memcpy(dest, src, sizeof(CustomVertex) * 4);

	for (INT vertex = 0; vertex < 4; vertex++)
	{
		dest[vertex].m_x = src[vertex].m_x+ moveX;
		dest[vertex].m_y = src[vertex].m_y+ moveY;
	}

	return;
}

VOID CirculateImageDeg(CustomVertex *dest, CustomVertex *src, FLOAT degree, FLOAT circulationCenterPosX, FLOAT circulationCenterPosY)
{
	memcpy(dest, src, sizeof(CustomVertex) * 4);

	typedef struct
	{
		FLOAT m_x;
		FLOAT m_y;
	}XY;

	XY srcCenter = { (src[0].m_x + src[2].m_x) / 2.f ,(src[0].m_y + src[2].m_y) / 2.f };

	const XY srcDistancesCenterToVertex[4] =
	{
		{ src[0].m_x - srcCenter.m_x,src[0].m_y - srcCenter.m_y },
		{ src[1].m_x - srcCenter.m_x,src[1].m_y - srcCenter.m_y },
		{ src[2].m_x - srcCenter.m_x,src[2].m_y - srcCenter.m_y },
		{ src[3].m_x - srcCenter.m_x,src[3].m_y - srcCenter.m_y }
	};

	XY movSrcCenterToCirculationPos =
	{
		{ srcCenter.m_x - circulationCenterPosX },
		{ srcCenter.m_y - circulationCenterPosY }
	};

	const double PI = 3.1415926;

	double rad = (double)(degree * PI / 180);

	for (INT vertex = 0; vertex < 4; vertex++)
	{
		dest[vertex].m_x = movSrcCenterToCirculationPos.m_x*(FLOAT)cos(rad) - movSrcCenterToCirculationPos.m_y*(FLOAT)sin(rad) 
			+ circulationCenterPosX + srcDistancesCenterToVertex[vertex].m_x;
		dest[vertex].m_y = movSrcCenterToCirculationPos.m_x*(FLOAT)sin(rad) + movSrcCenterToCirculationPos.m_y*(FLOAT)cos(rad) 
			+ circulationCenterPosY + srcDistancesCenterToVertex[vertex].m_y;
	}

	return;
}

VOID RescaleImage(CustomVertex *dest, CustomVertex *src, FLOAT expansionRateX, FLOAT expansionRateY)
{
	memcpy(dest, src, sizeof(CustomVertex) * 4);

	typedef struct
	{
		FLOAT m_x;
		FLOAT m_y;
	}XY;

	XY srcCenter = { (src[0].m_x + src[2].m_x) / 2.f ,(src[0].m_y + src[2].m_y) / 2.f };

	const XY srcDistancesCenterToVertex[4] =
	{
		{ (src[0].m_x - srcCenter.m_x)*expansionRateX,(src[0].m_y - srcCenter.m_y)*expansionRateY },
		{ (src[1].m_x - srcCenter.m_x)*expansionRateX,(src[1].m_y - srcCenter.m_y)*expansionRateY },
		{ (src[2].m_x - srcCenter.m_x)*expansionRateX,(src[2].m_y - srcCenter.m_y)*expansionRateY },
		{ (src[3].m_x - srcCenter.m_x)*expansionRateX,(src[3].m_y - srcCenter.m_y)*expansionRateY }
	};

	for (INT vertex = 0; vertex < 4; vertex++)
	{
		dest[vertex].m_x = srcCenter.m_x + srcDistancesCenterToVertex[vertex].m_x;
		dest[vertex].m_y = srcCenter.m_y + srcDistancesCenterToVertex[vertex].m_y;
	}

	return;
}

RENDER_FUNC_RETURN_VAL RoadTexture(const CHAR *pTexturePass, TEXTUREID *pTextureId)
{
	if (FAILED(D3DXCreateTextureFromFile(g_pDirect3DDevice, pTexturePass, pTextureId)))
	{
		return CREATE_TEX_ERR;
	}

	return FUNC_OK;
}

RENDER_FUNC_RETURN_VAL DrawImage(CustomVertex *pCustomVertex, TEXTUREID textureId)
{
	if (FAILED(g_pDirect3DDevice->SetTexture(0, textureId)))
	{
		return SET_TEX_ERR;
	}

	if(FAILED(g_pDirect3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, pCustomVertex, sizeof(CustomVertex))))
	{
		return DRAW_IMAGE_ERR;
	}

	return FUNC_OK;
}

RENDER_FUNC_RETURN_VAL SetFont(INT scaleX, UINT scaleY, const CHAR *pFontType, FONTID *pFontId, UINT thickness)
{
	if (FAILED(D3DXCreateFont(g_pDirect3DDevice, scaleX, scaleY, thickness, 0, 0, 0, 0, 0, 0, pFontType, pFontId)))
	{
		return CREATE_FONT_ERR;
	}

	return FUNC_OK;
}

RENDER_FUNC_RETURN_VAL WriteText(INT posX, INT posY, const CHAR *pText, UINT format, FONTID pFontId, DWORD color)
{
	D3DXFONT_DESC fontSetting;

	pFontId->GetDesc(&fontSetting);

	const FLOAT charSpace = 1.1f;
	INT textScaleX = (INT)(fontSetting.Width* charSpace * strlen(pText))/2;
	INT textScaleY = fontSetting.Height/2;

	RECT rcText = { posX - textScaleX ,posY - textScaleY ,posX + textScaleX ,posY + textScaleY };

	if (FAILED(pFontId->DrawText(NULL, pText, -1, &rcText, format, color)))
	{
		return DRAW_TEXT_ERR;
	}

	return FUNC_OK;
}

VOID PrepareRender(VOID)
{
	g_pDirect3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0x00, 0x00, 0x00), 1.0, 0);
	g_pDirect3DDevice->BeginScene();

	return;
}

VOID CleanUpRender(VOID)
{
	g_pDirect3DDevice->EndScene();
	g_pDirect3DDevice->Present(NULL, NULL, NULL, NULL);

	return;
}


VOID GetKeyInfo(VOID)
{
	g_pDirectInputDevice[KEY]->Acquire();
	g_pDirectInputDevice[KEY]->GetDeviceState(sizeof(BYTE) * 256, g_keyState.diks);

	memset(g_keyState.keyPush, NULL, sizeof(BOOL) * 256 * 4);

	for (INT key = 0; key < 256; key++)
	{
		if (g_keyState.prevDiks[key] & 0x80)
		{
			if (g_keyState.diks[key] & 0x80)
			{
				g_keyState.keyHold[key] = TRUE;
			}

			else
			{
				g_keyState.keyRelease[key] = TRUE;
			}
		}

		else
		{
			if (g_keyState.diks[key] & 0x80)
			{
				g_keyState.keyPush[key] = TRUE;
			}

			else
			{
				g_keyState.keyUninput[key] = TRUE;
			}
		}
	}

	return;
}

VOID UpdatePrevKeyInfo()
{
	memcpy(g_keyState.prevDiks, g_keyState.diks, sizeof(BYTE) * 256);
}

VOID GetMouseInfo(VOID)
{
	g_pDirectInputDevice[MOUSE]->Acquire();
	g_pDirectInputDevice[MOUSE]->GetDeviceState(sizeof(DIMOUSESTATE), &g_mouseState.directInputMouseState);

	GetCursorPos(&g_mouseState.absolutePos);

	memset(g_mouseState.mousePush, 0, sizeof(BOOL) * 4 * 4);

	for (INT button = 0; button < 4; button++)
	{
		if (g_mouseState.directInputMouseState.rgbButtons[button])
		{
			if (g_mouseState.prevDirectInputMouseState.rgbButtons[button])
			{
				g_mouseState.mouseHold[button] = TRUE;
			}

			else
			{
				g_mouseState.mouseRelease[button] = TRUE;
			}
		}

		else
		{
			if (g_mouseState.prevDirectInputMouseState.rgbButtons[button])
			{
				g_mouseState.mousePush[button] = TRUE;
			}

			else
			{
				g_mouseState.mouseUninput[button] = TRUE;
			}
		}
	}

	return;
}

VOID UpdatePrevMouseInfo(VOID)
{
	memcpy(&g_mouseState.prevDirectInputMouseState, &g_mouseState.directInputMouseState, sizeof(DIMOUSESTATE));

	return;
}

//VOID SetCamera(VOID)
//{
//	D3DXMATRIXA16 matWorld;
//
//	D3DXMatrixIdentity(&matWorld);
//	g_pDirect3DDevice->SetTransform(D3DTS_WORLD, &matWorld);
//
//	D3DXVECTOR3 vecEyePt(0.0f, 0.0f, 0.0f);
//	D3DXVECTOR3 vecLookatPt(0.0f, 0.0f, 10.0f);
//	D3DXVECTOR3 vecUpVec(0.0f, 1.0f, 0.0f);
//	D3DXMATRIXA16 matView;
//	D3DXMatrixLookAtLH(&matView, &vecEyePt, &vecLookatPt, &vecUpVec);
//	g_pDirect3DDevice->SetTransform(D3DTS_VIEW, &matView);
//
//	D3DXMATRIXA16 matProj;
//	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 5.0f, -100.0f);
//	g_pDirect3DDevice->SetTransform(D3DTS_VIEW, &matProj);
//
//	return;
//}
