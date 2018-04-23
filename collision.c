#include "collision.h"

#include <prism/blitz.h>

static struct {
	int mPlayerCollisionListID;
	int mEnemyCollisionListID;
	int mPlayerShotCollisionListID;
	int mEnemyShotCollisionListID;
	int mEnemyShotList2;
} gData;

void loadGameCollisions()
{
	gData.mPlayerCollisionListID = addCollisionListToHandler();
	gData.mEnemyCollisionListID = addCollisionListToHandler();
	gData.mPlayerShotCollisionListID = addCollisionListToHandler();
	gData.mEnemyShotCollisionListID = addCollisionListToHandler();
	gData.mEnemyShotList2 = addCollisionListToHandler();

	addCollisionHandlerCheck(gData.mPlayerCollisionListID, gData.mEnemyCollisionListID);
	addCollisionHandlerCheck(gData.mPlayerCollisionListID, gData.mEnemyShotCollisionListID);
	addCollisionHandlerCheck(gData.mPlayerShotCollisionListID, gData.mEnemyShotCollisionListID);
	addCollisionHandlerCheck(gData.mEnemyCollisionListID, gData.mPlayerShotCollisionListID);
	addCollisionHandlerCheck(gData.mEnemyCollisionListID, gData.mEnemyShotCollisionListID);
	addCollisionHandlerCheck(gData.mEnemyShotCollisionListID, gData.mEnemyShotList2);
}

int getPlayerCollisionListID()
{
	return gData.mPlayerCollisionListID;
}

int getEnemyCollisionListID()
{
	return gData.mEnemyCollisionListID;
}

int getPlayerShotCollisionListID()
{
	return gData.mPlayerShotCollisionListID;
}

int getEnemyShotCollisionListID()
{
	return gData.mEnemyShotCollisionListID;
}

int getEnemyShotCollisionListID2()
{
	return gData.mEnemyShotList2;
}