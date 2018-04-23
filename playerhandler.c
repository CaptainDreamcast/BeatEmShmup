#include "playerhandler.h"

#include "gamescreen.h"
#include "collision.h"
#include "ui.h"
#include "bgm.h"

static struct {
	MugenSpriteFile mSprites;
	MugenAnimations mAnimations;
	BlitzTimelineAnimations mTimelineAnimations;
	
	CollisionData mPlayerCollisionData;
	int mEntityID;

	CollisionData mLeftCollisionData;
	int mLeftFistEntityID;
	CollisionData mRightCollisionData;
	int mRightFistEntityID;

	Position mLeftPreviousPosition;
	Position mRightPreviousPosition;

	Position mLeftDelta;
	Position mRightDelta;


	int mIsPunchingLeft;
	int mIsPunchingRight;

	int mIsMovingTowardsCenter;

	int mLifeAmount;

	int mIsInvincible;
} gData;

static void resetScreenCB(void* tCaller) {
	(void)tCaller;
	setNewScreen(&GameScreen);
}

static void invincibleCB(void* tCaller) {
	(void)tCaller;
	setBlitzMugenAnimationTransparency(gData.mEntityID, 1);

	gData.mIsInvincible = 0;
}

static void playerHitCB(void* tCaller, void* tCollisionData) {
	(void)tCaller;
	(void)tCollisionData;

	if (gData.mIsInvincible) return;

	removeUILife();
	gData.mLifeAmount--;

	if (gData.mLifeAmount == 0) {
		setBlitzEntityScaleX(gData.mEntityID, 0);
		addFadeOut(30, resetScreenCB, NULL);
	}

	setBlitzMugenAnimationTransparency(gData.mEntityID, 0.4);
	playMugenSound(getGameSounds(), 2, 0);

	gData.mIsInvincible = 1;
	addTimerCB(120, invincibleCB, NULL);
}

static void punchHitCB(void* tCaller, void* tCollisionData) {
	(void)tCaller;
	feedSoundIncrease();
}

static void loadPlayer(void* tData) {
	gData.mSprites = loadMugenSpriteFileWithoutPalette("player/PLAYER.sff");
	gData.mAnimations = loadMugenAnimationFile("player/PLAYER.air");
	gData.mTimelineAnimations = loadBlitzTimelineAnimations("player/PLAYER.taf");

	gData.mEntityID = addBlitzEntity(makePosition((46 + 429) / 2, 380, 30));
	addBlitzMugenAnimationComponent(gData.mEntityID, &gData.mSprites, &gData.mAnimations, 10);
	addBlitzCollisionComponent(gData.mEntityID);
	int id = addBlitzCollisionCirc(gData.mEntityID, getPlayerCollisionListID(), makeCollisionCirc(makePosition(0, 0, 0), 2));
	gData.mPlayerCollisionData.mEntityID = gData.mEntityID; 
	gData.mPlayerCollisionData.mListID = getPlayerCollisionListID();
	setBlitzCollisionCollisionData(gData.mEntityID, id, &gData.mPlayerCollisionData);
	addBlitzCollisionCB(gData.mEntityID, id, playerHitCB, NULL);
	addBlitzTimelineComponent(gData.mEntityID, &gData.mTimelineAnimations);
	playBlitzTimelineAnimation(gData.mEntityID, 2);

	gData.mLeftFistEntityID = addBlitzEntity(makePosition(-100, -100, 35));
	addBlitzMugenAnimationComponent(gData.mLeftFistEntityID, &gData.mSprites, &gData.mAnimations, 20);
	int animationID = getBlitzMugenAnimationID(gData.mLeftFistEntityID);
	setMugenAnimationFaceDirection(animationID, 0);

	addBlitzCollisionComponent(gData.mLeftFistEntityID);
	id = addBlitzCollisionCirc(gData.mLeftFistEntityID, getPlayerShotCollisionListID(), makeCollisionCirc(makePosition(0, 0, 0), 11));
	gData.mLeftCollisionData.mEntityID = gData.mLeftFistEntityID;
	gData.mLeftCollisionData.mListID = getPlayerShotCollisionListID();
	setBlitzCollisionCollisionData(gData.mLeftFistEntityID, id, &gData.mLeftCollisionData);
	addBlitzCollisionCB(gData.mLeftFistEntityID, id, punchHitCB, NULL);
	addBlitzTimelineComponent(gData.mLeftFistEntityID, &gData.mTimelineAnimations);
	setBlitzEntityParent(gData.mLeftFistEntityID, gData.mEntityID);

	gData.mRightFistEntityID = addBlitzEntity(makePosition(-100, -100, 35));
	addBlitzMugenAnimationComponent(gData.mRightFistEntityID, &gData.mSprites, &gData.mAnimations, 20);
	addBlitzCollisionComponent(gData.mRightFistEntityID);
	id = addBlitzCollisionCirc(gData.mRightFistEntityID, getPlayerShotCollisionListID(), makeCollisionCirc(makePosition(0, 0, 0), 11));
	gData.mRightCollisionData.mEntityID = gData.mRightFistEntityID;
	gData.mRightCollisionData.mListID = getPlayerShotCollisionListID();
	setBlitzCollisionCollisionData(gData.mRightFistEntityID, id, &gData.mRightCollisionData);
	addBlitzCollisionCB(gData.mRightFistEntityID, id, punchHitCB, NULL);
	addBlitzTimelineComponent(gData.mRightFistEntityID, &gData.mTimelineAnimations);
	setBlitzEntityParent(gData.mRightFistEntityID, gData.mEntityID);

	gData.mIsPunchingLeft = 0;
	gData.mIsPunchingRight = 0;

	gData.mLifeAmount = 5;
	gData.mIsInvincible = 0;
}

