#include "bullethandler.h"

#include "collision.h"
#include "playerhandler.h"
#include "gamescreen.h"
#include "boss.h"

typedef struct {
	int mAnimation;
	double mRadius;
	int mTImelineAnimation;
	int mID;
} Bullet;

typedef struct {
	int mEntityID;
	int mID;
	CollisionData mCollisionData;	
	CollisionData mCollisionData2;
} ActiveBullet;

static struct {
	MugenSpriteFile mSprites;
	MugenAnimations mAnimations;
	BlitzTimelineAnimations mTimelineAnimations;

	IntMap mBullets;
	IntMap mActiveBullets;
} gData;

static void loadSingleBulletFromScript(MugenDefScriptGroup* tGroup) {
	char word1[100], word2[100];

	sscanf(tGroup->mName, "%s %s", word1, word2);


	Bullet* e = allocMemory(sizeof(Bullet));
	e->mAnimation = getMugenDefIntegerOrDefaultAsGroup(tGroup, "anim", 0);
	e->mRadius = getMugenDefFloatOrDefaultAsGroup(tGroup, "radius", 1);
	e->mTImelineAnimation = getMugenDefIntegerOrDefaultAsGroup(tGroup, "timeline", -1);
	e->mID = atoi(word2);

	int_map_push_owned(&gData.mBullets, e->mID, e);
}

static void loadBulletsFromScript() {
	MugenDefScript script = loadMugenDefScript("game/bullets/BULLET.def");

	MugenDefScriptGroup* current = script.mFirstGroup;
	while (current) {
		loadSingleBulletFromScript(current);
		current = current->mNext;
	}

	unloadMugenDefScript(script);
}

static void loadBulletHandler(void* tData) {
	(void)tData;

	gData.mSprites = loadMugenSpriteFileWithoutPalette("game/bullets/BULLET.sff");
	gData.mAnimations = loadMugenAnimationFile("game/bullets/BULLET.air");
	gData.mTimelineAnimations = loadBlitzTimelineAnimations("game/bullets/BULLET.taf");

	gData.mBullets = new_int_map();
	gData.mActiveBullets = new_int_map();

	loadBulletsFromScript();
}

static void unloadBullet(ActiveBullet* e) {
	removeAllBlitzCollisions(e->mEntityID);
	removeBlitzEntity(e->mEntityID);

}

static int updateSingleBullet(void* tCaller, void* tData) {
	(void)tCaller;
	ActiveBullet* e = tData;
	Position* p = getBlitzEntityPositionReference(e->mEntityID);
	Velocity* v = getBlitzPhysicsVelocityReference(e->mEntityID);

	if (p->x < 46) {
		playMugenSound(getGameSounds(), 3, 0);
		p->x = 46;
		v->x *= -1;
	}

	if (p->x > 429) {
		playMugenSound(getGameSounds(), 3, 0);
		p->x = 429;
		v->x *= -1;
	}

	if (p->y > 500 || p->y < -50) {
		unloadBullet(e);
		return 1;
	}

	return 0;
}

static void updateBulletHandler(void* tData) {
	(void)tData;
	int_map_remove_predicate(&gData.mActiveBullets, updateSingleBullet, NULL);
}

ActorBlueprint BulletHandler = {
	.mLoad = loadBulletHandler,
	.mUpdate = updateBulletHandler,
};

static void handleBulletPunch(ActiveBullet* e, CollisionData* tCollisionData) {
	Position delta = getPlayerFistDelta(tCollisionData->mEntityID);
	Velocity vel = getBlitzPhysicsVelocity(e->mEntityID);
	double prevLength = vecLength(vel);
	double deltaLength = vecLength(delta);
	delta = vecNormalize(delta);
	double length = min(prevLength, deltaLength)*4;
	setBlitzPhysicsVelocity(e->mEntityID, vecScale(delta, length));

}

static void handleBulletReflection(ActiveBullet* e) {
	Velocity vel = getBlitzPhysicsVelocity(e->mEntityID);
	double prevLength = vecLength(vel);
	vel = vecNormalize(vel);
	vel = vecScale(vel, -1);
	double length = prevLength*2;
	setBlitzPhysicsVelocity(e->mEntityID, vecScale(vel, length));

}

static void removeBullet(ActiveBullet* e) {
	unloadBullet(e);
	int_map_remove(&gData.mActiveBullets, e->mID);
}

static void updateVelocity(int e1, int e2) {
	Velocity vel1 = getBlitzPhysicsVelocity(e1);
	Velocity vel2 = getBlitzPhysicsVelocity(e2);

	double len1 = vecLength(vel1);
	double len2 = vecLength(vel2);

	Position dir = vecSub(getBlitzEntityPosition(e2), getBlitzEntityPosition(e1));
	dir = vecNormalize(dir);
	vel1 = vecScale(dir, -len1);
	vel2 = vecScale(dir, len2);

	setBlitzPhysicsVelocity(e1, vel1);
	setBlitzPhysicsVelocity(e2, vel2);
}

static void bulletHitCB(void* tCaller, void* tCollisionData) {
	ActiveBullet* e = tCaller;
	CollisionData* collisionData = tCollisionData;

	if (collisionData->mListID == getPlayerShotCollisionListID()) {
		playMugenSound(getGameSounds(), 4, 0);
		handleBulletPunch(e, collisionData);
	}
	else if(collisionData->mListID == getEnemyShotCollisionListID2()){
		if (collisionData->mEntityID <= e->mEntityID) return;

		updateVelocity(e->mEntityID, collisionData->mEntityID);
	}
	else if (collisionData->mListID == getEnemyCollisionListID() && getCurrentLevel() == 3 && isBossActive()) {
		if (collisionData->mEntityID <= e->mEntityID) return;

		updateVelocity(e->mEntityID, collisionData->mEntityID);
	}
	else {
		removeBullet(e);
	}

}

