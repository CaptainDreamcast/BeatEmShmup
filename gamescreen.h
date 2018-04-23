#pragma once

#include <prism/blitz.h>

extern Screen GameScreen;

MugenSpriteFile* getGameSprites();
MugenAnimations* getGameAnimations();
BlitzTimelineAnimations* getGameTimelineAnimations();
MugenSounds* getGameSounds();

void resetGame();
int getCurrentLevel();
void increaseCurrentLevel();