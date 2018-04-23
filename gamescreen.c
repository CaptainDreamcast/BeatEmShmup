#include "gamescreen.h"

#include "playerhandler.h"
#include "collision.h"
#include "ui.h"
#include "enemyhandler.h"
#include "bullethandler.h"
#include "boss.h"
#include "bosstext.h"
#include "bgm.h"

static struct {
	MugenSpriteFile mSprites;
	MugenAnimations mAnimations;
	BlitzTimelineAnimations mTimelineAnimations;

	MugenSounds mSounds;
	int mCurrentLevel;
} gData;

static void loadGameScreen() {
	char path[1024];

	sprintf(path, "game/%d.sff", getCurrentLevel());
	gData.mSprites = loadMugenSpriteFileWithoutPalette(path);
	sprintf(path, "game/%d.air", getCurrentLevel());
	gData.mAnimations = loadMugenAnimationFile(path);
	sprintf(path, "game/%d.taf", getCurrentLevel());
	gData.mTimelineAnimations = loadBlitzTimelineAnimations(path);
	gData.mSounds = loadMugenSoundFile("game/GAME.snd");

	loadGameCollisions();
	instantiateActor(PlayerHandler);
	instantiateActor(BulletHandler);
	instantiateActor(EnemyHandler);
	instantiateActor(Boss);
	instantiateActor(BossText);
	instantiateActor(UIHandler);

	setBGMSounds(&gData.mSounds);
	instantiateActor(BGMHandler);


	//activateCollisionHandlerDebugMode();
}

Screen GameScreen = {
	.mLoad = loadGameScreen,
};

MugenSpriteFile * getGameSprites()
{
	return &gData.mSprites;
}

MugenAnimations * getGameAnimations()
{
	return &gData.mAnimations;
}

BlitzTimelineAnimations * getGameTimelineAnimations()
{
	return &gData.mTimelineAnimations;
}

MugenSounds * getGameSounds()
{
	return &gData.mSounds;
}

void resetGame()
{
	gData.mCurrentLevel = 1;
}

int getCurrentLevel()
{
	return gData.mCurrentLevel;
}

void increaseCurrentLevel()
{
	gData.mCurrentLevel++;
}
