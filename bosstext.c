#include "bosstext.h"

#include "gamescreen.h"
#include "boss.h"

typedef struct {
	char* mSpeaker;
	char* mText;
	int mHasQuestion;
	char* mYes;
	char* mNo;
	int mYesNext;
	int mNoNext;

} TextStep;

static struct {
	int mIsActive;

	TextStep* mCurrentStep;

	int mTextBGEntity;
	int mIsShowingQuestion;
	int mSpeakerTextID;
	int mTextID;
	
	int mYesBGEntity;
	int mYesID;
	int mNoBGEntity;
	int mNoID;

	int mSelectorEntity;

	int mSelectedAnswer;

	Vector mTexts;
} gData;

static char* handleSingleOptionalText(MugenDefScriptGroup* tGroup, char* tVariableName) {
	if (!isMugenDefStringVariableAsGroup(tGroup, tVariableName)) {
		return NULL;
	}

	return getAllocatedMugenDefStringVariableAsGroup(tGroup, tVariableName);
}

static void loadSingleText(MugenDefScriptGroup* tGroup, int i) {
	TextStep* e = allocMemory(sizeof(TextStep));
	e->mSpeaker = handleSingleOptionalText(tGroup, "speaker");
	e->mText = handleSingleOptionalText(tGroup, "text");
	e->mYes = handleSingleOptionalText(tGroup, "yes");
	e->mNo = handleSingleOptionalText(tGroup, "no");
	e->mHasQuestion = isMugenDefStringVariableAsGroup(tGroup, "yes");
	e->mYesNext = getMugenDefIntegerOrDefaultAsGroup(tGroup, "yesnext", i + 1);
	e->mNoNext = getMugenDefIntegerOrDefaultAsGroup(tGroup, "nonext", i + 1);

	vector_push_back_owned(&gData.mTexts, e);
}

static void loadTexts() {
	char path[1024];
	sprintf(path, "game/boss/BOSSTEXT%d.def", getCurrentLevel());

	MugenDefScript script = loadMugenDefScript(path);
	MugenDefScriptGroup* current = script.mFirstGroup;
	int i = 0;
	while (current) {
		loadSingleText(current, i);
		i++;
		current = current->mNext;
	}

	unloadMugenDefScript(script);
}


static void loadBossText(void* tData) {
	(void)tData;
	gData.mTexts = new_vector();
	loadTexts();
	gData.mIsActive = 0;

}

static void showQuestionText() {
	gData.mYesBGEntity = addBlitzEntity(makePosition((46 + 429) / 2, 250, 70));
	addBlitzMugenAnimationComponent(gData.mYesBGEntity, getGameSprites(), getGameAnimations(), 51);

	gData.mYesID = addMugenText(gData.mCurrentStep->mYes, makePosition(130, 254, 71), 3);

	gData.mNoBGEntity = addBlitzEntity(makePosition((46 + 429) / 2, 280, 70));
	addBlitzMugenAnimationComponent(gData.mNoBGEntity, getGameSprites(), getGameAnimations(), 51);

	gData.mNoID = addMugenText(gData.mCurrentStep->mNo, makePosition(130, 284, 71), 3);

	gData.mSelectedAnswer = 0;
	gData.mSelectorEntity = addBlitzEntity(makePosition(90, 250, 72));
	addBlitzMugenAnimationComponent(gData.mSelectorEntity, getGameSprites(), getGameAnimations(), 52);
	addBlitzTimelineComponent(gData.mSelectorEntity, getGameTimelineAnimations());
	playBlitzTimelineAnimation(gData.mSelectorEntity, 52);

	gData.mIsShowingQuestion = 1;
}

static void changeSelection() {
	gData.mSelectedAnswer ^= 1;

	if (gData.mSelectedAnswer) {
		setBlitzEntityPosition(gData.mSelectorEntity, makePosition(90, 280, 72));
	}
	else {
		setBlitzEntityPosition(gData.mSelectorEntity, makePosition(90, 250, 72));
	}

}

static void unloadActiveStep();
static void loadActiveStep();

static void gotoNextTextStep(int tValue) {
	unloadActiveStep();
	if (tValue == -1) {
		startActualBossFight();
		gData.mIsActive = 0;
		return;
	}

	gData.mCurrentStep = vector_get(&gData.mTexts, tValue);
	loadActiveStep();
}

static void updateBInput() {
	if (hasPressedAFlank()) {
		if (!isMugenTextBuiltUp(gData.mTextID)) {
			setMugenTextBuiltUp(gData.mTextID);
		}
		else {
			if (gData.mIsShowingQuestion) {
				if (gData.mSelectedAnswer) gotoNextTextStep(gData.mCurrentStep->mNoNext);
				else gotoNextTextStep(gData.mCurrentStep->mYesNext);
			}
			else gotoNextTextStep(gData.mCurrentStep->mYesNext);
		}
		playMugenSound(getGameSounds(), 3, 0);
	}
}

static void updateAnswerSelection() {
	if (!gData.mIsShowingQuestion) return;

	if (hasPressedUpFlank() || hasPressedDownFlank()) {
		changeSelection();
		playMugenSound(getGameSounds(), 1, 0);
	}
}

static void updateShowingQuestions() {
	if (isMugenTextBuiltUp(gData.mTextID) && !gData.mIsShowingQuestion && gData.mCurrentStep->mHasQuestion) {
		showQuestionText();
	}
}

static void updateTextStep() {
	if (!gData.mIsActive) return;

	
	updateAnswerSelection();
	updateShowingQuestions();
	updateBInput();
}

static void updateBossText(void* tData) {
	(void)tData;
	updateTextStep();
}

ActorBlueprint BossText = {
	.mLoad = loadBossText,
	.mUpdate = updateBossText,
};


static void loadActiveStep() {
	TextStep* e = gData.mCurrentStep;

	gData.mTextBGEntity = addBlitzEntity(makePosition((46 + 429) / 2, 200, 70));
	addBlitzMugenAnimationComponent(gData.mTextBGEntity, getGameSprites(), getGameAnimations(), 50);

	gData.mSpeakerTextID = addMugenText(e->mSpeaker, makePosition(120, 163, 71), 3);
	gData.mTextID = addMugenText(e->mText, makePosition(100, 185, 71), 3);
	setMugenTextBuildup(gData.mTextID, 1);
	setMugenTextTextBoxWidth(gData.mTextID, 280);

	gData.mIsShowingQuestion = 0;
}

static void unloadActiveStep() {
	removeBlitzEntity(gData.mTextBGEntity);
	removeMugenText(gData.mSpeakerTextID);
	removeMugenText(gData.mTextID);

	if (gData.mIsShowingQuestion) {
		removeBlitzEntity(gData.mYesBGEntity);
		removeBlitzEntity(gData.mNoBGEntity);
		removeBlitzEntity(gData.mSelectorEntity);

		removeMugenText(gData.mYesID);
		removeMugenText(gData.mNoID);
	}
}

void setBossTextActive()
{
	gData.mCurrentStep = vector_get(&gData.mTexts, 0);
	loadActiveStep();

	gData.mIsActive = 1;
}