void addBulletToBulletHandler(int tID, Position tPos) {
	Bullet* bullet = int_map_get(&gData.mBullets, tID);

	ActiveBullet* e = allocMemory(sizeof(ActiveBullet));
	e->mEntityID = addBlitzEntity(vecAdd(tPos, makePosition(0, 15+bullet->mRadius, 1)));
	addBlitzMugenAnimationComponent(e->mEntityID, &gData.mSprites, &gData.mAnimations, bullet->mAnimation);
	addBlitzCollisionComponent(e->mEntityID);
	int id = addBlitzCollisionCirc(e->mEntityID, getEnemyShotCollisionListID(), makeCollisionCirc(makePosition(0, 0, 0), bullet->mRadius));
	e->mCollisionData.mEntityID = e->mEntityID;
	e->mCollisionData.mListID = getEnemyShotCollisionListID();
	setBlitzCollisionCollisionData(e->mEntityID, id, &e->mCollisionData);
	addBlitzCollisionCB(e->mEntityID, id, bulletHitCB, e);

	id = addBlitzCollisionCirc(e->mEntityID, getEnemyShotCollisionListID2(), makeCollisionCirc(makePosition(0, 0, 0), bullet->mRadius));
	e->mCollisionData2.mEntityID = e->mEntityID;
	e->mCollisionData2.mListID = getEnemyShotCollisionListID2();
	setBlitzCollisionCollisionData(e->mEntityID, id, &e->mCollisionData2);

	addBlitzPhysicsComponent(e->mEntityID);
	addBlitzTimelineComponent(e->mEntityID, &gData.mTimelineAnimations);
	if (bullet->mTImelineAnimation != -1) {
		playBlitzTimelineAnimation(e->mEntityID, bullet->mTImelineAnimation);
	}

	e->mID = int_map_push_back_owned(&gData.mActiveBullets, e);
}

void addManualBulletToBulletHandler(int tID, Position tPos) {

	int bulletType = randfromInteger(1, 1);

	double radius = 8;


	ActiveBullet* e = allocMemory(sizeof(ActiveBullet));
	e->mEntityID = addBlitzEntity(vecAdd(tPos, makePosition(0, 15 + radius, 1)));
	addBlitzMugenAnimationComponent(e->mEntityID, &gData.mSprites, &gData.mAnimations, bulletType);
	addBlitzCollisionComponent(e->mEntityID);
	int id = addBlitzCollisionCirc(e->mEntityID, getEnemyShotCollisionListID(), makeCollisionCirc(makePosition(0, 0, 0), radius));

	e->mCollisionData.mEntityID = e->mEntityID;
	e->mCollisionData.mListID = getEnemyShotCollisionListID();
	setBlitzCollisionCollisionData(e->mEntityID, id, &e->mCollisionData);
	addBlitzCollisionCB(e->mEntityID, id, bulletHitCB, e);

	id = addBlitzCollisionCirc(e->mEntityID, getEnemyShotCollisionListID2(), makeCollisionCirc(makePosition(0, 0, 0), radius));
	e->mCollisionData2.mEntityID = e->mEntityID;
	e->mCollisionData2.mListID = getEnemyShotCollisionListID2();
	setBlitzCollisionCollisionData(e->mEntityID, id, &e->mCollisionData2);

	addBlitzPhysicsComponent(e->mEntityID);

	double angle = M_PI / 2 + randfrom(-M_PI / 4, M_PI / 4);
	Vector3D direction = getDirectionFromAngleZ(angle);
	double speed = randfrom(2, 4);
	addBlitzPhysicsImpulse(e->mEntityID, vecScale(direction, speed));

	e->mID = int_map_push_back_owned(&gData.mActiveBullets, e);
}

void addManualFixedVelocityBulletToBulletHandler(int tID, Position tPos, Velocity tVel)
{
	int bulletType = randfromInteger(1, 1);

	double radius = 8;


	ActiveBullet* e = allocMemory(sizeof(ActiveBullet));
	e->mEntityID = addBlitzEntity(vecAdd(tPos, makePosition(0, 15 + radius, 1)));
	addBlitzMugenAnimationComponent(e->mEntityID, &gData.mSprites, &gData.mAnimations, bulletType);
	addBlitzCollisionComponent(e->mEntityID);
	int id = addBlitzCollisionCirc(e->mEntityID, getEnemyShotCollisionListID(), makeCollisionCirc(makePosition(0, 0, 0), radius));

	e->mCollisionData.mEntityID = e->mEntityID;
	e->mCollisionData.mListID = getEnemyShotCollisionListID();
	setBlitzCollisionCollisionData(e->mEntityID, id, &e->mCollisionData);
	addBlitzCollisionCB(e->mEntityID, id, bulletHitCB, e);

	id = addBlitzCollisionCirc(e->mEntityID, getEnemyShotCollisionListID2(), makeCollisionCirc(makePosition(0, 0, 0), radius));
	e->mCollisionData2.mEntityID = e->mEntityID;
	e->mCollisionData2.mListID = getEnemyShotCollisionListID2();
	setBlitzCollisionCollisionData(e->mEntityID, id, &e->mCollisionData2);

	addBlitzPhysicsComponent(e->mEntityID);
	addBlitzPhysicsImpulse(e->mEntityID, tVel);

	e->mID = int_map_push_back_owned(&gData.mActiveBullets, e);
}

static int removeSingleActiveBullet(void* tCaller, void* tData) {
	(void)tCaller;
	ActiveBullet* e = tData;

	unloadBullet(e);
	return 1;
}

void removeAllBullets() {
	int_map_remove_predicate(&gData.mActiveBullets, removeSingleActiveBullet, NULL);
}