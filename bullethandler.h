#pragma once

#include <prism/blitz.h>

extern ActorBlueprint BulletHandler;

void addBulletToBulletHandler(int tID, Position tPos);
void addManualBulletToBulletHandler(int tID, Position tPos);
void addManualFixedVelocityBulletToBulletHandler(int tID, Position tPos, Velocity tVel);
void removeAllBullets();

