#include "Direct3D.h"
#include "Direct3DMath.h"


int WIDTH = 800;
int HEIGHT = 600;

//dynamic Camera
float camY = 5.0f;
float camX = 0.0f;
float camZ = 7.0f;

bool updateableCamera = true;

UINT numLayoutElements = ARRAYSIZE(layout);

Direct3D::Direct3D(HWND hwnd)
{
	DXGI_MODE_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));
	bufferDesc.Width = WIDTH;
	bufferDesc.Height = HEIGHT;
	bufferDesc.RefreshRate.Numerator = 60;
	bufferDesc.RefreshRate.Denominator = 1;
	bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	//swapChain
	DXGI_SWAP_CHAIN_DESC swapChainDesc; 
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferDesc = bufferDesc;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hwnd; 
	swapChainDesc.Windowed = TRUE; 
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL, D3D11_SDK_VERSION, &swapChainDesc, &SwapChain, &d3d11Device, NULL, &d3d11DevCon);

	//rendertarget in backbuffer
	ID3D11Texture2D* BackBuffer;
	SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&BackBuffer );

	d3d11Device->CreateRenderTargetView( BackBuffer, NULL, &renderTargetView );
	BackBuffer->Release();

	//depthstencil
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width     = WIDTH;
	depthStencilDesc.Height    = HEIGHT;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count   = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage          = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0; 
	depthStencilDesc.MiscFlags      = 0;

	d3d11Device->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);
	d3d11Device->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);

	//d3d11DevCon->OMSetRenderTargets( 1, &renderTargetView, NULL );
	d3d11DevCon->OMSetRenderTargets( 1, &renderTargetView, depthStencilView );
}

Direct3D::~Direct3D(void)
{
	SwapChain->SetFullscreenState(false, NULL);

	SwapChain->Release();
	d3d11Device->Release();
	d3d11DevCon->Release();
	renderTargetView->Release();
	squareVertBuffer->Release();
	squareIndexBuffer->Release();
	VS->Release();
	PS->Release();
	VS_Buffer->Release();
	PS_Buffer->Release();
	vertLayout->Release();
	depthStencilView->Release();
	depthStencilBuffer->Release();
	cbPerObjectBuffer->Release();
	//WireFrame->Release();
	cbPerFrameBuffer->Release();

	skyIndexBuffer->Release();
	skyVertBuffer->Release();

	SKY_VS->Release();
	SKY_PS->Release();
	SKY_VS_Buffer->Release();
	SKY_PS_Buffer->Release();

	skyresview->Release();

	DSLessEqual->Release();
	RSCullNone->Release();





	DIKeyboard->Unacquire();
	DIMouse->Unacquire();
	DirectInput->Release();

}

