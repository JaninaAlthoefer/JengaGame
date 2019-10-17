#include <windows.h>
#include "Direct3D.h"
#include "Physics.h"
#include "Sound.h"
#include "AI.h"


HWND hwnd = NULL;

UINT WIDTH = 800;
UINT HEIGHT = 600;

class Physics *phys;
class Direct3D *d3d;
class Sound *d8Sound;
class AI *ai;

//stuff
float dist = 0.0f;
int selecter = -1;
int pickedBlock = -1;

int lastBlock = -1;

int lastX;
int lastY;

//player stuff
bool playAI = false;
bool hasAI = false;

char *firstPlayer = "1";
char *secondPlayer = "2";
char *aiPlayer = "ai";
char *currPlayer = firstPlayer;

//states
bool statePicking = true;
bool standing = true;
bool donePicking = false;

//timer
__int64 frameTimeOld = 0;
float countsPerSecond;
float gameTimeCountForWait = 0;

//functions go here
bool InitWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool windowed);
void SetCurrentPlayer(const char *name);
float GetDeltaTime();
void Wait(int sec);
void initMP();
void changePlayer();
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


int WINAPI WinMain(HINSTANCE hInstance,	HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{

	if(!InitWindow(hInstance, nShowCmd, WIDTH, HEIGHT, true))
	{
		MessageBox(0, L"Window Initialization - Failed",
			L"Error", MB_OK);
		exit(1);
	}

	phys = new Physics();
	d3d = new Direct3D(hwnd);
	
	if(!d3d->InitScene())
	{
		MessageBox(0, L"Scene Initialization - Failed",
			L"Error", MB_OK);
		exit(1);
	}


	if(!d3d->InitDirectInput(hInstance, hwnd))
	{
		MessageBox(0, L"Direct Input Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}


	
	d8Sound = new Sound(hwnd);

	if(!d8Sound)
	{
		MessageBox(0, L"Sound Initialization - Failed",
			L"Error", MB_OK);
		exit(1);
	}
	d8Sound->playGameMusic(); //*/
	d8Sound->toggleMusic();

	initMP();

	if (playAI)
	{
		ai = new AI(phys, d8Sound);
		hasAI = true;
	}

	//messageloop
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while(true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);	
			DispatchMessage(&msg);
		}
		else
		{
			// run game code here
			
			vector<int> leftOverBlocks = phys->onGround();
			
			if((!phys->stillStanding()||!leftOverBlocks.empty()||ai->giveUp())&&standing)
			{                                                   //ai giveup overlay missing
				standing = false;
				selecter = -1;
				float h = phys->getTowerHeight();
				d3d->SetGameOverCamera();
				phys->dropBlock(&selecter);
				statePicking = true;
				d8Sound->playFallingSF();
				//MessageBox(0, L"Game Over", L"Error", MB_OK); 
			} 

			if(phys->outOfTower(&selecter)&&statePicking)
			{
				pickedBlock = selecter;
				selecter = -1;
				phys->disableCollision(&pickedBlock);
				//MessageBox(0, L"OutOfTower", L"Error", MB_OK);
				statePicking = false;
			}

			if (!statePicking)
			{
				float h = phys->getTowerHeight();
				d3d->SetPlacingStateCamera(h);
			}

			

			float time = GetDeltaTime();

			//manage AI
			if (currPlayer == aiPlayer && standing)
			{
				states stat = ai->getCurrentState();

				if (stat == FINISHED)
				{
					changePlayer();
					ai->setCurrentState(PICK);
				}
				else
					ai->play(time);
			}

			phys->stepPhysX(time);
			d3d->DetectCamInput(time, hwnd);

			d3d->UpdateScene(phys);
			d3d->DrawScene(&selecter, &standing, currPlayer);

			//sound effects
			for (int i = 0; i < numBlocks; i++)
			{
				bool result = phys->fallingBlock(&i);
				
				if (standing&&result)
					d8Sound->playCollisionSF(i);

			}
			

		}
	}

	delete d8Sound;
	if (hasAI)
		delete ai;	

	delete d3d;
	delete phys;

	return msg.wParam;
}

bool InitWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool windowed)
{
	typedef struct _WNDCLASS {
		UINT cbSize;
		UINT style;
		WNDPROC lpfnWndProc;
		int cbClsExtra;
		int cbWndExtra;
		HANDLE hInstance;
		HICON hIcon;
		HCURSOR hCursor;
		HBRUSH hbrBackground;
		LPCTSTR lpszMenuName;
		LPCTSTR lpszClassName;
	} WNDCLASS;

	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"firstwindow";
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Error registering class",	
			L"Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	RECT wr = {0, 0, WIDTH, HEIGHT};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	hwnd = CreateWindowEx(
		NULL,
		L"firstwindow",
		L"Jenga",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		wr.right-wr.left, wr.bottom - wr.top,
		NULL,
		NULL,
		hInstance,
		NULL
		);

	if (!hwnd)
	{
		MessageBox(NULL, L"Error creating window",
			L"Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	ShowWindow(hwnd, ShowWnd);
	UpdateWindow(hwnd);

	return true;
}

void SetCurrentPlayer(const char *name)
{
	*currPlayer = *name;
}

float GetDeltaTime()
{
	LARGE_INTEGER frequencyCount;
	QueryPerformanceFrequency(&frequencyCount);

	countsPerSecond = float(frequencyCount.QuadPart);

	LARGE_INTEGER currentTime;
	__int64 tickCount;
	QueryPerformanceCounter(&currentTime);

	tickCount = currentTime.QuadPart-frameTimeOld;
	frameTimeOld = currentTime.QuadPart;

	if(tickCount < 0.0f)
		tickCount = 0.0f;

	return float(tickCount)/countsPerSecond;
}

void Wait(int sec)
{
	LARGE_INTEGER gTCFWait;
	QueryPerformanceCounter(&gTCFWait);

	gameTimeCountForWait = gameTimeCountForWait + float (sec * gTCFWait.QuadPart);
}

void initMP()
{
	int answer = MessageBox(0, L"Play against AI?", L"Question", MB_YESNO | MB_ICONQUESTION);
	if (answer == 0)
	{
		MessageBox(0, L"Couldn't initialize Multiplayer", L"Error", MB_OK);
	}
	if (answer == IDYES)
	{
		if(!hasAI)
		{
			ai = new AI(phys, d8Sound);
			hasAI = true;
		}
		playAI = true;
	}
	else
		playAI = false;

}

void changePlayer()
{
	if (currPlayer == aiPlayer || currPlayer == secondPlayer)
		currPlayer = firstPlayer;
	else if (currPlayer == firstPlayer && !playAI)
		currPlayer = secondPlayer;
	else if (currPlayer == firstPlayer && playAI)
		currPlayer = aiPlayer;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_KEYDOWN:
		{
			/*int xPos = (short) LOWORD (lParam);
			int yPos = (short) HIWORD (lParam);

			float position[4], direction[4];

			d3d->PickScreenSpaceToWorldSpace(xPos, yPos, position, direction);
			d3d->PickFromWorldPosAndDir(&selecter, position, direction, &dist);

			lastX = xPos;
			lastY = yPos; //*/

			switch (wParam)
			{/*
			case VK_RETURN: {	if (!statePicking) 
								{
									float h = phys->getTowerHeight();
									d3d->SetPickingStateCamera(h);
									statePicking = !statePicking;
								}
								changePlayer();
								
								break; }
							*/
			case VK_ESCAPE: { DestroyWindow(hwnd); break; return 0;}

			case VK_SPACE:  { phys->buildTower(); standing = true; 
							statePicking = true;
							float h = phys->getTowerHeight();
							d3d->SetPickingStateCamera(h/2);
							initMP();
							if (playAI)
								ai->setCurrentState(PICK);
							currPlayer = firstPlayer;
							break; }

			case 'm':
			case 'M':{
					 d8Sound->toggleMusic();
					 break;}
			default:
				break;
			}
		}
		return 0;

	case WM_MOUSEMOVE:
		{
			if (currPlayer == aiPlayer)
				return 0;

			if (standing&&statePicking)
			{
				if (wParam == MK_RBUTTON)
				{
					int xPos = (short) LOWORD (lParam);
					int yPos = (short) HIWORD (lParam);

					float position[4], direction[4];
					d3d->PickScreenSpaceToWorldSpace(xPos, yPos, position, direction);

					float deltaX = xPos - lastX;
					float deltaY = yPos - lastY;
					
					float length = sqrtf((deltaX*deltaX)+(deltaY*deltaY));

					float angle = atan2f(deltaX, deltaY);

					phys->updateSpringPos(angle, 0.0f, direction);

					lastX = xPos;
					lastY = yPos;
				}
				else
				{ 
					int xPos = (short) LOWORD (lParam);
					int yPos = (short) HIWORD (lParam);

					float position[4], direction[4];

					d3d->PickScreenSpaceToWorldSpace(xPos, yPos, position, direction);
					d3d->PickFromWorldPosAndDir(&selecter, position, direction, &dist);

					lastX = xPos;
					lastY = yPos;
				} 
			}

			if (standing&&!statePicking)
			{
				int xPos = (short) LOWORD (lParam);
				int yPos = (short) HIWORD (lParam);

				float mouseX =  ((( 2.0f * xPos) / WIDTH ) - 1 );
				float mouseY = -((( 2.0f * yPos) / HEIGHT) - 1 );

				phys->moveBlock(&pickedBlock, mouseX, mouseY, WIDTH, HEIGHT);
			}

			return 0;
		}

	case WM_LBUTTONUP: 
		{
			if (currPlayer == aiPlayer)
				return 0;

			if (standing&&statePicking)
			{
				int xPos = (short) LOWORD (lParam);
				int yPos = (short) HIWORD (lParam);

				float position[4], direction[4];

				d3d->PickScreenSpaceToWorldSpace(xPos, yPos, position, direction);

				if (phys->pushBlock(&selecter, direction))
					d8Sound->playCollisionSF(selecter);

			}
			return 0;
		}

	case WM_RBUTTONDOWN:
		{
			if (currPlayer == aiPlayer)
				return 0;

			if (standing&&statePicking)
			{
				int xPos = (short) LOWORD (lParam);
				int yPos = (short) HIWORD (lParam);

				float position[4], direction[4];
				d3d->PickScreenSpaceToWorldSpace(xPos, yPos, position, direction);
				
				phys->addSpring(&selecter, position, direction); 

				lastX = xPos;
				lastY = yPos;

			}

			if (standing&&!statePicking)
			{
				donePicking = true;
			}
			
			return 0;
		}

	case WM_RBUTTONUP:
		{
			if (currPlayer == aiPlayer)
				return 0;

			if (standing&&statePicking)
				phys->releaseSpring();

			if (standing&&!statePicking&&donePicking)
			{
				phys->dropBlock(&pickedBlock);
				//d8Sound->playCollisionSF(pickedBlock);
				float h = phys->getTowerHeight();
				d3d->SetPickingStateCamera(h);
				statePicking = true;
				donePicking = false;
				lastBlock = selecter;
				changePlayer();
				selecter = -1;
				pickedBlock = -1;
			}
			
			return 0;
		}

	/*case WM_MOUSEWHEEL:
		{
			int xPos = (short) LOWORD (lParam);
			int yPos = (short) HIWORD (lParam);

			float position[4], direction[4];

			d3d->PickScreenSpaceToWorldSpace(xPos, yPos, position, direction);
			d3d->PickFromWorldPosAndDir(&selecter, position, direction, &dist);

			lastX = xPos;
			lastY = yPos;

			if (statePicking)
			{
				short delta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;

				if (delta < 0) d3d->SetCamZ(-0.1f);
				else if (delta > 0) d3d->SetCamZ(+0.1f);
				return 0; 
			}
			return 0;
		}//*/

	case WM_SIZE:
		{
			RECT rect;
			if (GetWindowRect(hwnd, &rect) && d3d != NULL)
			{
				WIDTH = rect.right - rect.left;
				HEIGHT = rect.bottom - rect.top;

				d3d->OnWindowResize(WIDTH, HEIGHT);
			}

			return 0;
		}

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	} 
	return DefWindowProc(hwnd, msg, wParam, lParam);
}