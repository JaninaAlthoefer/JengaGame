#pragma once

#include <DirectXMath.h>
#include <vector>
#include "GlobalSizes.h"

using namespace DirectX;

//const int numWorlds = 54;

//Camera stuff
XMMATRIX WVP;
//XMMATRIX cubeWorld;			//dynamic 
//XMMATRIX cubeWorld2;
XMMATRIX blockWorldArray[numBlocks];
XMMATRIX groundWorld;		//stationary 
XMMATRIX World;
XMMATRIX camView;
XMMATRIX camProjection;
XMMATRIX Rotation;
XMMATRIX Scale;
XMMATRIX Translation;

XMVECTOR camPosition;

//Overlay
XMMATRIX gameOverWorld;
XMMATRIX playerTurnWorld;

//Skybox
XMMATRIX skyWorld;

//Picking
std::vector<XMFLOAT3> vertPosArray;
std::vector<DWORD> vertIndexArray;

//functions
bool IsInTriangle(XMVECTOR& triV1, XMVECTOR& triV2, XMVECTOR& triV3, XMVECTOR& point);

//constant buffer
struct cbPerObject
{
	XMMATRIX WVP;
	XMMATRIX World;
};
cbPerObject cbPerObj;

//light structure
struct Light
{
	Light()
	{
		ZeroMemory(this, sizeof(Light));
	}
	XMFLOAT3 dir;
	float padding1;  //hlsl won't put ambient.x with dir.w
	XMFLOAT3 pos; 
	float range;
	XMFLOAT3 attentuation;
	float padding2;
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
};
Light light;

struct cbPerFrame
{
	Light light;
};
cbPerFrame cbPerFrm;

//vertex structure
struct Vertex
{
	Vertex(){}
	Vertex(float x, float y, float z, 
			float u, float v, 
			float nx, float ny, float nz) 
			: pos(x, y, z), texCoord(u, v), normal(nx, ny, nz) {}

	XMFLOAT3 pos;
	XMFLOAT2 texCoord;
	XMFLOAT3 normal;
};

//input layout
D3D11_INPUT_ELEMENT_DESC layout[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },  
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },  
};