bool Direct3D::InitScene()
{
	CreateSkyBox(10, 10);

	//shader
	D3DX11CompileFromFile(L"Shader.fx", 0, 0, "VS", "vs_5_0", 0, 0, 0, &VS_Buffer, 0, 0);
	D3DX11CompileFromFile(L"Shader.fx", 0, 0, "PS", "ps_5_0", 0, 0, 0, &PS_Buffer, 0, 0);
	D3DX11CompileFromFile(L"Shader.fx", 0, 0, "TEX2D_PS", "ps_5_0", 0, 0, 0, &OVER_PS_Buffer, 0, 0);

	D3DX11CompileFromFile(L"Shader.fx", 0, 0, "SKYBOX_VS", "vs_5_0", 0, 0, 0, &SKY_VS_Buffer, 0, 0);
	D3DX11CompileFromFile(L"Shader.fx", 0, 0, "SKYBOX_PS", "ps_5_0", 0, 0, 0, &SKY_PS_Buffer, 0, 0);
    
	d3d11Device->CreateVertexShader(SKY_VS_Buffer->GetBufferPointer(), SKY_VS_Buffer->GetBufferSize(), NULL, &SKY_VS);
	d3d11Device->CreatePixelShader(SKY_PS_Buffer->GetBufferPointer(), SKY_PS_Buffer->GetBufferSize(), NULL, &SKY_PS);

	d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);
	d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);
	d3d11Device->CreatePixelShader(OVER_PS_Buffer->GetBufferPointer(), OVER_PS_Buffer->GetBufferSize(), NULL, &OVER_PS);

	d3d11DevCon->VSSetShader(VS, 0, 0);
	d3d11DevCon->PSSetShader(PS, 0, 0);

	light.pos = XMFLOAT3(-5.0f, 15.0f, 5.0f);
	light.range = 150.0f;
	light.attentuation = XMFLOAT3(0.5f, 0.1f, 0.0f); //lower => brighter
	light.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	light.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);


	Vertex v[] =
	{         //	    x				   y			      z			   u	    v		nx	   ny	  nz
		// front
		Vertex(-halfBoxMiddleSide, -halfBoxShortSide, -halfBoxLongSide, 0.3026f, 0.5284f, -1.0f, -1.0f, -1.0f),
		Vertex(-halfBoxMiddleSide,  halfBoxShortSide, -halfBoxLongSide, 0.3026f, 0.332f,  -1.0f,  1.0f, -1.0f),
		Vertex( halfBoxMiddleSide,  halfBoxShortSide, -halfBoxLongSide, 0.6385f, 0.332f,   1.0f,  1.0f, -1.0f),
		Vertex( halfBoxMiddleSide, -halfBoxShortSide, -halfBoxLongSide, 0.6385f, 0.5248f,  1.0f, -1.0f, -1.0f),

		// back
		Vertex(-halfBoxMiddleSide, -halfBoxShortSide,  halfBoxLongSide, 0.9627f, 0.5284f, -1.0f, -1.0f,  1.0f),
		Vertex( halfBoxMiddleSide, -halfBoxShortSide,  halfBoxLongSide, 0.6385f, 0.5284f,  1.0f, -1.0f,  1.0f),
		Vertex( halfBoxMiddleSide,  halfBoxShortSide,  halfBoxLongSide, 0.6385f, 0.332f,   1.0f,  1.0f,  1.0f),
		Vertex(-halfBoxMiddleSide,  halfBoxShortSide,  halfBoxLongSide, 0.9627f, 0.332f,  -1.0f,  1.0f,  1.0f),

		// top
		Vertex(-halfBoxMiddleSide,  halfBoxShortSide, -halfBoxLongSide,   0.0f,  0.0f,    -1.0f,  1.0f, -1.0f),
		Vertex(-halfBoxMiddleSide,  halfBoxShortSide,  halfBoxLongSide,   1.0f,  0.0f,    -1.0f,  1.0f,  1.0f),
		Vertex( halfBoxMiddleSide,  halfBoxShortSide,  halfBoxLongSide,   1.0f,  0.332f,   1.0f,  1.0f,  1.0f),
		Vertex( halfBoxMiddleSide,  halfBoxShortSide, -halfBoxLongSide,   0.0f,  0.332f,   1.0f,  1.0f, -1.0f),

		// bottom
		Vertex(-halfBoxMiddleSide, -halfBoxShortSide, -halfBoxLongSide,   0.0f,  0.0f,    -1.0f, -1.0f, -1.0f),
		Vertex( halfBoxMiddleSide, -halfBoxShortSide, -halfBoxLongSide,   0.0f,  0.332f,   1.0f, -1.0f, -1.0f),
		Vertex( halfBoxMiddleSide, -halfBoxShortSide,  halfBoxLongSide,   1.0f,  0.332f,   1.0f, -1.0f,  1.0f),
		Vertex(-halfBoxMiddleSide, -halfBoxShortSide,  halfBoxLongSide,   1.0f,  0.0f,    -1.0f, -1.0f,  1.0f),

		// left
		Vertex(-halfBoxMiddleSide, -halfBoxShortSide,  halfBoxLongSide,   1.0f,  0.5284f, -1.0f, -1.0f,  1.0f),
		Vertex(-halfBoxMiddleSide,  halfBoxShortSide,  halfBoxLongSide,   1.0f,  0.727f,  -1.0f,  1.0f,  1.0f),
		Vertex(-halfBoxMiddleSide,  halfBoxShortSide, -halfBoxLongSide,   0.0f,  0.727f,  -1.0f,  1.0f, -1.0f),
		Vertex(-halfBoxMiddleSide, -halfBoxShortSide, -halfBoxLongSide,   0.0f,  0.5284f, -1.0f, -1.0f, -1.0f),

		// right
		Vertex( halfBoxMiddleSide, -halfBoxShortSide, -halfBoxLongSide,   0.0f,  0.9273f,  1.0f, -1.0f, -1.0f),
		Vertex( halfBoxMiddleSide,  halfBoxShortSide, -halfBoxLongSide,   0.0f,  0.727f,   1.0f,  1.0f, -1.0f),
		Vertex( halfBoxMiddleSide,  halfBoxShortSide,  halfBoxLongSide,   1.0f,  0.727f,   1.0f,  1.0f,  1.0f),
		Vertex( halfBoxMiddleSide, -halfBoxShortSide,  halfBoxLongSide,   1.0f,  0.9273f,  1.0f, -1.0f,  1.0f),

		//ground
		Vertex(-1.0f, 1.0f, -1.0f, 0.0f, 10.0f, -1.0f,  1.0f, -1.0f),
		Vertex(-1.0f, 1.0f,  1.0f, 0.0f, 0.0f, -1.0f,  1.0f,  1.0f),
		Vertex( 1.0f, 1.0f,  1.0f, 10.0f, 0.0f,  1.0f,  1.0f,  1.0f),
		Vertex( 1.0f, 1.0f, -1.0f, 10.0f, 10.0f,  1.0f,  1.0f, -1.0f),

		// GameOverOverlay
		Vertex(-1.0f, -0.75f, -1.0f, 0.0f, 1.0f, -1.0f, -1.0f, -1.0f),
		Vertex(-1.0f,  0.75f, -1.0f, 0.0f, 0.0f, -1.0f,  1.0f, -1.0f),
		Vertex( 1.0f,  0.75f, -1.0f, 1.0f, 0.0f,  1.0f,  1.0f, -1.0f),
		Vertex( 1.0f, -0.75f, -1.0f, 1.0f, 1.0f,  1.0f, -1.0f, -1.0f),
		
		// PlayerTurn
		Vertex( 0.55f,  0.75f, -1.0f, 0.0f, 1.0f, -1.0f, -1.0f, -1.0f),
		Vertex( 0.55f,  1.00f, -1.0f, 0.0f, 0.0f, -1.0f,  1.0f, -1.0f),
		Vertex( 1.05f,  1.00f, -1.0f, 1.0f, 0.0f,  1.0f,  1.0f, -1.0f),
		Vertex( 1.05f,  0.75f, -1.0f, 1.0f, 1.0f,  1.0f, -1.0f, -1.0f)
	};

	for (int i=0; i <24; i++)
		vertPosArray.push_back(v[i].pos);

	DWORD indices[] = {
		// front
		0,  1,  2,		0,  2,  3,
		// back
		4,  5,  6,		4,  6,  7,
		// top
		8,  9, 10,		8, 10, 11,
		// bottom
		12, 13, 14,		12, 14, 15,
		// left
		16, 17, 18,		16, 18, 19,
		// right
		20, 21, 22,		20, 22, 23,
		//ground
		24, 25, 26,     24, 26, 27,
		//GameOverOverlay
		28, 29, 30,     28, 30, 31,
		//PlayerTurn
		32, 33, 34,     32, 34, 35
	};

	for (int i =0; i<36; i++)
		vertIndexArray.push_back(indices[i]);

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory( &indexBufferDesc, sizeof(indexBufferDesc) );

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * 54;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;
	d3d11Device->CreateBuffer(&indexBufferDesc, &iinitData, &squareIndexBuffer);

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory( &vertexBufferDesc, sizeof(vertexBufferDesc) );

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof( Vertex ) * 36;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData; 
	ZeroMemory( &vertexBufferData, sizeof(vertexBufferData) );
	vertexBufferData.pSysMem = v;
	d3d11Device->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &squareVertBuffer);

	//input layout
	d3d11Device->CreateInputLayout( layout, numLayoutElements, VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), &vertLayout );
	d3d11DevCon->IASetInputLayout( vertLayout );
	d3d11DevCon->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	//viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = WIDTH;
	viewport.Height = HEIGHT;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	d3d11DevCon->RSSetViewports(1, &viewport);

	//constant buffer per object
	D3D11_BUFFER_DESC cbBufferDesc;
	ZeroMemory(&cbBufferDesc, sizeof(D3D11_BUFFER_DESC));

	cbBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	cbBufferDesc.ByteWidth = sizeof(cbPerObject);
	cbBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbBufferDesc.CPUAccessFlags = 0;
	cbBufferDesc.MiscFlags = 0;

	d3d11Device->CreateBuffer(&cbBufferDesc, NULL, &cbPerObjectBuffer);

	//constant buffer per frame
	ZeroMemory(&cbBufferDesc, sizeof(D3D11_BUFFER_DESC));

	cbBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	cbBufferDesc.ByteWidth = sizeof(cbPerFrame);
	cbBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbBufferDesc.CPUAccessFlags = 0;
	cbBufferDesc.MiscFlags = 0;

	d3d11Device->CreateBuffer(&cbBufferDesc, NULL, &cbPerFrameBuffer);


	/*//wireframe
	D3D11_RASTERIZER_DESC wfdesc;
	ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	wfdesc.FillMode = D3D11_FILL_WIREFRAME;
	wfdesc.CullMode = D3D11_CULL_NONE;
	d3d11Device->CreateRasterizerState(&wfdesc, &WireFrame);
	//*/

	D3DX11CreateShaderResourceViewFromFile( d3d11Device, L"JengaTexture.png", NULL, NULL, &BlocksTexture, NULL );
	D3DX11CreateShaderResourceViewFromFile( d3d11Device, L"JengaTextureBright.png", NULL, NULL, &BlocksTextureLitUp, NULL );
	D3DX11CreateShaderResourceViewFromFile( d3d11Device, L"ground.png", NULL, NULL, &GroundTexture, NULL );
	D3DX11CreateShaderResourceViewFromFile( d3d11Device, L"Player1.png", NULL, NULL, &Player1OverTexture, NULL );
	D3DX11CreateShaderResourceViewFromFile( d3d11Device, L"Player2.png", NULL, NULL, &Player2OverTexture, NULL );
	D3DX11CreateShaderResourceViewFromFile( d3d11Device, L"PlayerAI.png", NULL, NULL, &PlayerAIOverTexture, NULL );
	D3DX11CreateShaderResourceViewFromFile( d3d11Device, L"GameOver.png", NULL, NULL, &GameOverTexture, NULL );

	//SkyBox Texture
	D3DX11_IMAGE_LOAD_INFO loadSMInfo;
	loadSMInfo.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	//Load the texture
	ID3D11Texture2D* SMTexture = 0;
	D3DX11CreateTextureFromFile(d3d11Device, L"SkyBox.dds", &loadSMInfo, 0, (ID3D11Resource**)&SMTexture, 0);

	//Create the textures description
	D3D11_TEXTURE2D_DESC SMTextureDesc;
	SMTexture->GetDesc(&SMTextureDesc);

	//Tell D3D We have a cube texture, which is an array of 2D textures
	D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
	SMViewDesc.Format = SMTextureDesc.Format;
	SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SMViewDesc.TextureCube.MipLevels = SMTextureDesc.MipLevels;
	SMViewDesc.TextureCube.MostDetailedMip = 0;

	//Create the Resource view
	d3d11Device->CreateShaderResourceView(SMTexture, &SMViewDesc, &skyresview);


	// Describe the Sample State
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory( &sampDesc, sizeof(sampDesc) );
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    
	//Create the Sample State
	d3d11Device->CreateSamplerState( &sampDesc, &BlocksTexSamplerState );


	//SkyBox stencil and rasterizer
	D3D11_RASTERIZER_DESC cmdesc;

	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_NONE;
	d3d11Device->CreateRasterizerState(&cmdesc, &RSCullNone);

	D3D11_DEPTH_STENCIL_DESC dssDesc;
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	d3d11Device->CreateDepthStencilState(&dssDesc, &DSLessEqual);

	return true;
}

