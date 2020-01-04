#include "Game.h"
#include "Pet.h"
#include "Item.h"
#include "PrintScreen.h"
#include "GameProc.h"
#include <My/Common/C.h>

Pet::Pet(Game* p)
{
	m_pGame = p;
}

// 宠物出征
bool Pet::PetOut(int no)
{
	// no=-1 出征前三个
	if (no == -1 || no == 1) {
		m_pGame->m_pGameProc->Click(60, 166, 100, 180); // 第一个宠物头像
		Sleep(1000);
		m_pGame->m_pGameProc->Click(60, 330, 100, 350); // 合体
	}
	
	if (no == -1) Sleep(1000);
	if (no == -1 || no == 2) {
		m_pGame->m_pGameProc->Click(152, 166, 200, 180); // 第二个宠物头像
		Sleep(1000);
		m_pGame->m_pGameProc->Click(152, 330, 200, 350); //合体
	}
	
	if (no == -1) Sleep(1000);
	if (no == -1 || no == 3) {
		m_pGame->m_pGameProc->Click(250, 166, 300, 180); // 第三个宠物头像
		Sleep(1000);
		m_pGame->m_pGameProc->Click(250, 330, 300, 350); // 合体
	}
	

	return true;
}

// 宠物合体
bool Pet::PetFuck(DWORD* nums, DWORD length)
{
	return true;
}

// 复活所有没有血量宠物
DWORD Pet::Revive()
{
	bool result = false;
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_hWndBig, 62, 162, 305, 168, 0, true);
	if (!result) { // 第一个宠物 62-97
		result = m_pGame->m_pPrintScreen->GetGrayPiexlCount(0, 35, false) > 100;
	}
	if (!result) { // 第二个宠物 152-187
		result = m_pGame->m_pPrintScreen->GetGrayPiexlCount(90, 125, false) > 100;
	}
	if (!result) { // 第三个宠物 252-287
		result = m_pGame->m_pPrintScreen->GetGrayPiexlCount(190, 225, false) > 100;
	}

	if (result && m_pGame->m_pGameProc->IsForegroundWindow()) {
		m_pGame->m_pItem->UseLingYao();
	}
	return 0;
}