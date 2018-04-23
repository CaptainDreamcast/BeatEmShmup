#include "ui.h"

#include "gamescreen.h"
#include "playerhandler.h"

static struct {
	MugenSpriteFileSprite* mBGSprite;
	int mBGEntities[2];

	int mLifeAmount;
	int mLifeEntities[10];

	TextureData mWhiteTexture;
} gData;

static void addSingleLife(int i) {
	Position pos = makePosition(456 + 30 * i, 48, 81);
	gData.mLifeEntities[i] = addBlitzEntity(pos);
	addBlitzMugenAnimationComponent(gData.mLifeEntities[i], getGameSprites(), getGameAnimations(), 11);
}

static void removeSingleLife(int i) {
	removeBlitzEntity(gData.mLifeEntities[i]);
}

static void loadLives() {
	gData.mLifeAmount = getPlayerLifeAmount();
	int i;
	for (i = 0; i < gData.mLifeAmount; i++) {
		addSingleLife(i);
	}
}

static void loadUIHandler(void* tData) {
	(void)tData;
	int id = addBlitzEntity(makePosition(0, 0, 80));
	addBlitzMugenAnimationComponentStatic(id, getGameSprites(), 10, 0);

	gData.mBGSprite = getMugenSpriteFileTextureReference(getGameSprites(), 20, 0);

	Position pos = makePosition(0, 0, 1);
	int i;
	for (i = 0; i < 2; i++) {
		gData.mBGEntities[i] = addBlitzEntity(pos);
		addBlitzMugenAnimationComponentStatic(gData.mBGEntities[i], getGameSprites(), 20, 0);

		pos = vecSub2D(pos, makePosition(0, gData.mBGSprite->mOriginalTextureSize.y, 0));
	}

	loadLives();

	gData.mWhiteTexture = createWhiteTexture();
	id = playOneFrameAnimationLoop(makePosition(0,0,2), &gData.mWhiteTexture);
	setAnimationSize(id, makePosition(640, 480, 1), makePosition(0, 0, 0));
	setAnimationTransparency(id, 0.4);
	setAnimationColorType(id, COLOR_BLACK);
}

static void updateSingleBackgroundEntity(int tEntityID) {
	Position* pos = getBlitzEntityPositionReference(tEntityID);
	pos->y += 4;

	if (pos->y > 480) {
		pos->y -= 2*gData.mBGSprite->mOriginalTextureSize.y;
	}
}

static void updateUIHandler(void* tData) {
	(void)tData;
	int i;
	for (i = 0; i < 2; i++) {
		updateSingleBackgroundEntity(gData.mBGEntities[i]);
	}
}

ActorBlueprint UIHandler = {
	.mLoad = loadUIHandler,
	.mUpdate = updateUIHandler,
};

void removeUILife()
{
	if (!gData.mLifeAmount) return;

	removeSingleLife(gData.mLifeAmount - 1);
	gData.mLifeAmount--;
}
