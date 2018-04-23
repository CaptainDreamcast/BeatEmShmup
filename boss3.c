#include "boss1.h"

#include "bullethandler.h"

static struct {
	int mNow;
	int mNow2;
	int mEntityID;

	Position mTarget;
} gData;

void initBoss3(int tEntityID)
{
	gData.mEntityID = tEntityID;
	gData.mNow = 0;
	gData.mNow2 = 0;

	gData.mTarget = getBlitzEntityPosition(gData.mEntityID);
}

static void updateNewAction() {
	double rand = randfrom(0, 100);

	if (rand < 2) {
		gData.mTarget = makePosition(randfrom(60, 380), randfrom(50, 150), gData.mTarget.z);
	}	if (gData.mNow2 >= 10 && rand < 4) {
		addManualBulletToBulletHandler(1000, vecAdd(getBlitzEntityPosition(gData.mEntityID), makePosition(0, 25, 0)));
		gData.mNow2 = 0;
	}


	if (gData.mNow >= 10) {
		addManualFixedVelocityBulletToBulletHandler(1000, makePosition(randfrom(60, 380), getBlitzEntityPositionY(gData.mEntityID) + 25, gData.mTarget.z), makePosition(0, 2, 0));
		gData.mNow = 0;
	}
}

static void updateMovement() {
	Position* p = getBlitzEntityPositionReference(gData.mEntityID);
	Position delta = vecSub(gData.mTarget, *p);

	if (vecLength(delta) > 2) {
		delta = vecNormalize(delta);
		delta = vecScale(delta, 2);
	}

	*p = vecAdd2D(*p, delta);
}

void updateBoss3()
{
	updateNewAction();
	updateMovement();
	gData.mNow++;
	gData.mNow2++;
}
