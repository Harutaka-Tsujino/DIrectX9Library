#pragma once

#include <windows.h>

#define DISPLAY_WIDTH 1280
#define DISPLAY_HEIGHT 720
#define CHECK_FPS 0
#define UPDATE_TIME 1
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
#define KEY 0
#define MOUSE 1
#define KEY_AND_MOUSE 2

typedef LPDIRECT3DTEXTURE9 TEXTUREID;

enum AXIS
{
	xAXIS,
	yAXIS,
	zAXIS
};

enum RENDER_FUNC_RETURN_VAL
{
	FUNC_OK,
	CREATE_TEX_ERR,
	SET_TEX_ERR,
	DRAW_IMAGE_ERR
};

extern LPDIRECT3D9 g_pDirect3D;
extern LPDIRECT3DDEVICE9 g_pDirect3DDevice;
extern LPDIRECTINPUT8 g_pDirectInput;
extern LPDIRECT3DTEXTURE9 g_pDirect3DTexture;
extern LPDIRECTINPUTDEVICE8 g_pDirectInputDevice[KEY_AND_MOUSE];
extern LPD3DXFONT g_pDirect3DXFont;
extern D3DPRESENT_PARAMETERS g_Direct3DPresentParameters;

typedef struct
{
	FLOAT m_x;
	FLOAT m_y;
	FLOAT m_scaleX;
	FLOAT m_scaleY;
}ImageState;

typedef struct
{
	FLOAT m_x;
	FLOAT m_y;
	FLOAT m_z;
	FLOAT m_rhw;
	DWORD m_color;
	FLOAT m_tu;
	FLOAT m_tv;
}CustomVertex;

INT CreateWindowAndRepeatToControlAndRender(HINSTANCE hInst, const CHAR *appName, VOID(*func)(VOID), INT displayWidth = 1280, INT displayHeight = 720, INT keyAndMouse = KEY_AND_MOUSE);

VOID CreateWindowOverall(HWND *hWnd, MSG *msg, HINSTANCE hInst, const CHAR *appName, INT displayWidth, INT displayHeight);

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

INT ConfirmMessage(MSG *msg);

INT CoordinateFPS(INT action, UINT frame = 60);

HRESULT InitDirect3D(VOID);

VOID SetBuckBufferOverall(VOID);

HRESULT InitDirect3DDevice(HWND hWnd);

VOID SetRenderStateOverall(VOID);

VOID SetTextureStageStateOverall(VOID);

HRESULT InitDinput(HWND hWnd, INT keyAndMouse);

VOID GetMouseState(VOID);

VOID CustomImageVerticies(CustomVertex *pCustomVertex, FLOAT posX, FLOAT posY, FLOAT scaleX, FLOAT scaleY,
	DWORD color = 0xFFFFFFFF, FLOAT startPosTu = 0.f, FLOAT startPosTv = 0.f, FLOAT scaleTu = 1.f, FLOAT scaleTv = 1.f, FLOAT scaleImageX = 1.f, FLOAT scaleImageY = 1.f);

VOID SetImageTuTv(CustomVertex *dest, CustomVertex *src, FLOAT startPosTu, FLOAT startPosTv, FLOAT scaleTu, FLOAT scaleTv, FLOAT scaleImageX, FLOAT scaleImageY);

DWORD GetColor(UCHAR alpha, UCHAR red, UCHAR blue, UCHAR green);

VOID RotateImageDeg(CustomVertex *dest, CustomVertex *src, FLOAT degree, INT axis = zAXIS, FLOAT relativeCenterPosX = 0.f, FLOAT relativeCenterPosY = 0.f);

VOID MoveImage(CustomVertex *dest, CustomVertex *src, FLOAT moveX, FLOAT moveY);

VOID CirculateImageDeg(CustomVertex *dest, CustomVertex *src, FLOAT degree, FLOAT circulationCenterPosX, FLOAT circulationCenterPosY);

VOID RescaleImage(CustomVertex *dest, CustomVertex *src, FLOAT expansionRateX, FLOAT expansionRateY);

RENDER_FUNC_RETURN_VAL RoadTexture(const CHAR *pTexturePass, TEXTUREID *pTextureId);

RENDER_FUNC_RETURN_VAL DrawImage(CustomVertex *pCustomVertex, TEXTUREID textureId);