void Direct3D::UpdateScene(Physics *phys)
{
	for (int i=0; i < numBlocks; i++)
	{
		//Reset cube matrix
		blockWorldArray[i] = XMMatrixIdentity();

		//cube world space matrix
		float physMat[16];
		phys->getBoxPoseRender(&i, physMat);
		blockWorldArray[i] = XMMATRIX(physMat);
	}

	//Reset ground matrix
	groundWorld = XMMatrixIdentity();

	Translation = XMMatrixTranslation( 0.0f, -(1.0f/*-halfBoxShortSide*/), 0.0f );
	Scale = XMMatrixScaling( 150.0f, 1.0f, 150.0f );

	groundWorld = Scale * Translation;

	//Reset skyWorld
	skyWorld = XMMatrixIdentity();

	Scale = XMMatrixScaling( 5.0f, 5.0f, 5.0f );
	Translation = XMMatrixTranslation( XMVectorGetX(camPosition), XMVectorGetY(camPosition), XMVectorGetZ(camPosition) );
	skyWorld = Scale * Translation;


}

void Direct3D::DrawScene(const int *selected, const bool *standing, const char *name)
{
	//clear backbuffer & depth/stencil
	float bgColor[4] = {(0.2f, 0.2f, 0.2f, 0.0f)};
	d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);
	d3d11DevCon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	cbPerFrm.light = light;
	d3d11DevCon->UpdateSubresource( cbPerFrameBuffer, 0, NULL, &cbPerFrm, 0, 0 );
	d3d11DevCon->PSSetConstantBuffers(0, 1, &cbPerFrameBuffer);	

	if (updateableCamera)
		UpdateCamView();

	//SkyBox
	d3d11DevCon->IASetIndexBuffer( skyIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//Set the spheres vertex buffer
	UINT stride = sizeof( Vertex );
	UINT offset = 0;
	d3d11DevCon->IASetVertexBuffers( 0, 1, &skyVertBuffer, &stride, &offset );

	WVP = skyWorld * camView * camProjection;
	cbPerObj.WVP = XMMatrixTranspose(WVP);	
	cbPerObj.World = XMMatrixTranspose(skyWorld);	
	d3d11DevCon->UpdateSubresource( cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0 );
	d3d11DevCon->VSSetConstantBuffers( 0, 1, &cbPerObjectBuffer );

	d3d11DevCon->PSSetShaderResources( 0, 1, &skyresview );
	d3d11DevCon->PSSetSamplers( 0, 1, &BlocksTexSamplerState );

	d3d11DevCon->RSSetState(RSCullNone);
	d3d11DevCon->VSSetShader(SKY_VS, 0, 0);
	d3d11DevCon->PSSetShader(SKY_PS, 0, 0);
	d3d11DevCon->OMSetDepthStencilState(DSLessEqual, 0);

	d3d11DevCon->DrawIndexed( numSkyFaces * 3, 0, 0 );


	//reset default shader, etc
	d3d11DevCon->RSSetState(NULL);
	d3d11DevCon->OMSetDepthStencilState(NULL, 0);
	d3d11DevCon->IASetIndexBuffer( squareIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	stride = sizeof( Vertex );
	offset = 0;
	d3d11DevCon->IASetVertexBuffers( 0, 1, &squareVertBuffer, &stride, &offset );

	d3d11DevCon->VSSetShader(VS, 0, 0);
	d3d11DevCon->PSSetShader(PS, 0, 0);
	
	d3d11DevCon->PSSetShaderResources( 0, 1, &BlocksTexture );

	//"ground" world init
	WVP = groundWorld * camView * camProjection;
	cbPerObj.World = XMMatrixTranspose(groundWorld);
	cbPerObj.WVP = XMMatrixTranspose(WVP);	
	d3d11DevCon->UpdateSubresource( cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0 );
	d3d11DevCon->VSSetConstantBuffers( 0, 1, &cbPerObjectBuffer );
	d3d11DevCon->PSSetShaderResources( 0, 1, &GroundTexture );

	//just draw top of "gound cube"
	d3d11DevCon->DrawIndexed( 6, 36, 0 ); 
	

	//drawing all blocks
	for (int i=0; i < numBlocks; i++)
	{
		//cube init
		WVP = blockWorldArray[i] * camView * camProjection;
		cbPerObj.World = XMMatrixTranspose(blockWorldArray[i]);
		cbPerObj.WVP = XMMatrixTranspose(WVP);	
		d3d11DevCon->UpdateSubresource( cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0 );
		d3d11DevCon->VSSetConstantBuffers( 0, 1, &cbPerObjectBuffer );

		d3d11DevCon->PSSetShaderResources( 0, 1, &BlocksTexture );
		//d3d11DevCon->PSSetSamplers( 0, 1, &BlocksTexSamplerState );

		if (*selected == i)
		{			
			d3d11DevCon->PSSetShaderResources(0, 1, &BlocksTextureLitUp);
			//d3d11DevCon->RSSetState(WireFrame);
			d3d11DevCon->DrawIndexed( 36, 0, 0 );
			//d3d11DevCon->RSSetState(NULL);
			//d3d11DevCon->PSSetShaderResources(0, 1, &BlocksTexture);
		}
		else
			d3d11DevCon->DrawIndexed( 36, 0, 0 );

	}

	
	//render GameOverScreen
	if (!*standing)
		RenderGameOver();
	//*/

	//render playerturn
	if(*standing)
		RenderTurn(name);


	SwapChain->Present(0, 0);
}



void Direct3D::OnWindowResize(int w, int h)
{
	WIDTH = w;
	HEIGHT = h;

	//release buffers
	d3d11DevCon->OMSetRenderTargets(0,0,0);
	renderTargetView->Release();
	depthStencilView->Release();
	depthStencilBuffer->Release();

	//resize
	SwapChain->ResizeBuffers(0, WIDTH, HEIGHT, DXGI_FORMAT_UNKNOWN, 0);

	//rendertarget in backbuffer
	ID3D11Texture2D* BackBuffer;
	SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&BackBuffer );

	d3d11Device->CreateRenderTargetView( BackBuffer, NULL, &renderTargetView );
	BackBuffer->Release();

	//depthstencil
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width     = WIDTH;
	depthStencilDesc.Height    = HEIGHT;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count   = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage          = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0; 
	depthStencilDesc.MiscFlags      = 0;

	d3d11Device->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);
	d3d11Device->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);

	d3d11DevCon->OMSetRenderTargets( 1, &renderTargetView, depthStencilView );

	//new viewport
	D3D11_VIEWPORT vp;
	ZeroMemory(&vp, sizeof(D3D11_VIEWPORT));
		
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = WIDTH;
	vp.Height = HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	d3d11DevCon->RSSetViewports(1, &vp);

}



