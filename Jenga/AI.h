#pragma once

#include "GlobalSizes.h"
#include "Physics.h"
#include "Sound.h"

enum states {PICK, CHOOSE, MOVE, PLACE, FINISHED};

struct possibilities {int num; float way;};

class AI
{
private:

	states currentState;
	vector<possibilities> possibleBlocks;
	blockLayout layout;

	PxVec3 before, lastBefore, now;

	void pickBlock(float deltatime);
	void chooseBlock(float deltatime);
	void moveBlock(float deltatime);
	void placeBlock();

public: 

	AI(Physics *phys, Sound *ds);
	~AI();

	void play(float deltatime);
	bool giveUp();

	int getPickedBlock();
	states getCurrentState();
	void setCurrentState(states s);

};