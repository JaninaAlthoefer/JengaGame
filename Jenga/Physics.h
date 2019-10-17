#pragma once

#include <iostream>
#include <vector>
#include <time.h>
#include <PxPhysicsAPI.h>  //PhysX main header file
#include "GlobalSizes.h"

#ifdef _DEBUG
#pragma comment(lib, "PhysX3DEBUG_x86.lib")
#pragma comment(lib, "PhysX3CommonDEBUG_x86.lib")
#pragma comment(lib, "PhysX3ExtensionsDEBUG.lib")
#else
#pragma comment(lib, "PhysX3_x86.lib")
#pragma comment(lib, "PhysX3Common_x86.lib")
#pragma comment(lib, "PhysX3Extensions.lib")
#endif

using namespace std;
using namespace physx;

//const int numBlocks = 54;

class Physics
{
private:

	

public:
	Physics(void);
	~Physics(void);
	void stepPhysX(float time);
	void getBoxPoseRender(const int *num, float *mat);
	void buildTower();

	//picking
	void addSpring(const int *num, const float *pos, const float *dir);
	void updateSpringPos(float angleInRadians, float length, const float *dir);
	void releaseSpring();	
	bool pushBlock(const int *num, const float *dir);

	//placing
	void disableCollision(const int *num);
	void moveBlock(const int *i, float x, float y, int screenW, int screenH);
	void dropBlock(const int *i);

	float getTowerHeight();
	bool stillStanding();
	bool outOfTower(const int *num);
	bool fallingBlock(const int *num);

	vector<int> onGround();
	bool turnBlock();
	void adjustNumHighestBlocks();
	//int numBlocksOnHighestLayer();

	//ai
	void getTopBlockLayout(blockLayout *layout);
	void getBlockLayout(int *num, blockLayout *layout);
	vector<int> getUnusableBlocks();
	PxVec3 getBlockPosition(int *num);
	float getTopVelocity();
	void getHighestMiddleBlockPosition(float *position);

	bool aiOutOfTower(const int *num);
	//void aiAdjustHighestBlocks(int *num);

	void aiPushBlock(int *num);
	void aiPullBlock(int *num, bool way);
	void aiDisableCollision(int *num);
	void aiMoveBlock(int *num, float *position, float *delta, bool turned);
	void aiDropBlock(int *num); //, PxTransform *position);

};