void Direct3D::SetCamX(float num)
{
	camX = camX + num;

	if (camX > 360)
		camX = fmod(camX, 360);
}

void Direct3D::SetCamY(float num)
{
	camY = camY + num;

	if (camY < 1.0f)		camY = 1.0f;
	if (camY > 15.0f)		camY = 15.0f;
}

void Direct3D::SetCamZ(float num)
{
	camZ = camZ + num;

	if (camZ < 3.0f)		camZ = 3.0f;
	if (camZ > 10.0f)		camZ = 10.0f;
}

/*float Direct3D::GetCamZ()
{
	return camZ;
}//*/

void Direct3D::UpdateCamView()
{
	///* interactive Camera
	if (camY < 1.0f)		camY = 1.0f;
	if (camY > 15.0f)		camY = 15.0f;
	if (camZ < 3.0f)		camZ = 3.0f;
	if (camZ > 10.0f)		camZ = 10.0f;
	//*/

	float angle;

	if(camX < 0.0f)
	{
		angle = (fmod(camX, 360) * 3.14159)/180;
	}
	else
	{
		angle = (fmod(camX, 360) * 3.14159)/180;
	}

	camProjection = XMMatrixPerspectiveFovLH(0.4f*3.14f, (float) WIDTH/HEIGHT, 1.0f, 100.0f);
	//XMVECTOR 
		camPosition = XMVectorSet( camZ * cosf(angle)/*-halfBoxMiddleSide*/, camY, camZ * sinf(angle)/*-halfBoxLongSide*/, 0.0f );
	XMVECTOR camTarget = XMVectorSet(0.0f/*-halfBoxMiddleSide*/, camY, 0.0f/*-halfBoxLongSide*/, 0.0f);
	XMVECTOR camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);
}

