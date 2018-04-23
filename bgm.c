#include "bgm.h"

#include <prism/soundeffect.h>

#include "gamescreen.h"


static struct {
	int mCurrentLevel;
	int mSoundEffectID;
	int mNow;
	int mSinceLastFeed;
	int mMinimalLevel;

	MugenSounds* mSounds;
} gData;

static void playCurrentSoundbit() {	
	gData.mSoundEffectID = playMugenSound(gData.mSounds, 101 + gData.mCurrentLevel, 0);
	gData.mNow = 0;
}

static void loadBGMHandler(void* tData) {
	(void)tData;

	gData.mMinimalLevel = 0;
	gData.mCurrentLevel = 0;
	playCurrentSoundbit();
}

static void updateBGMHandler(void* tData) {
	(void)tData;

	gData.mSinceLastFeed++;
	if (gData.mSinceLastFeed > 240) {
		gData.mCurrentLevel = max(gData.mMinimalLevel, gData.mCurrentLevel - 1);
		gData.mSinceLastFeed = 0;
	}

	gData.mNow++;
	if (gData.mNow >= 115) {
		playCurrentSoundbit();
	}
}

ActorBlueprint BGMHandler = {
	.mLoad = loadBGMHandler,
	.mUpdate = updateBGMHandler,
};

void feedSoundIncrease()
{
	gData.mCurrentLevel = min(4, gData.mCurrentLevel + 1);
	gData.mSinceLastFeed = 0;

	stopSoundEffect(gData.mSoundEffectID);
	playCurrentSoundbit();
}


void setMinimumBackgroundLevel(int tLevel)
{
	gData.mMinimalLevel = tLevel;
	gData.mCurrentLevel = max(gData.mMinimalLevel, gData.mCurrentLevel);
	stopSoundEffect(gData.mSoundEffectID);
	playCurrentSoundbit();
}

void setBGMSounds(MugenSounds * tSounds)
{
	gData.mSounds = tSounds;
}
