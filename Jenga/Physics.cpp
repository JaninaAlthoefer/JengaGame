#include "Physics.h"

//testing
//PxVec3 dirTest;

//moving with tower try out
//PxVec3 lastPosition;

static PxDefaultAllocator gDefaultAllocatorCallback;
static PxDefaultErrorCallback gDefaultErrorCallback;
static PxFoundation* gFoundation = NULL;
static PxPhysics* gPhysicsSDK = NULL;

int currBlock = -1;
int numBlocksHighest = 0;

float minHeight = halfBoxShortSide * 36.0f;  // *2 for halfHeight
float poseOffset = 0.01f;
float randFriction = 0.1f;
float randSize = 0.00075f;

float timeAccumulator = 0.0f;
float stepSize = 1.0f/60.0f;

PxScene* gScene = NULL;
PxMaterial *woodMat1 = NULL, *woodMat2 = NULL, *woodMat3 = NULL;
PxRigidStatic* plane;

float springConstant = 35.0f; // 0.375f;
PxVec3 localPosBlock;
PxVec3 globalPosMouse;
float springNormalLength;

vector<PxRigidDynamic*> blockArray;
//vector<int> indexOfLayerBlocks;
vector<int> highestBlocks;

Physics::Physics ()
{
	srand(time(NULL) * time(NULL));

	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
	gPhysicsSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale());

	if(gPhysicsSDK == NULL)
	{
		cerr<<"Error creating PhysX3 device, Exiting..."<<endl;
		exit(1);
	}

	PxSceneDesc sceneDesc(gPhysicsSDK->getTolerancesScale());

	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0);
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1);
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;

	gScene = gPhysicsSDK->createScene(sceneDesc);

	PxMaterial* stoneMat = gPhysicsSDK->createMaterial(5.0, 0.5f, 0.1f);
	PxTransform planePos = PxTransform(PxVec3(0.0f, 0.0f, 0.0f), PxQuat(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f)));
	plane = gPhysicsSDK->createRigidStatic(planePos);
	plane->createShape(PxPlaneGeometry(), *stoneMat);
	gScene->addActor(*plane);

	buildTower();

	//highestBlocks.push_back();
	
}

Physics::~Physics()
{
	gScene->release();
	//PxCloseExtensions();
	gPhysicsSDK->release();
	gFoundation->release();
}