void Direct3D::SetPlacingStateCamera(float h)
{
	updateableCamera = false;
	//float height = 0.5*h;

	camProjection = XMMatrixPerspectiveFovLH(0.4f*3.14f, (float) WIDTH/HEIGHT, 1.0f, 100.0f);
	//XMVECTOR 
		camPosition = XMVectorSet( 0.001f/*-halfBoxMiddleSide*/, h+5.0f, 0.001f/*-halfBoxLongSide*/, 0.0f );
	XMVECTOR camTarget = XMVectorSet(0.0f/*-halfBoxMiddleSide*/, h, 0.0f/*-halfBoxLongSide*/, 0.0f);
	XMVECTOR camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);
}

void Direct3D::SetPickingStateCamera(float height)
{
	camY = height;
	updateableCamera = true;
}

void Direct3D::SetGameOverCamera()
{	
	camX = 0.0f;
	camY = 5.0f;
	camZ = 6.0f;
	
	updateableCamera = true;
}



void Direct3D::PickScreenSpaceToWorldSpace(float mouseX, float mouseY, float *position, float *direction)
{
	XMVECTOR pickingRayViewSpacePosition = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	//Transform from 2D to 3D
	XMFLOAT4X4 projectionMat;
	XMStoreFloat4x4(&projectionMat, camProjection);

	float rayVecX =  ((( 2.0f * mouseX) / WIDTH ) - 1 ) / projectionMat._11;
	float rayVecY = -((( 2.0f * mouseY) / HEIGHT) - 1 ) / projectionMat._22;
	float rayVecZ =  1.0f;	

	XMVECTOR pickingRayViewSpaceDirection = XMVectorSet(rayVecX, rayVecY, rayVecZ, 0.0f);

	//ViewSpace --> WorldSpace
	XMMATRIX pickingRayWorldSpaceMatrix;
	XMVECTOR matInvDeter;			// unused

	pickingRayWorldSpaceMatrix = XMMatrixInverse(&matInvDeter, camView);

	XMVECTOR pickingRayWorldSpaceDirection = XMVector3TransformNormal(pickingRayViewSpaceDirection, pickingRayWorldSpaceMatrix);
	XMVECTOR pickingRayWorldSpacePosition = XMVector3TransformCoord(pickingRayViewSpacePosition, pickingRayWorldSpaceMatrix);

	//store the crap
	XMFLOAT4 posXMF, dirXMF;
	XMStoreFloat4(&dirXMF, pickingRayWorldSpaceDirection);
	XMStoreFloat4(&posXMF, pickingRayWorldSpacePosition);

	position[0] = posXMF.x;
	position[1] = posXMF.y;
	position[2] = posXMF.z;
	position[3] = posXMF.w;

	direction[0] = dirXMF.x;
	direction[1] = dirXMF.y;
	direction[2] = dirXMF.z;
	direction[3] = dirXMF.w;

}

