#pragma once

typedef struct {
	int mEntityID;
	int mListID;
} CollisionData;

void loadGameCollisions();

int getPlayerCollisionListID();
int getEnemyCollisionListID();
int getPlayerShotCollisionListID();
int getEnemyShotCollisionListID();
int getEnemyShotCollisionListID2();