void Physics::buildTower()
{
	if (blockArray.size() > 0)
	{
		for (int i = 0; i < numBlocks; i++)
		{
			gScene->removeActor(*blockArray[i]);
		} 
	}
	blockArray.clear(); 

	bool yes = true;

	for (int i=0; i < numBlocks/3; i++)
	{
		float sFricRand1 = 0.0f - ((float(rand())/float(RAND_MAX))*randFriction);
		float dFricRand1 = 0.0f - ((float(rand())/float(RAND_MAX))*randFriction);

		float sFricRand2 = 0.0f - ((float(rand())/float(RAND_MAX))*randFriction);
		float dFricRand2 = 0.0f - ((float(rand())/float(RAND_MAX))*randFriction);

		float sFricRand3 = 0.0f - ((float(rand())/float(RAND_MAX))*randFriction);
		float dFricRand3 = 0.0f - ((float(rand())/float(RAND_MAX))*randFriction);


		float shortRand1 = randSize - ((float(rand())/float(RAND_MAX))*2*randSize);
		float middleRand1 = randSize - ((float(rand())/float(RAND_MAX))*2*randSize);
		float longRand1 = randSize - ((float(rand())/float(RAND_MAX))*2*randSize);

		float shortRand2 = randSize - ((float(rand())/float(RAND_MAX))*2*randSize);
		float middleRand2 = randSize - ((float(rand())/float(RAND_MAX))*2*randSize);
		float longRand2 = randSize - ((float(rand())/float(RAND_MAX))*2*randSize);

		float shortRand3 = randSize - ((float(rand())/float(RAND_MAX))*2*randSize);
		float middleRand3 = randSize - ((float(rand())/float(RAND_MAX))*2*randSize);
		float longRand3 = randSize - ((float(rand())/float(RAND_MAX))*2*randSize);

		if (shortRand1 < shortRand2 && shortRand2 <= shortRand3)
		{
			if(yes)
				shortRand3 = shortRand1;
			else
				shortRand2 = shortRand1;

			yes = !yes;
		}
		else if (shortRand1 > shortRand2 && shortRand3 > shortRand2)
		{	
			if (!yes)
				shortRand1 = shortRand2;
			else 
				shortRand3 = shortRand2;

			yes = !yes;
		}
		else if (shortRand1 > shortRand2 && shortRand2 >= shortRand3)
		{
			if(yes)
				shortRand3 = shortRand1;
			else
				shortRand2 = shortRand3;

			yes = !yes;
		}


		woodMat1 = gPhysicsSDK->createMaterial(0.5f+sFricRand1, 0.2f+dFricRand1, 0.603f);
		woodMat2 = gPhysicsSDK->createMaterial(0.5f+sFricRand2, 0.2f+dFricRand2, 0.603f);
		woodMat3 = gPhysicsSDK->createMaterial(0.5f+sFricRand3, 0.2f+dFricRand3, 0.603f);

		PxTransform bPos1, bPos2, bPos3;
		PxRigidDynamic *temp1 = NULL, *temp2 = NULL, *temp3 = NULL;

		if (i%2 != 0)
		{
			bPos1 = PxTransform(PxVec3(0.0f, (float(i) * 2*halfBoxShortSide)+0.1f, (-poseOffset - 2*halfBoxMiddleSide)), PxQuat(PxHalfPi, PxVec3(0.0f, 1.0f, 0.0f)));
			bPos2 = PxTransform(PxVec3(0.0f, (float(i) * 2*halfBoxShortSide)+0.1f, 0.0f), PxQuat(PxHalfPi, PxVec3(0.0f, 1.0f, 0.0f)));
			bPos3 = PxTransform(PxVec3(0.0f, (float(i) * 2*halfBoxShortSide)+0.1f, (+poseOffset + 2*halfBoxMiddleSide)), PxQuat(PxHalfPi, PxVec3(0.0f, 1.0f, 0.0f)));
		}
		else
		{
			bPos1 = PxTransform(PxVec3((-poseOffset - 2*halfBoxMiddleSide), (float(i) * 2*halfBoxShortSide)+0.1f, 0.0f)); //+halfBoxShortSide
			bPos2 = PxTransform(PxVec3(0.0f, (float(i) * 2*halfBoxShortSide)+0.1f, 0.0f));
			bPos3 = PxTransform(PxVec3((+poseOffset + 2*halfBoxMiddleSide), (float(i) * 2*halfBoxShortSide)+0.1f, 0.0f));
		}

		PxBoxGeometry bGeometry1 = PxBoxGeometry(PxVec3(halfBoxMiddleSide+middleRand1,halfBoxShortSide+shortRand1,halfBoxLongSide+longRand1));
		temp1 = PxCreateDynamic(*gPhysicsSDK, bPos1, bGeometry1, *woodMat1, 1.0f);
		gScene->addActor(*temp1);

		blockArray.push_back(temp1);

		PxBoxGeometry bGeometry2 = PxBoxGeometry(PxVec3(halfBoxMiddleSide+middleRand2,halfBoxShortSide+shortRand2,halfBoxLongSide+longRand2));
		temp2 = PxCreateDynamic(*gPhysicsSDK, bPos2, bGeometry2, *woodMat2, 1.0f);
		gScene->addActor(*temp2);

		blockArray.push_back(temp2);

		PxBoxGeometry bGeometry3 = PxBoxGeometry(PxVec3(halfBoxMiddleSide+middleRand3,halfBoxShortSide+shortRand3,halfBoxLongSide+longRand3));
		temp3 = PxCreateDynamic(*gPhysicsSDK, bPos3, bGeometry3, *woodMat3, 1.0f);
		gScene->addActor(*temp3);

		blockArray.push_back(temp3);
	}

	if (!highestBlocks.empty())
		highestBlocks.clear();

	highestBlocks.push_back(49);
	highestBlocks.push_back(50);
	highestBlocks.push_back(51);
	highestBlocks.push_back(52);
	highestBlocks.push_back(53);

	numBlocksHighest = 0;
}

void Physics::stepPhysX(float time)
{
	timeAccumulator += time;

	if (timeAccumulator < (stepSize))
		return;

	timeAccumulator -= stepSize;

	gScene->simulate(stepSize);
	gScene->fetchResults(true);
}