void Direct3D::PickFromWorldPosAndDir(int *selected, const float *position, const float *direction, float *distance)
{
	XMFLOAT4 dirXMF4 = XMFLOAT4(direction);
	XMFLOAT4 posXMF4 = XMFLOAT4(position);

	XMVECTOR pickingRayWorldSpaceDirection = XMLoadFloat4(&dirXMF4);
	XMVECTOR pickingRayWorldSpacePosition = XMLoadFloat4(&posXMF4);

	//XMLoadFloat4(&pickingRayWorldSpaceDirection, XMFLOAT4(direction));

	//go through all three-part indices
	float closestDistance = FLT_MAX;
	
	// wrap in another for loop for every box => selected = i
	for (int j = 0; j < numBlocks; j++)
	{
		for (int i=0; i< vertIndexArray.size()/3; i++)
		{
			//Get triangle 
			XMFLOAT3 tV1 = vertPosArray[vertIndexArray[(i*3)+0]];
			XMFLOAT3 tV2 = vertPosArray[vertIndexArray[(i*3)+1]];
			XMFLOAT3 tV3 = vertPosArray[vertIndexArray[(i*3)+2]];

			//Triangle's vertices V1, V2, V3
			XMVECTOR tri1V1 = XMVectorSet(tV1.x, tV1.y, tV1.z, 0.0f);
			XMVECTOR tri1V2 = XMVectorSet(tV2.x, tV2.y, tV2.z, 0.0f);
			XMVECTOR tri1V3 = XMVectorSet(tV3.x, tV3.y, tV3.z, 0.0f);	

			//Transform the vertices to world space, here also array for worlds
			tri1V1 = XMVector3TransformCoord(tri1V1, blockWorldArray[j]);
			tri1V2 = XMVector3TransformCoord(tri1V2, blockWorldArray[j]);
			tri1V3 = XMVector3TransformCoord(tri1V3, blockWorldArray[j]);

			//Find the normal using U, V coordinates (two edges)
			XMVECTOR U = tri1V2 - tri1V1;
			XMVECTOR V = tri1V3 - tri1V1;
			XMVECTOR faceNormal = XMVector3Cross(U, V);
			faceNormal = XMVector3Normalize(faceNormal);

			//Calculate a point on the triangle for the plane equation ("Ax + By + Cz + D = 0")
			XMVECTOR triPoint = tri1V1;
			float tri1A = XMVectorGetX(faceNormal);
			float tri1B = XMVectorGetY(faceNormal);
			float tri1C = XMVectorGetZ(faceNormal);
			float tri1D = (-tri1A*XMVectorGetX(triPoint) - tri1B*XMVectorGetY(triPoint) - tri1C*XMVectorGetZ(triPoint));

			//intersection ray & triangle plane
			float ep1, ep2, t = 0.0f;
			float planeIntersectX, planeIntersectY, planeIntersectZ = 0.0f;
			XMVECTOR pointInPlane = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

			ep1 = (XMVectorGetX(pickingRayWorldSpacePosition) * tri1A) + (XMVectorGetY(pickingRayWorldSpacePosition) * tri1B) + (XMVectorGetZ(pickingRayWorldSpacePosition) * tri1C);
			ep2 = (XMVectorGetX(pickingRayWorldSpaceDirection) * tri1A) + (XMVectorGetY(pickingRayWorldSpaceDirection) * tri1B) + (XMVectorGetZ(pickingRayWorldSpaceDirection) * tri1C);

			//auswertung
			if(ep2 != 0.0f)
			t = -(ep1 + tri1D)/(ep2);

			if(t > 0.0f && t < 10.0f)    //not too far or behind
			{
				planeIntersectX = XMVectorGetX(pickingRayWorldSpacePosition) + XMVectorGetX(pickingRayWorldSpaceDirection) * t;
				planeIntersectY = XMVectorGetY(pickingRayWorldSpacePosition) + XMVectorGetY(pickingRayWorldSpaceDirection) * t;
				planeIntersectZ = XMVectorGetZ(pickingRayWorldSpacePosition) + XMVectorGetZ(pickingRayWorldSpaceDirection) * t;

				pointInPlane = XMVectorSet(planeIntersectX, planeIntersectY, planeIntersectZ, 0.0f);

				if(IsInTriangle(tri1V1, tri1V2, tri1V3, pointInPlane))
				{
					// how to incorporate distance for picking nearest block
					float tempDistance = t/2.0f;

					if (tempDistance < closestDistance)
					{
						*selected = j; // put selected = j-loop here
						closestDistance = tempDistance;
						*distance = closestDistance;
					}
				}
			}
		}
	}

	if (closestDistance == FLT_MAX)
		*selected = -1;
}

