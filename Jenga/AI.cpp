#include <algorithm>
#include "AI.h"

float timeAccu = 0.0f;
float step = 0.75f;

bool way = false;

int lastB = -1;
int pickedB = -1;

int wrongLayout = 0;

class Physics *physics;
class Sound *d8s;

bool comparePossFunc(possibilities i, possibilities j) {return (i.way > j.way);};


AI::AI(Physics *phys, Sound *ds)
{
	srand(time(NULL));
	physics = phys;
	d8s = ds;

	currentState = PICK;
}

AI::~AI()
{

}

void AI::play(float deltatime)
{
	
	switch(currentState)
	{
	case PICK: pickBlock(deltatime); break;

	case CHOOSE: chooseBlock(deltatime); break;

	case MOVE: moveBlock(deltatime); break;

	case PLACE: placeBlock(); break; 
	}


}

bool AI::giveUp()
{
	return (wrongLayout > 7);
}


void AI::pickBlock(float deltatime)
{
	timeAccu += deltatime;

	if (timeAccu < (step))
		return;

	timeAccu -= step;

	
	if (physics->getTopVelocity() > 0.25f)
		return;

	int number = (int) ((float(rand())/float(RAND_MAX))*numBlocks);

	vector<int> unusable = physics->getUnusableBlocks();

	for (int i = 0; i < unusable.size(); i++)
	{
		if (unusable[i] == number)
			return;
	}

	blockLayout lay;
	physics->getBlockLayout(&number, &lay);

	//stop if picking specific block would DEFINITELY topple tower
	if ((lay == LEFT)||(lay == RIGHT)||(lay == MIDDLE)||(lay == LEFTRIGHT))
	{
		wrongLayout++;
		return;//*/
	}

	before = physics->getBlockPosition(&number);
	physics->aiPushBlock(&number);
	d8s->playCollisionSF(number);

	if (lastB > 0)
	{
		now = physics->getBlockPosition(&lastB);
		
		float deltaX = now.x - lastBefore.x;
		float deltaY = now.y - lastBefore.y;
		float deltaZ = now.z - lastBefore.z;

		float dist = sqrtf((deltaX*deltaX)+(deltaY*deltaY)+(deltaZ*deltaZ));

		possibilities p;
		p.num = lastB;
		p.way = dist;

		possibleBlocks.push_back(p);

		if(dist > 0.1f )
		{
			pickedB = p.num;
			possibleBlocks.clear();
			lastB = -1;
			wrongLayout = 0;
			currentState = MOVE;
		}
	}

	lastBefore = before;
	lastB = number;

	if (possibleBlocks.size() >= 6 && currentState == PICK)
		currentState = CHOOSE;
}

void AI::chooseBlock(float deltatime)
{
	timeAccu += deltatime;

	if (timeAccu < (step))
		return;

	timeAccu -= step;

	
	//sort the vector by distance
	sort(possibleBlocks.begin(), possibleBlocks.end(), comparePossFunc);

	//choose one block
	float rndm =  (float(rand())/float(RAND_MAX));

	if (rndm >= 0.5f)
		pickedB = possibleBlocks[0].num;
	else if (rndm >= 0.25f)
		pickedB = possibleBlocks[1].num;
	else if (rndm >= 0.10f)
		pickedB = possibleBlocks[2].num;
	else
		pickedB = possibleBlocks[3].num;

	if (rndm >= 0.5f)
		way = true;
	else
		way = false;

	currentState = MOVE;
	lastB = -1;
	wrongLayout = 0;
	possibleBlocks.clear();

}

void AI::moveBlock(float deltatime)
{

	if (physics->getTopVelocity() > 0.25f)
		return;

	physics->aiPullBlock(&pickedB, way);

	timeAccu += deltatime;

	if (timeAccu < (1.0f/60.0f))
		return;

	timeAccu -= (1.0f/60.0f);//*/

	if (physics->aiOutOfTower(&pickedB))
	{
		physics->aiDisableCollision(&pickedB);
		currentState = PLACE;
	}
}

void AI::placeBlock()
{
	float rndm =  (float(rand())/float(RAND_MAX));

	vector<int> forHeight = physics->getUnusableBlocks();
	float height = physics->getTowerHeight()+2.1f*halfBoxShortSide;//(forHeight[2])+2.1f*halfBoxShortSide;
	blockLayout layout;
	physics->getTopBlockLayout(&layout);

	float dest[3];

	switch(layout)
	{
	case FULL:{ 
		if (rndm >0.5f)
		{
			dest[0] = 2*halfBoxMiddleSide+0.015f;	dest[1] = height;	dest[2] = 0.0f;
		}
		else 
		{
			dest[0] =-2*halfBoxMiddleSide-0.015f;	dest[1] = height;	dest[2] = 0.0f;
		}
		break;}
	case LEFT:{
		dest[0] = 2*halfBoxMiddleSide+0.015f;	dest[1] = height;	dest[2] = 0.0f;
		//dest[0] = 0.04f;	dest[1] = height;	dest[2] = 0.0f;
		break;}
	case RIGHT:{ 
		dest[0] =-2*halfBoxMiddleSide-0.015f;	dest[1] = height;	dest[2] = 0.0f;
		//dest[0] = -0.04f;	dest[1] = height;	dest[2] = 0.0f;
		break;}
	case MIDDLE:{ 
		if (rndm >0.5f)
		{
			dest[0] = 2*halfBoxMiddleSide+0.015f;	dest[1] = height;	dest[2] = 0.0f;
		}
		else 
		{
			dest[0] =-2*halfBoxMiddleSide-0.015f;	dest[1] = height;	dest[2] = 0.0f;
		}
		break;}
	case LEFTMIDDLE:{ 
		dest[0] = 2*halfBoxMiddleSide+0.015f;	dest[1] = height;	dest[2] = 0.0f;
		break;}
	case RIGHTMIDDLE:{ 
		dest[0] =-2*halfBoxMiddleSide-0.015f;	dest[1] = height;	dest[2] = 0.0f;
		break;} 
	case LEFTRIGHT:{ 
		dest[0] = 0.0f;	dest[1] = height;	dest[2] = 0.0f;
		break;}
	}

	if (dest[1] == NULL)
		return;

	//adjust moving tower
	float delta[3];
	physics->getHighestMiddleBlockPosition(delta);
	
	bool turned = physics->turnBlock();


	physics->aiMoveBlock(&pickedB, dest, delta, turned);
	
	/*if (physics->getTopVelocity() > 0.25f)
		return; //*/
	
	physics->aiDropBlock(&pickedB);
	d8s->playCollisionSF(pickedB);

	currentState = FINISHED;
	pickedB = -1;
}



int AI::getPickedBlock()
{
	return pickedB;
}

states AI::getCurrentState()
{
	return currentState;
}

void AI::setCurrentState(states s)
{
	currentState = s;

	if (!possibleBlocks.empty() && s == PICK)
	{
		wrongLayout = 0;
		possibleBlocks.clear();
	}
}