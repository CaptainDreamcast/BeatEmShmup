#include "boss.h"

#include "gamescreen.h"
#include "collision.h"
#include "bosstext.h"
#include "playerhandler.h"
#include "bgm.h"
#include "bullethandler.h"

#include "boss1.h"
#include "boss2.h"
#include "boss3.h"
#include "storyscreen.h"

typedef enum {
	BOSS_ACTION_TYPE_SHOT,
	BOSS_ACTION_TYPE_ANIMATION,

} BossActionType;

typedef struct {
	int mTime;
	BossActionType mType;
	int mValue;
} BossAction;

static struct {
	BlitzTimelineAnimations mTimelineAnimations;

	int mPatternTime;
	char* mName;
	int mLifeMax;

	int mEntityID;
	CollisionData colData;

	int mNameTextID;
	int mHealthBarEntity;
	int mLife;

	int mIsInvincible;

	int mIsFighting;

	Vector mBossActions;
} gData;

static void loadHeaderGroup(MugenDefScriptGroup* tGroup) {
	gData.mPatternTime = getMugenDefIntegerOrDefaultAsGroup(tGroup, "time", 60);
	gData.mName = getAllocatedMugenDefStringVariableAsGroup(tGroup, "name");
	gData.mLifeMax = getMugenDefIntegerOrDefaultAsGroup(tGroup, "life", 100);
}

static void loadActionGroup(MugenDefScriptGroup* tGroup) {
	BossAction* e = allocMemory(sizeof(BossAction));
	e->mTime = getMugenDefIntegerOrDefaultAsGroup(tGroup, "time", 0);
	char* type = getAllocatedMugenDefStringVariableAsGroup(tGroup, "type");
	if (!strcmp("shot", type)) {
		e->mType = BOSS_ACTION_TYPE_SHOT;
	}
	else {
		e->mType = BOSS_ACTION_TYPE_ANIMATION;
	}
	freeMemory(type);

	e->mValue = getMugenDefIntegerOrDefaultAsGroup(tGroup, "value", 0);

	vector_push_back_owned(&gData.mBossActions, e);
}

static void loadBossActionsFromScript() {
	char path[1024];
	sprintf(path, "game/boss/BOSS%d.def", getCurrentLevel());

	MugenDefScript script = loadMugenDefScript(path);
	MugenDefScriptGroup* current = script.mFirstGroup;
	loadHeaderGroup(current);
	current = current->mNext;

	while (current) {
		loadActionGroup(current);
		current = current->mNext;
	}


	unloadMugenDefScript(script);
}

static void loadBoss(void* tData) {
	(void)tData;
	char path[1024];
	sprintf(path, "game/boss/BOSS%d.taf", getCurrentLevel());


	gData.mTimelineAnimations = loadBlitzTimelineAnimations(path);

	gData.mBossActions = new_vector();

	loadBossActionsFromScript();

	gData.mIsInvincible = 0;
	gData.mIsFighting = 0;
}

static void updateBoss(void* tData) {
	if (!gData.mIsFighting) return;

	if (getCurrentLevel() == 1) {
		updateBoss1();
	}
	else if (getCurrentLevel() == 2) {
		updateBoss2();
	}
	else {
		updateBoss3();
	}
}

ActorBlueprint Boss = {
	.mLoad = loadBoss,
	.mUpdate = updateBoss,
};

static void updateHealthBar() {
	double t = gData.mLife / (double)gData.mLifeMax;
	double width = t * 362;
	setMugenAnimationRectangleWidth(getBlitzMugenAnimationID(gData.mHealthBarEntity), (int)width);
}

static void invincibleCB(void* tCaller) {
	(void)tCaller;
	gData.mIsInvincible = 0;
}

static void gotoNextLevelCB(void* tCaller) {
	(void)tCaller;
	if (getCurrentLevel() == 3) {
		setCurrentStoryDefinitionFile("story/OUTRO.def");
		setNewScreen(&StoryScreen);
		return;
	} 

	increaseCurrentLevel();
	setNewScreen(&GameScreen);
}

static void bossHitCB(void* tCaller, void* tData) {
	(void)tCaller;
	(void)tData;

	if (gData.mIsInvincible) return;

	CollisionData* colData = tData;
	if (colData->mListID == getPlayerCollisionListID()) return;

	gData.mLife = max(0, gData.mLife - 1);
	updateHealthBar();

	playMugenSound(getGameSounds(), 2, 0);

	if (!gData.mLife) {
		addFadeOut(30, gotoNextLevelCB, NULL);
		gData.mIsFighting = 0;
	}

	addTimerCB(60, invincibleCB, NULL);
	gData.mIsInvincible = 1;
}

void setBossActive()
{
	gData.mEntityID = addBlitzEntity(makePosition(200, 100, 20));
	addBlitzMugenAnimationComponent(gData.mEntityID, getGameSprites(), getGameAnimations(), 1000);
	addBlitzCollisionComponent(gData.mEntityID);
	int id = addBlitzCollisionPassiveMugen(gData.mEntityID, getEnemyCollisionListID());
	addBlitzCollisionCB(gData.mEntityID, id, bossHitCB, NULL);
	gData.colData.mEntityID = gData.mEntityID;
	gData.colData.mListID = getEnemyCollisionListID();
	setBlitzCollisionCollisionData(gData.mEntityID, id, &gData.colData);
	addBlitzTimelineComponent(gData.mEntityID, &gData.mTimelineAnimations);
	setPlayerMovingToCenter();
	removeAllBullets();
	setBossTextActive();

}

static void loadHealthBar() {
	gData.mNameTextID = addMugenText(gData.mName, makePosition(53, 30, 71), 3);

	gData.mHealthBarEntity = addBlitzEntity(makePosition(53, 38, 70));
	addBlitzMugenAnimationComponent(gData.mHealthBarEntity, getGameSprites(), getGameAnimations(), 53);
	gData.mLife = gData.mLifeMax;
}

void startActualBossFight()
{
	loadHealthBar();
	setPlayerNotMovingToCenter();
	setMinimumBackgroundLevel(1);
	gData.mIsFighting = 1;

	if (getCurrentLevel() == 1) {
		initBoss1(gData.mEntityID);
	} else if (getCurrentLevel() == 2) {
		initBoss2(gData.mEntityID);
	}
	else {
		initBoss3(gData.mEntityID);
	}
}

int isBossActive()
{
	return gData.mIsFighting;
}
