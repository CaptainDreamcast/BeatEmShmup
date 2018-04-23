#pragma once

#include <prism/blitz.h>

extern ActorBlueprint PlayerHandler;

Position getPlayerFistDelta(int tEntityID);
int getPlayerLifeAmount();

void setPlayerMovingToCenter();
void setPlayerNotMovingToCenter();