bool IsInTriangle(XMVECTOR& triV1, XMVECTOR& triV2, XMVECTOR& triV3, XMVECTOR& point)
{
	// if inside triangle then on right side of every edge
	XMVECTOR cp1 = XMVector3Cross((triV3 - triV2), (point - triV2));
	XMVECTOR cp2 = XMVector3Cross((triV3 - triV2), (triV1 - triV2));
	if(XMVectorGetX(XMVector3Dot(cp1, cp2)) >= 0)
	{
		cp1 = XMVector3Cross((triV3 - triV1), (point - triV1));
		cp2 = XMVector3Cross((triV3 - triV1), (triV2 - triV1));
		if(XMVectorGetX(XMVector3Dot(cp1, cp2)) >= 0)
		{
			cp1 = XMVector3Cross((triV2 - triV1), (point - triV1));
			cp2 = XMVector3Cross((triV2 - triV1), (triV3 - triV1));
			if(XMVectorGetX(XMVector3Dot(cp1, cp2)) >= 0)
			{
				return true;
			}
			else
				return false;
		}
		else
			return false;
	}
	return false;
}



void Direct3D::CreateSkyBox(int numLatitude, int numLongitude)
{
	numSkyVertices = ((numLatitude-2) * numLongitude) + 2;
	numSkyFaces  = ((numLatitude-3)*(numLongitude)*2) + (numLongitude*2);

	float skyYaw = 0.0f;
	float skyPitch = 0.0f;

	vector<Vertex> vertices(numSkyVertices);

	XMVECTOR currVertPos = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	vertices[0].pos.x = 0.0f;
	vertices[0].pos.y = 0.0f;
	vertices[0].pos.z = 1.0f;

	for(DWORD i = 0; i < numLatitude-2; ++i)
	{
		skyPitch = (i+1) * (3.14/(numLatitude-1));
		XMMATRIX Rotationx = XMMatrixRotationX(skyPitch);
		for(DWORD j = 0; j < numLongitude; ++j)
		{
			skyYaw = j * (6.28/(numLongitude));
			XMMATRIX Rotationy = XMMatrixRotationZ(skyYaw);
			currVertPos = XMVector3TransformNormal( XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), (Rotationx * Rotationy) );	
			currVertPos = XMVector3Normalize( currVertPos );
			vertices[i*numLongitude+j+1].pos.x = XMVectorGetX(currVertPos);
			vertices[i*numLongitude+j+1].pos.y = XMVectorGetY(currVertPos);
			vertices[i*numLongitude+j+1].pos.z = XMVectorGetZ(currVertPos);
		}
	}

	vertices[numSkyVertices-1].pos.x =  0.0f;
	vertices[numSkyVertices-1].pos.y =  0.0f;
	vertices[numSkyVertices-1].pos.z = -1.0f;


	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory( &vertexBufferDesc, sizeof(vertexBufferDesc) );

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof( Vertex ) * numSkyVertices;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData; 

	ZeroMemory( &vertexBufferData, sizeof(vertexBufferData) );
	vertexBufferData.pSysMem = &vertices[0];
	d3d11Device->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &skyVertBuffer);


	vector<DWORD> indices(numSkyFaces * 3);

	int k = 0;
	for(DWORD l = 0; l < numLongitude-1; ++l)
	{
		indices[k] = 0;
		indices[k+1] = l+1;
		indices[k+2] = l+2;
		k += 3;
	}

	indices[k] = 0;
	indices[k+1] = numLongitude;
	indices[k+2] = 1;
	k += 3;

	for(DWORD i = 0; i < numLatitude-3; ++i)
	{
		for(DWORD j = 0; j < numLongitude-1; ++j)
		{
			indices[k]   = i*numLongitude+j+1;
			indices[k+1] = i*numLongitude+j+2;
			indices[k+2] = (i+1)*numLongitude+j+1;

			indices[k+3] = (i+1)*numLongitude+j+1;
			indices[k+4] = i*numLongitude+j+2;
			indices[k+5] = (i+1)*numLongitude+j+2;

			k += 6; // next quad
		}

		indices[k]   = (i*numLongitude)+numLongitude;
		indices[k+1] = (i*numLongitude)+1;
		indices[k+2] = ((i+1)*numLongitude)+numLongitude;

		indices[k+3] = ((i+1)*numLongitude)+numLongitude;
		indices[k+4] = (i*numLongitude)+1;
		indices[k+5] = ((i+1)*numLongitude)+1;

		k += 6;
	}

	for(DWORD l = 0; l < numLongitude-1; ++l)
	{
		indices[k] = numSkyVertices-1;
		indices[k+1] = (numSkyVertices-1)-(l+1);
		indices[k+2] = (numSkyVertices-1)-(l+2);
		k += 3;
	}

	indices[k] = numSkyVertices-1;
	indices[k+1] = (numSkyVertices-1)-numLongitude;
	indices[k+2] = numSkyVertices-2;

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory( &indexBufferDesc, sizeof(indexBufferDesc) );

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * numSkyFaces * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = &indices[0];
	d3d11Device->CreateBuffer(&indexBufferDesc, &iinitData, &skyIndexBuffer);

}