void Physics::getBoxPoseRender(const int *num, float *mat)
{
	if (*num < 0 ) 
		return;

	PxTransform trans = blockArray[*num]->getGlobalPose();
	PxMat44 m = PxMat44(trans);

	mat[0]  = m.column0.x;
	mat[1]  = m.column0.y;
	mat[2]  = m.column0.z;
	mat[3]  = m.column0.w;

	mat[4]  = m.column1.x;
	mat[5]  = m.column1.y;
	mat[6]  = m.column1.z;
	mat[7]  = m.column1.w;

	mat[8]  = m.column2.x;
	mat[9]  = m.column2.y;
	mat[10] = m.column2.z;
	mat[11] = m.column2.w;

	mat[12] = m.column3.x;// + halfBoxMiddleSide;
	mat[13] = m.column3.y;// + halfBoxShortSide;
	mat[14] = m.column3.z;// + halfBoxLongSide;
	mat[15] = m.column3.w;
}

void Physics::addSpring(const int *num, const float *pos, const float *dir)
{
	//avoid wrong blocks
	if (*num < 0) return;

	vector<int> unusableBlocks = highestBlocks;
	int start = 2 - numBlocksHighest;
	for (int i = start; i< 5; i++)
	{
		if (unusableBlocks[i]==*num)
			return;
	}
	
	currBlock = *num;

	PxVec3 startPoint = PxVec3(pos[0], pos[1], pos[2]);
	//PxQuat(PxHalfPi, PxVec3(0.0f, 1.0f, 0.0f)).rotate(startPoint);

	//dirTest = PxVec3(dir[0], dir[1], dir[2]);
	PxVec3 goUnit = PxVec3(dir[0], dir[1], dir[2]);
	goUnit.normalize();
	PxRaycastBuffer hit;

	bool status; 
	status = gScene->raycast(startPoint, goUnit, 100.0f, hit);

	/*
	int whichBlock;

	if (status)
	{
		for (int i = 0; i <numBlocks; i++)
		{
			if (hit.block.actor == blockArray[i])
				whichBlock = i;

		} 
	}

	if(hit.block.actor == plane)
		whichBlock = -2; //*/


	if (!status)
	{
		PxVec3 tempBlockPos = blockArray[currBlock]->getGlobalPose().p;
		globalPosMouse = tempBlockPos;
		localPosBlock =  tempBlockPos;
	}
	else
	{//*/
		PxVec3 tempBlockPos = blockArray[currBlock]->getGlobalPose().p;
		globalPosMouse = hit.block.position;
		//globalPosMouse.y = tempBlockPos.y;
		localPosBlock =  (globalPosMouse - tempBlockPos);

	}	

	springNormalLength = 0.0f;

}

void Physics::updateSpringPos(float angleInRadians, float length, const float *dir)
{
	if (currBlock < 0)
		return;

	vector<int> unusableBlocks = highestBlocks;
	int start = 2 - numBlocksHighest;
	for (int i = start; i< 5; i++)
	{
		if (unusableBlocks[i]==currBlock)
			return;
	}

	float angle = angleInRadians + PxPi;

	float xDir = cosf(angle) * dir[0] - sinf(angle) * dir[2];
	float zDir = sinf(angle) * dir[0] + cosf(angle) * dir[2];

	//PxTransform trans = PxTransform(PxVec3(xDir, 0.0f, zDir), PxQuat(PxHalfPi, PxVec3(0.0f, 1.0f, 0.0f)));
	PxVec3 nDir = PxVec3(xDir, 0.0f, zDir);
	
	//PxRigidBodyExt::addForceAtPos(*blockArray[currBlock], nDir, globalPosMouse, PxForceMode::eIMPULSE);
	nDir.normalize();

	///*
	globalPosMouse = globalPosMouse + (0.1f * nDir);

	PxVec3 dirBetweenBoxAndMouse = globalPosMouse - (blockArray[currBlock]->getGlobalPose().p + localPosBlock);
	dirBetweenBoxAndMouse.y = 0.0f;
	float dirBetweenBoxAndMouseLength = dirBetweenBoxAndMouse.magnitude();

	float deltaLength = dirBetweenBoxAndMouseLength - springNormalLength;

	dirBetweenBoxAndMouse.normalize();
	
	PxVec3 forceToAdd = springConstant * dirBetweenBoxAndMouse;// (deltaLength * springConstant) * dirBetweenBoxAndMouse;
	//forceToAdd.y = 0.0f;

	//blockArray[currBlock]->setAngularVelocity(PxVec3(0.0f, 0.0f, 0.0f));
	PxRigidBodyExt::addForceAtLocalPos(*blockArray[currBlock], forceToAdd, localPosBlock, PxForceMode::eFORCE);
	//*/
}

