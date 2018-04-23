#include "titlescreen.h"

#include "gamescreen.h"
#include "bgm.h"
#include "storyscreen.h"

static struct {
	MugenSpriteFile mSprites;
	MugenAnimations mAnimations;
	MugenSounds mSounds;

	int mPlayerEntity;

	int mIsShowingMiniPlayer;
	int mMiniPlayerID;
	int mIsAcceptingInput;

	int mTextID;
	
	TextureData mWhiteTexture;
} gData;


static void fadeToBlackCB(void* tCaller);

static void loadTitleScreen() {
	gData.mSprites = loadMugenSpriteFileWithoutPalette("title/TITLE.sff");
	gData.mAnimations = loadMugenAnimationFile("title/TITLE.air");
	gData.mSounds = loadMugenSoundFile("game/GAME.snd");

	setBGMSounds(&gData.mSounds);
	instantiateActor(BGMHandler);


	gData.mWhiteTexture = createWhiteTexture();

	int id = addBlitzEntity(makePosition(0, 0, 1));
	addBlitzMugenAnimationComponentStatic(id, &gData.mSprites, 1, 0);

	gData.mPlayerEntity = addBlitzEntity(makePosition(280, 450, 3));
	addBlitzMugenAnimationComponent(gData.mPlayerEntity, &gData.mSprites, &gData.mAnimations, 1);

	id = addBlitzEntity(makePosition(360, 450, 2));
	addBlitzMugenAnimationComponent(id, &gData.mSprites, &gData.mAnimations, 4);

	gData.mIsShowingMiniPlayer = 0;
	gData.mIsAcceptingInput = 1;
	resetGame();
	addFadeIn(60, NULL, NULL);
}

static void gotoGameCB(void* tCaller) {
	setCurrentStoryDefinitionFile("story/INTRO.def");
	setNewScreen(&StoryScreen);
}

static void fadeToGameCB(void* tCaller) {
	addFadeOut(60, gotoGameCB, NULL);
}

static void showBeatCB(void* tCaller) {
	int id = addBlitzEntity(makePosition(320, 200, 11));
	addBlitzMugenAnimationComponent(id, &gData.mSprites, &gData.mAnimations, 10);

	gData.mMiniPlayerID = addBlitzEntity(makePosition(320, 500, 12));
	addBlitzMugenAnimationComponent(gData.mMiniPlayerID, &gData.mSprites, &gData.mAnimations, 5);
	addBlitzPhysicsComponent(gData.mMiniPlayerID);
	addBlitzPhysicsVelocityY(gData.mMiniPlayerID, -4);
	gData.mIsShowingMiniPlayer = 1;

	gData.mTextID = addMugenText("SHMUP", makePosition(320, 250, 11), 3);
	setMugenTextAlignment(gData.mTextID, MUGEN_TEXT_ALIGNMENT_CENTER);
	setMugenTextRectangle(gData.mTextID, makeGeoRectangle(0, 500, INF, INF));
}

static void fadeToBlackCB(void* tCaller) {
	int id = playOneFrameAnimationLoop(makePosition(0, 0, 10), &gData.mWhiteTexture);
	setAnimationColor(id, 0, 0, 0);
	setAnimationSize(id, makePosition(640, 480, 0), makePosition(0, 0, 0));
	showBeatCB(NULL);
}

static void updateMiniPlayer() {
	if (!gData.mIsShowingMiniPlayer) return;

	setMugenTextRectangle(gData.mTextID, makeGeoRectangle(0, getBlitzEntityPositionY(gData.mMiniPlayerID), INF, INF));

	if (getBlitzEntityPositionY(gData.mMiniPlayerID) < 0) {
		addTimerCB(120, fadeToGameCB, NULL);
		gData.mIsShowingMiniPlayer = 0;
	}
}

static void updateTInput() {
	if (!gData.mIsAcceptingInput) return;

	if (hasPressedAFlank() || hasPressedStartFlank()) {
		changeBlitzMugenAnimation(gData.mPlayerEntity, 3);
		addTimerCB(10, fadeToBlackCB, NULL);
		feedSoundIncrease();		
		playMugenSound(&gData.mSounds, 1, 0);
		gData.mIsAcceptingInput = 0;
	}
}

static void updateTitleScreen() {
	updateTInput();
	updateMiniPlayer();
}

Screen TitleScreen = {
	.mLoad = loadTitleScreen,
	.mUpdate = updateTitleScreen,
};