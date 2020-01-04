#pragma once
#include <Windows.h>

class Game;
class Pet
{
public:
	Pet(Game* p);

	// 宠物出征
	bool  PetOut(int no);
	// 宠物合体
	bool  PetFuck(DWORD* nums, DWORD length);
	// 复活所有没有血量宠物
	DWORD Revive();
public:
	// Game类
	Game* m_pGame;
};