static void updatePlayerMovement() {
	if (gData.mIsMovingTowardsCenter) return;

	Position* pos = getBlitzEntityPositionReference(gData.mEntityID);
	if (hasPressedLeft()) {
		pos->x -= 2;
	}
	if (hasPressedRight()) {
		pos->x += 2;
	}
	if (hasPressedUp()) {
		pos->y -= 2;
	}
	if (hasPressedDown()) {
		pos->y += 2;
	}
}

static void updatePlayerMovementToCenter() {
	if (!gData.mIsMovingTowardsCenter) return;

	Position* pos = getBlitzEntityPositionReference(gData.mEntityID);
	Position center = makePosition((46 + 429) / 2, 380, 30);

	Position delta = vecSub(center, *pos);
	*pos = vecAdd2D(*pos, vecScale(delta, 0.1));
}

static void punchRightOverCB(void* tCaller) {
	(void)tCaller;
	gData.mIsPunchingRight = 0;
}

static void punchLeftOverCB(void* tCaller) {
	(void)tCaller;
	gData.mIsPunchingLeft = 0;
}

static void updatePlayerPunching() {
	if (gData.mIsMovingTowardsCenter) return;

	if (hasPressedAFlank() && !gData.mIsPunchingLeft) {
		int id = playBlitzTimelineAnimation(gData.mLeftFistEntityID, 10);
		setBlitzTimelineAnimationCB(gData.mLeftFistEntityID, id, 1, punchLeftOverCB, NULL);
		playMugenSound(getGameSounds(), 1, 0);
		gData.mIsPunchingLeft = 1;
	}

	if (hasPressedBFlank() && !gData.mIsPunchingRight) {
		int id = playBlitzTimelineAnimation(gData.mRightFistEntityID, 20);
		setBlitzTimelineAnimationCB(gData.mRightFistEntityID, id, 1, punchRightOverCB, NULL);
		playMugenSound(getGameSounds(), 1, 0);
		gData.mIsPunchingRight = 1;
	}
}

static void constraintPlayerPosition() {
	Position* p = getBlitzEntityPositionReference(gData.mEntityID);
	*p = clampPositionToGeoRectangle(*p, makeGeoRectangle(46, 16, 429-46, 463-16));
}

static void updatePlayerFistPositions() {
	gData.mLeftDelta = vecSub(getBlitzEntityPosition(gData.mLeftFistEntityID), gData.mLeftPreviousPosition);
	gData.mRightDelta = vecSub(getBlitzEntityPosition(gData.mRightFistEntityID), gData.mRightPreviousPosition);

	gData.mLeftPreviousPosition = getBlitzEntityPosition(gData.mLeftFistEntityID);
	gData.mRightPreviousPosition = getBlitzEntityPosition(gData.mRightFistEntityID);
}

static void updatePlayer(void* tData) {
	(void)tData;

	updatePlayerMovement();
	updatePlayerMovementToCenter();
	constraintPlayerPosition();
	updatePlayerPunching();
	updatePlayerFistPositions();
}

ActorBlueprint PlayerHandler = {
	.mLoad = loadPlayer,
	.mUpdate = updatePlayer,
};

Position getPlayerFistDelta(int tEntityID)
{
	if (tEntityID == gData.mLeftFistEntityID) {
		return gData.mLeftDelta;
	}
	else {
		return gData.mRightDelta;
	}
}

int getPlayerLifeAmount()
{
	return gData.mLifeAmount;
}

void setPlayerMovingToCenter()
{
	gData.mIsMovingTowardsCenter = 1;
}

void setPlayerNotMovingToCenter()
{
	gData.mIsMovingTowardsCenter = 0;
}