void Physics::releaseSpring()
{
	currBlock = -1;
	springNormalLength = 0.0f;

	//localPosBlock = PxVec3(0.0f);
	//globalPosMouse = PxVec3(0.0f);
}

bool Physics::pushBlock(const int *num, const float *dir)
{
	
	if (*num < 0) return false;

	vector<int> unusableBlocks = highestBlocks;
	int top = numBlocksHighest;
	int start = 2 - top ; //(numBlocksOnHighestLayer()%3);
	for (int i = start; i< 5; i++)
	{
		if (unusableBlocks[i]==*num)
			return false;
	}

	//blockArray[*num]->setLinearVelocity(PxVec3(2*dir[0], 0.0f, 2*dir[2]));
	PxVec3 impulse = PxVec3(dir[0], 0.0f, dir[2]);
	PxVec3 position = blockArray[*num]->getGlobalPose().p;
	PxRigidBodyExt::addForceAtPos(*blockArray[*num], impulse, position, PxForceMode::eIMPULSE);

	return true;
}

void Physics::disableCollision(const int *num)
{
	if (*num < 0)
		return;

	currBlock = *num;
	gScene->removeActor(*blockArray[currBlock]);
	blockArray[currBlock]->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);

}

void Physics::moveBlock(const int *i, float x, float y, int screenW, int screenH)
{
	if (currBlock < 0) 
		return;

	PxTransform tempPos;
	
	//align mouse
	float mouseX = 4.0f * ((cosf(0.75f * 3.14156)*x) - (sinf(0.75f * 3.14156)*y)); // + (x+y)*halfBoxMiddleSide;
	float mouseZ = 4.0f * ((sinf(0.75f * 3.14156)*x) + (cosf(0.75f * 3.14156)*y)); // + (x+y)*halfBoxMiddleSide;

	//align block
	vector<int> unusableBlocks = highestBlocks;
	int lookUp = unusableBlocks[2]; //immer 90° zu dem wie muss
	bool res = turnBlock();

	float h = blockArray[lookUp]->getGlobalPose().p.y;


	PxVec3 position = PxVec3(mouseX, (h+2.1f*halfBoxShortSide), mouseZ); //blockArray[48]->getGlobalPose().p;// PxVec3(0.0f, (h+2.1f*halfBoxShortSide), 0.0f);
	
	if (res)
	{
		tempPos = PxTransform(position, PxQuat(PxHalfPi, PxVec3(0.0f, 1.0f, 0.0f)));
	}
	else
	{
		tempPos = PxTransform(position);
	}
	
	

	blockArray[currBlock]->setGlobalPose(tempPos);
}

void Physics::dropBlock(const int *i)
{
	
	if (currBlock < 0) return; 

	gScene->addActor(*blockArray[currBlock]);
	blockArray[currBlock]->setLinearVelocity(PxVec3(0.0f, 0.0f, 0.0f));
	blockArray[currBlock]->setAngularVelocity(PxVec3(0.0f, 0.0f, 0.0f));
	blockArray[currBlock]->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false);

	//stop here if re-pickup - not needed anymore??
	if (highestBlocks[4]==currBlock)
		return; 
	if (highestBlocks[3]==currBlock)
		return;

	adjustNumHighestBlocks();

	highestBlocks.push_back(currBlock);
	highestBlocks.erase(highestBlocks.begin());

	currBlock = -1;

}

float Physics::getTowerHeight()
{
	float tHeight = 0;

	for (int i =0; i < numBlocks; i++)
	{
		PxVec3 temp = blockArray[i]->getGlobalPose().p;

		if ((i != currBlock)&&(temp.y > tHeight))
			tHeight = temp.y;
	}

	return tHeight;
}

