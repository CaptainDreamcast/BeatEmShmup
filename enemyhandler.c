#include "enemyhandler.h"

#include "gamescreen.h"
#include "bullethandler.h"
#include "collision.h"
#include "boss.h"

typedef struct {
	int mTime;
	int mAnimation;
	int mTimelineAnimation;
	int mIsFinalBoss;
	int mShot;

	int mWasSetActive;
} Enemy;

typedef struct {
	Enemy* mData;
	int mEntityID;
	int mID;
	Position mSource;
	Position mTarget;
	int mTime;

	CollisionData mCollisionData;
} ActiveEnemy;

static struct {
	BlitzTimelineAnimations mTimelineAnimations;

	Vector mEnemies;
	IntMap mActiveEnemies;

	int mTime;
} gData;

static void loadSingleEnemyFromScriptGroup(MugenDefScriptGroup* tGroup) {
	Enemy* e = allocMemory(sizeof(Enemy));

	e->mTime = getMugenDefIntegerOrDefaultAsGroup(tGroup, "time", 0);
	e->mAnimation = getMugenDefIntegerOrDefaultAsGroup(tGroup, "anim", 0);
	e->mTimelineAnimation = getMugenDefIntegerOrDefaultAsGroup(tGroup, "timeline", 0);
	e->mIsFinalBoss = getMugenDefIntegerOrDefaultAsGroup(tGroup, "finalboss", 0);
	e->mShot = getMugenDefIntegerOrDefaultAsGroup(tGroup, "shot", 1);
	e->mWasSetActive = 0;

	vector_push_back_owned(&gData.mEnemies, e);
}

static void loadEnemiesFromScript() {
	char path[1024];
	sprintf(path, "game/enemies/ENEMY%d.def", getCurrentLevel());

	MugenDefScript script = loadMugenDefScript(path);
	
	MugenDefScriptGroup* current = script.mFirstGroup;
	while (current) {
		loadSingleEnemyFromScriptGroup(current);
		current = current->mNext;
	}

	unloadMugenDefScript(script);
}

static void loadEnemyHandler(void* tData) {
	(void)tData;
	char path[1024];
	sprintf(path, "game/enemies/ENEMY%d.taf", getCurrentLevel());

	gData.mTimelineAnimations = loadBlitzTimelineAnimations(path);

	gData.mEnemies = new_vector();
	gData.mActiveEnemies = new_int_map();

	loadEnemiesFromScript();

	gData.mTime = 0;
}

static void fireEnemyBullet(void* tCaller) {
	ActiveEnemy* e = tCaller;
	Position pos = getBlitzEntityPosition(e->mEntityID);
	addManualBulletToBulletHandler(e->mData->mShot, pos);
}

static void unloadEnemy(ActiveEnemy* e) {
	removeBlitzEntity(e->mEntityID);
}

static void removeEnemy(ActiveEnemy* e) {
	unloadEnemy(e);
	int_map_remove(&gData.mActiveEnemies, e->mID);
}

static void removeEnemyCB(void* tCaller) {
	ActiveEnemy* e = tCaller;
	removeEnemy(e);
}

static void enemyMoveCB(void* tCaller) {
	ActiveEnemy* e = tCaller;
	Position* p = getBlitzEntityPositionReference(e->mEntityID);
	
	e->mSource = *p;
	e->mTarget = makePosition(randfrom(45, 450), -100, p->z);
}


static void enemyHitCB(void* tCaller, void* tCollisionData) {
	(void)tCollisionData;

	playMugenSound(getGameSounds(), 2, 0);

	ActiveEnemy* e = tCaller;
	removeEnemy(e);
}

static void loadFinalBossEnemy() {
	setBossActive();
}



static void updateSingleEnemy(void* tCaller, void* tData) {
	(void)tCaller;

	Enemy* e = tData;
	if (e->mWasSetActive) return;
	if (gData.mTime < e->mTime) return;

	if (e->mIsFinalBoss) {
		loadFinalBossEnemy();
		e->mWasSetActive = 1;
		return;
	}

	ActiveEnemy* activeEnemy = allocMemory(sizeof(ActiveEnemy));
	activeEnemy->mEntityID = addBlitzEntity(makePosition(randfrom(45, 450), -100, 20 + randfrom(-0.1, 0.1)));
	addBlitzMugenAnimationComponent(activeEnemy->mEntityID, getGameSprites(), getGameAnimations(), randfromInteger(100, 108));
	addBlitzCollisionComponent(activeEnemy->mEntityID);
	int id = addBlitzCollisionAttackMugen(activeEnemy->mEntityID, getEnemyCollisionListID());
	addBlitzCollisionCB(activeEnemy->mEntityID, id, enemyHitCB, activeEnemy);
	activeEnemy->mCollisionData.mEntityID = activeEnemy->mEntityID;
	activeEnemy->mCollisionData.mListID = getEnemyCollisionListID();
	setBlitzCollisionCollisionData(activeEnemy->mEntityID, id, &activeEnemy->mCollisionData);
	addBlitzTimelineComponent(activeEnemy->mEntityID, &gData.mTimelineAnimations);
	id = playBlitzTimelineAnimation(activeEnemy->mEntityID, e->mTimelineAnimation);
	setBlitzTimelineAnimationCB(activeEnemy->mEntityID, id, 1, fireEnemyBullet, activeEnemy);
	setBlitzTimelineAnimationCB(activeEnemy->mEntityID, id, 2, removeEnemyCB, activeEnemy);
	setBlitzTimelineAnimationCB(activeEnemy->mEntityID, id, 3, enemyMoveCB, activeEnemy);
	activeEnemy->mData = e;

	activeEnemy->mID = int_map_push_back_owned(&gData.mActiveEnemies, activeEnemy);

	activeEnemy->mTime = 0;
	activeEnemy->mSource = getBlitzEntityPosition(activeEnemy->mEntityID);
	activeEnemy->mTarget = makePosition(randfrom(45, 450), 150, 20);

	e->mWasSetActive = 1;
}

static int updateActiveEnemy(void* tCaller, void* tData) {
	(void)tCaller;
	ActiveEnemy* e = tData;
	Position* p = getBlitzEntityPositionReference(e->mEntityID);
	if (e->mTime <= 120) {
		double t = e->mTime / 120.0;
		*p = interpolatePositionLinear(e->mSource, e->mTarget, t);
	}
	else if (e->mTime > 400) {
		double t = (e->mTime - 400) / 100.0;
		*p = interpolatePositionLinear(e->mSource, e->mTarget, t);
		if (e->mTime == 500) {
			unloadEnemy(e);
			return 1;
		}
	}
	
	e->mTime++;
	return 0;
}

static void updateEnemyHandler(void* tData) {
	vector_map(&gData.mEnemies, updateSingleEnemy, NULL);
	int_map_remove_predicate(&gData.mActiveEnemies, updateActiveEnemy, NULL);

	gData.mTime++;
}

ActorBlueprint EnemyHandler = {
	.mLoad = loadEnemyHandler,
	.mUpdate = updateEnemyHandler,
};

