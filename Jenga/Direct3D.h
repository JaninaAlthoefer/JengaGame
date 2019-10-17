#pragma once

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include "Physics.h"

#include <dinput.h>

//class Physics;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")



class Direct3D
{

private:
	IDXGISwapChain* SwapChain;
	ID3D11Device* d3d11Device;
	ID3D11DeviceContext* d3d11DevCon;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11DepthStencilView* depthStencilView;
	ID3D11Texture2D* depthStencilBuffer;
	
	ID3D11Buffer* squareIndexBuffer;
	ID3D11Buffer* squareVertBuffer;
	
	ID3D11VertexShader* VS;
	ID3D11PixelShader* PS;
	ID3D10Blob* VS_Buffer;
	ID3D10Blob* PS_Buffer;
	ID3D11PixelShader* OVER_PS;
	ID3D10Blob* OVER_PS_Buffer;
	ID3D11InputLayout* vertLayout;
	ID3D11Buffer* cbPerObjectBuffer;
	ID3D11Buffer* cbPerFrameBuffer;
	
	//ID3D11RasterizerState* WireFrame;

	ID3D11ShaderResourceView* BlocksTexture;
	ID3D11ShaderResourceView* BlocksTextureLitUp;
	ID3D11ShaderResourceView* GroundTexture;
	ID3D11ShaderResourceView* Player1OverTexture;
	ID3D11ShaderResourceView* Player2OverTexture;
	ID3D11ShaderResourceView* PlayerAIOverTexture;
	ID3D11ShaderResourceView* GameOverTexture;

	ID3D11SamplerState* BlocksTexSamplerState;

	//SkyBox
	ID3D11Buffer* skyIndexBuffer;
	ID3D11Buffer* skyVertBuffer;

	ID3D11VertexShader* SKY_VS;
	ID3D11PixelShader* SKY_PS;
	ID3D10Blob* SKY_VS_Buffer;
	ID3D10Blob* SKY_PS_Buffer;

	ID3D11ShaderResourceView* skyresview;

	ID3D11DepthStencilState* DSLessEqual;
	ID3D11RasterizerState* RSCullNone;

	int numSkyVertices;
	int numSkyFaces;


	IDirectInputDevice8* DIKeyboard;
	IDirectInputDevice8* DIMouse;
	DIMOUSESTATE mouseLastState;
	LPDIRECTINPUT8 DirectInput;



public:
	Direct3D(HWND hwnd);
	~Direct3D(void);
	void UpdateScene(Physics *phys);
	void DrawScene(const int *selected, const bool *standing, const char *name);
	bool InitScene();

	void OnWindowResize(int w, int h);
	
	void SetCamX(float num);
	void SetCamY(float num);
	void SetCamZ(float num);
	//float GetCamZ();
	void UpdateCamView();
	void SetPlacingStateCamera(float h);
	void SetPickingStateCamera(float height);
	void SetGameOverCamera();
	void PickScreenSpaceToWorldSpace(float mouseX, float mouseY, float *position, float *direction);
	void PickFromWorldPosAndDir(int *selected, const float *position, const float *direction, float *distance);
	
	void CreateSkyBox(int numLatitude, int numLongitude);

	void RenderGameOver();
	void RenderTurn(const char *name);

	bool InitDirectInput(HINSTANCE hInstance, HWND hwnd);
	void DetectCamInput(double time, HWND hwnd);

};