bool Physics::stillStanding()
{
	return (getTowerHeight() >= (0.9f * minHeight));
}

bool Physics::outOfTower(const int *num)
{
	if (*num < 0) return false;

	PxVec3 tempPose = blockArray[*num]->getGlobalPose().p;
	
	if (tempPose.x >= 5.7f*halfBoxMiddleSide || tempPose.x <= -5.7f*halfBoxMiddleSide || tempPose.z >= 5.7f*halfBoxMiddleSide || tempPose.z <= -5.7f*halfBoxMiddleSide)
	{
		currBlock = *num;
		return true;
	}
	else
		return false;
}

bool Physics::fallingBlock(const int *num)
{
	bool res;

	PxVec3 velVec = blockArray[*num]->getLinearVelocity();
	float vel = velVec.magnitudeSquared();

	PxVec3 posVec = blockArray[*num]->getGlobalPose().p;

	if (posVec.y < 0.3 && vel > 0.5f)
		res = true;
	else 
		res = false;

	return res;
}

vector<int> Physics::onGround()
{
	vector<int> onGround;

	for (int i = 3; i< numBlocks; i++)
	{
		PxVec3 temp = blockArray[i]->getGlobalPose().p;

		if (temp.y <= (2.0f*halfBoxShortSide))
			onGround.push_back(i);
	}

	return onGround;
}

bool Physics::turnBlock() // turn block to fit top layer
{
	bool result;

	vector<int> unusableBlocks = highestBlocks;
	int lookUp = unusableBlocks[2]; //immer 90° zu dem wie muss
	PxMat44 tempMat = blockArray[lookUp]->getGlobalPose();

	if ((tempMat.column0.x >= 0.9f && tempMat.column1.y >= 0.9f && tempMat.column2.z >= 0.9f)&&(tempMat.column0.x <= 1.1f && tempMat.column1.y <= 1.1f && tempMat.column2.z <= 1.1f))
		result = true;
	else
		result = false;
	

	return result;
} 

void Physics::adjustNumHighestBlocks()
{
	numBlocksHighest++;

	numBlocksHighest = numBlocksHighest % 3;
}

//int Physics::numBlocksOnHighestLayer()
//{
	//return numBlocksHighest;
//}


void Physics::getTopBlockLayout(blockLayout *layout)
{
	int tempNum = numBlocksHighest;

	PxVec3 temp1, temp2;

	if (tempNum == 0) 
	{
		*layout = FULL;
	}
	else if (tempNum == 1)
	{
		int testint = highestBlocks[4];
		temp1 = blockArray[highestBlocks[4]]->getGlobalPose().p;

		if ((temp1.x < 0.2f && temp1.z < 0.2f) && (temp1.x > -0.2f && temp1.z > -0.2f))
		{
			*layout = MIDDLE;
		}
		else if (((temp1.x < 1.1f && temp1.z < 0.3f) && (temp1.x > 0.45f && temp1.z > -0.3f) )|| 
					((temp1.x < 0.3f && temp1.z < 1.1f) && (temp1.x > -0.3f && temp1.z > 0.45f)))
		{
			*layout = RIGHT;
		}
		else
		{
			*layout = LEFT;
		}
	}
	else if (tempNum == 2)
	{
		temp1 = blockArray[highestBlocks[4]]->getGlobalPose().p;
		temp2 = blockArray[highestBlocks[3]]->getGlobalPose().p;

		if ((temp1.x < 0.2f && temp1.z < 0.2f) && (temp1.x > -0.2f && temp1.z > -0.2f))
		{
			*layout = MIDDLE;
		}
		else if (((temp1.x < 1.1f && temp1.z < 0.3f) && (temp1.x > 0.45f && temp1.z > -0.3f) )|| 
					((temp1.x < 0.3f && temp1.z < 1.1f) && (temp1.x > -0.3f && temp1.z > 0.45f)))
		{
			*layout = RIGHT;
		}
		else
		{
			*layout = LEFT;
		}

		//adjust with second block
		if ((temp2.x < 0.2f && temp2.z < 0.2f) && (temp2.x > -0.2f && temp2.z > -0.2f))
		{
			if(*layout == RIGHT)
				*layout = RIGHTMIDDLE;
			else 
				*layout = LEFTMIDDLE;
		}
		else if (((temp2.x < 1.1f && temp2.z < 0.3f) && (temp2.x > 0.45f && temp2.z > -0.3f) )|| 
					((temp2.x < 0.3f && temp2.z < 1.1f) && (temp2.x > -0.3f && temp2.z > 0.45f)))
		{
			if (*layout == MIDDLE)
				*layout = RIGHTMIDDLE;
			else 
				*layout = LEFTRIGHT;
		}
		else
		{
			if(*layout == MIDDLE)
				*layout = LEFTMIDDLE;
			else
				*layout = LEFTRIGHT;
		}
	}
}