void Direct3D::RenderGameOver()
{
	d3d11DevCon->VSSetShader(VS, 0, 0);
	d3d11DevCon->PSSetShader(OVER_PS, 0, 0);

	//Set Place for Overlay
	gameOverWorld = XMMatrixOrthographicRH(2, 1.5f, 1.0f, 100.0f);

	d3d11DevCon->PSSetShaderResources( 0, 1, &GameOverTexture );

	WVP = gameOverWorld; 
	cbPerObj.World = XMMatrixTranspose(gameOverWorld);
	cbPerObj.WVP = XMMatrixTranspose(WVP);	
	d3d11DevCon->UpdateSubresource( cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0 );
	d3d11DevCon->VSSetConstantBuffers( 0, 1, &cbPerObjectBuffer );

	//just draw the damn thing
	d3d11DevCon->DrawIndexed( 6, 42, 0 );

}

void Direct3D::RenderTurn(const char *name)
{
	d3d11DevCon->VSSetShader(VS, 0, 0);
	d3d11DevCon->PSSetShader(OVER_PS, 0, 0);

	
	//Set Place for Overlay
	playerTurnWorld = XMMatrixOrthographicRH(2.0f, 2.0f, 1.0f, 100.0f);

	//choose texture
	if (name == "1")
		d3d11DevCon->PSSetShaderResources( 0, 1, &Player1OverTexture );
	else if (name == "2")
		d3d11DevCon->PSSetShaderResources( 0, 1, &Player2OverTexture );
	else
		d3d11DevCon->PSSetShaderResources( 0, 1, &PlayerAIOverTexture );

	WVP = playerTurnWorld; 
	cbPerObj.World = XMMatrixTranspose(playerTurnWorld);
	cbPerObj.WVP = XMMatrixTranspose(WVP);	
	d3d11DevCon->UpdateSubresource( cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0 );
	d3d11DevCon->VSSetConstantBuffers( 0, 1, &cbPerObjectBuffer );

	//just draw the damn thing
	d3d11DevCon->DrawIndexed( 6, 48, 0 );
}



bool Direct3D::InitDirectInput(HINSTANCE hInstance, HWND hwnd)
{
	DirectInput8Create(hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&DirectInput,
		NULL); 

	DirectInput->CreateDevice(GUID_SysKeyboard,
		&DIKeyboard,
		NULL);

	DirectInput->CreateDevice(GUID_SysMouse,
		&DIMouse,
		NULL);

	DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	DIKeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	DIMouse->SetDataFormat(&c_dfDIMouse);
	DIMouse->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);

	return true;
}

void Direct3D::DetectCamInput(double time, HWND hwnd)
{
	DIMOUSESTATE mouseCurrState;
	BYTE keyboardState[256];

	DIKeyboard->Acquire();
	DIMouse->Acquire();

	DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);
	DIKeyboard->GetDeviceState(sizeof(keyboardState),(LPVOID)&keyboardState);

	if(keyboardState[DIK_A] & 0x80)
	{
		SetCamX(-37.0f*time);
	}
	if(keyboardState[DIK_D] & 0x80)
	{
		SetCamX(+37.0f*time);
	}
	if(keyboardState[DIK_W] & 0x80)
	{
		 SetCamY(+1.5f*time);
	}
	if(keyboardState[DIK_S] & 0x80)
	{
		 SetCamY(-1.5f*time);
	}
	if(keyboardState[DIK_Q] & 0x80)
	{
		SetCamZ(+2.5f*time);
	}
	if(keyboardState[DIK_E] & 0x80)
	{
		SetCamZ(-2.5f*time);
	}
	if(mouseCurrState.lZ < 0.0f)
	{
		SetCamZ(+2.5f*time);
	}
	if(mouseCurrState.lZ > 0.0f)
	{
		SetCamZ(-2.5f*time);
	}

	mouseLastState = mouseCurrState;

	return;
}