void Physics::getBlockLayout(int *num, blockLayout *layout)
{
	PxVec3 picked = blockArray[*num]->getGlobalPose().p;
	int itemp1 = -1;
	int itemp2 = -1;

	for (int i = 0; i < numBlocks; i++)
	{
		if (i == *num)
			continue;

		PxVec3 foo = blockArray[i]->getGlobalPose().p;

		if ((foo.y < (picked.y + 0.1f)) && (foo.y > (picked.y - 0.1f)) && (itemp1 < 0))
			itemp1 = i;
		else if ((foo.y < (picked.y + 0.1f)) && (foo.y > (picked.y - 0.1f)) && (itemp1 >= 0))
			itemp2 = i;
	}

	if (itemp1 >= 0 && itemp2 >= 0)
	{
		*layout = FULL;
	}
	else if (itemp1 >= 0 && itemp2 < 0)
	{
		PxVec3 temp1 = blockArray[itemp1]->getGlobalPose().p;

		if ((temp1.x < 0.1f && temp1.z < 0.1f) && (temp1.x > -0.1f && temp1.z > -0.1f))
		{
			*layout = MIDDLE;
		}
		else if (((temp1.x < 1.1f && temp1.z < 0.3f) && (temp1.x > 0.45f && temp1.z > -0.3f) )|| 
					((temp1.x < 0.3f && temp1.z < 1.1f) && (temp1.x > -0.3f && temp1.z > 0.45f)))
		{
			*layout = RIGHT;
		}
		else
		{
			*layout = LEFT;
		}

		//second block
		if ((picked.x < 0.2f && picked.z < 0.2f) && (picked.x > -0.2f && picked.z > -0.2f))
		{
			if(*layout == RIGHT)
				*layout = LEFTRIGHT;//RIGHTMIDDLE; //if picked = middle -> don't take right/leftmiddle
			else 
				*layout = LEFTRIGHT;// LEFTMIDDLE;
		}
		else if (((picked.x < 1.1f && picked.z < 0.3f) && (picked.x > 0.45f && picked.z > -0.3f) )|| 
					((picked.x < 0.3f && picked.z < 1.1f) && (picked.x > -0.3f && picked.z > 0.45f)))
		{
			if (*layout == MIDDLE)
				*layout = RIGHTMIDDLE;
			else 
				*layout = LEFTRIGHT;
		}
		else
		{
			if(*layout == MIDDLE)
				*layout = LEFTMIDDLE;
			else
				*layout = LEFTRIGHT;
		}
	}
	else if (itemp1 < 0 && itemp2 < 0)
	{
		if ((picked.x < 0.4f && picked.z < 0.4f) && (picked.x > -0.4f && picked.z > -0.4f))
		{
			*layout = MIDDLE;
		}
		else if (((picked.x < 1.1f && picked.z < 0.3f) && (picked.x > 0.45f && picked.z > -0.3f) )|| 
					((picked.x < 0.3f && picked.z < 1.1f) && (picked.x > -0.3f && picked.z > 0.45f)))
		{
			*layout = RIGHT;
		}
		else
		{
			*layout = LEFT;
		}
	}
}



PxVec3 Physics::getBlockPosition(int *num)
{
	return blockArray[*num]->getGlobalPose().p;
}

vector<int> Physics::getUnusableBlocks()
{
	vector<int> unusable = highestBlocks;
	int top = numBlocksHighest;
	int stop = 2 - top ; //(numBlocksOnHighestLayer()%3);

	for (int i = 0; i < stop; i++)
	{
		unusable.erase(unusable.begin());
	}

	return unusable;
}

float Physics::getTopVelocity()
{
	PxRigidDynamic *temp = blockArray[highestBlocks[4]];
	PxVec3 vel = temp->getLinearVelocity();

	float velocity = vel.magnitude();

	return velocity;
}

void Physics::getHighestMiddleBlockPosition(float *position)
{
	/*int num = highestBlocks[0]; 

	for (int i =1; i < highestBlocks.size(); i++)
	{
		int lookup = highestBlocks[i];
		//int lastlookup = highestBlocks[i-1];

		PxVec3 temp = blockArray[lookup]->getGlobalPose().p;
		PxVec3 last = blockArray[num]->getGlobalPose().p;

		;
		if (((temp.x < 0.3f && temp.z < 0.3f) && (temp.x > -0.3f && temp.z > -0.3f)) && (temp.y > last.y))
		{
			num = lookup;
		}
	}//*/

	PxVec3 tempPos = blockArray[52]->getGlobalPose().p;

	position[0] = tempPos.x;
	position[1] = tempPos.y;
	position[2] = tempPos.z;
}

bool Physics::aiOutOfTower(const int *num)
{
	if (*num < 0) return false;

	PxVec3 tempPose = blockArray[*num]->getGlobalPose().p;
	
	if (tempPose.x >= 5.5f*halfBoxMiddleSide || tempPose.x <= -5.5f*halfBoxMiddleSide || tempPose.z >= 5.5f*halfBoxMiddleSide || tempPose.z <= -5.5f*halfBoxMiddleSide)
		return true;
	else
		return false;
}

/*void Physics::aiAdjustHighestBlocks(int *num)
{
	highestBlocks.push_back(*num);
	highestBlocks.erase(highestBlocks.begin());
}//*/

void Physics::aiPushBlock(int *num)
{
	PxRigidDynamic *temp = blockArray[*num];
	PxVec3 tempPos = temp->getGlobalPose().p;

	PxRigidBodyExt::addLocalForceAtPos(*temp, PxVec3(0.0f, 0.0f, 1.0f), tempPos, PxForceMode::eIMPULSE);
}

void Physics::aiPullBlock(int *num, bool way)
{
	int go = 1;

	PxRigidDynamic *temp = blockArray[*num];
	PxVec3 tempPos = temp->getGlobalPose().p;

	if(way)
		go = -go;

	PxRigidBodyExt::addLocalForceAtPos(*temp, PxVec3(0.0f, 0.0f, go*2.5f), tempPos, PxForceMode::eIMPULSE);
}

void Physics::aiDisableCollision(int *num)
{
	
	if (*num < 0)
		return;

	gScene->removeActor(*blockArray[*num]);
	blockArray[*num]->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
}

void Physics::aiMoveBlock(int *num, float *position, float *delta, bool turned)
{
	
	PxTransform tempPos;

	if (turned)	
	{
		tempPos = PxTransform(PxVec3(position[0], position[1], position[2]), PxQuat(PxHalfPi, PxVec3(0.0f, 1.0f, 0.0f)));

		tempPos.p.x = (cosf(PxHalfPi)* position[0]) - (sinf(PxHalfPi)* position[2]);
		tempPos.p.z = (sinf(PxHalfPi)* position[0]) + (cosf(PxHalfPi)* position[2]);
	}
	else
	{
		tempPos = PxTransform(PxVec3(position[0], position[1], position[2]));
	}

	tempPos.p.x += delta[0];
	tempPos.p.z += delta[2];
	
	
	
	blockArray[*num]->setGlobalPose(tempPos);
}

void Physics::aiDropBlock(int *num) //, PxTransform *position)
{
	
	if (*num < 0) return;

	gScene->addActor(*blockArray[*num]);
	blockArray[*num]->setLinearVelocity(PxVec3(0.0f, 0.0f, 0.0f));
	blockArray[*num]->setAngularVelocity(PxVec3(0.0f, 0.0f, 0.0f));
	blockArray[*num]->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false);

	//stop here if re-pickup
	if (highestBlocks[4]==*num)
		return;
	if (highestBlocks[3]==*num)
		return;
	
	adjustNumHighestBlocks();

	highestBlocks.push_back(*num);
	highestBlocks.erase(highestBlocks.begin());

	//currBlock = -1;
}