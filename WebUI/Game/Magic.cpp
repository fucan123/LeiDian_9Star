#include "Game.h"
#include "Magic.h"
#include "Move.h"
#include "GameProc.h"
#include "PrintScreen.h"
#include <My/Common/C.h>

Magic::Magic(Game* p)
{
	m_pGame = p;
}

// 使用技能
int Magic::UseMagic(const char* name, int mv_x, int mv_y)
{
	int click_x = 0, click_y = 0;
	GetMagicClickPos(name, click_x, click_y);
	if (click_x && click_y) {
		if (!mv_x && !mv_y) {
			DWORD use_ms = GetTickCount();
			printf("使用技能:%s(%d,%d)\n", name, click_x, click_y);
			m_pGame->m_pGameProc->Click(click_x, click_y);

			int wait_ms = strcmp(name, "最终审判") == 0 ? 3000 : 1000;
			for (int i = 1; i <= 10; i++) {
				if (i > 2) {
					if (m_pGame->m_pMove->IsOpenMap()) { // 是否已打开了地图
						m_pGame->m_pMove->CloseMap();
					}
				}

				Sleep(100);
				for (int j = 0; j < wait_ms; j += 100) {
					int result = MagicIsOut(name);
					if (result == -1) {
						return -1;
					}
					if (result > 0) {
						DWORD ms = GetTickCount() - use_ms;
						if (ms >= 300) {
							printf("技能:%s(%d)已完成释放, 用时:%d毫秒\n", name, m_nPixelCount, ms);
							return 1;
						}
					}

					Sleep(100);
				}
				printf("%d.再次使用技能:%s(%d,%d)\n", i + 1, name, click_x, click_y);

				m_pGame->m_pGameProc->Click(click_x, click_y);
			}
			printf("技能:%s未放出\n", name);
		}
		else {
			printf("使用技能:%s(%d,%d) 滑动(%d,%d)\n", name, click_x, click_y, mv_x, mv_y);
			m_pGame->m_pGameProc->Click(click_x, click_y, 0x01);
			m_pGame->m_pGameProc->MouseMove(click_x, click_y, mv_x, mv_y);
			m_pGame->m_pGameProc->Click(click_x + mv_x, click_y + mv_y, 0x02);

			return -1;
		}
	}

	return 0;
}

// 获取技能点击坐标
void Magic::GetMagicClickPos(const char* name, int& x, int& y)
{
	if (strcmp(name, "星陨") == 0) {         // 按键1 最下面
		x = MyRand(1006, 1050);
		y = MyRand(650, 690);
	}
	else if (strcmp(name, "影魂契约") == 0) { // 按键2
		x = MyRand(990, 1030);
		y = MyRand(540, 586);
	}
	else if (strcmp(name, "最终审判") == 0) { // 按键3
		x = MyRand(1052, 1095);
		y = MyRand(452, 503);
	}
	else if (strcmp(name, "虚无空间") == 0) { // 按键4
		x = MyRand(1153, 1196);
		y = MyRand(440, 482);
	}
	else if (strcmp(name, "诸神裁决") == 0) { // ...
		x = MyRand(1045, 1086);
		y = MyRand(360, 395);
	}
	else if (strcmp(name, "炎舞燎原") == 0) { // ...
		x = MyRand(1125, 1160);
		y = MyRand(585, 615);
	}
}

// 技能是否放出
int Magic::MagicIsOut(const char* name)
{
	DWORD color = 0xFFFFFFFF, diff = 0x003A3A3A;
	int need_count = 20;

	int x = 0, y = 0, x2 = 0, y2 = 0;
	if (strcmp(name, "星陨") == 0) {         // 按键1 最下面
		x = 1015, y = 666;
		x2 = 1050, y2 = 680;
	}
	else if (strcmp(name, "影魂契约") == 0) { // 按键2
		x = 1005, y = 562;
		x2 = 1035, y2 = 575;
	}
	else if (strcmp(name, "虚无空间") == 0) { // 按键4
		x = 1166, y = 453;
		x2 = 1196, y2 = 466;
	}
	else if (strcmp(name, "最终审判") == 0) { // 按键3
		x = 1060, y = 460;
		x2 = 1090, y2 = 472;

		color = 0xFFFF0000;
		diff = 0x00205050;
	}
	else {
		return -1;
	}

	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, x, y, x2, y2, 0, true);
	m_nPixelCount = m_pGame->m_pPrintScreen->GetPixelCount(color, diff);
	int result = m_nPixelCount >= need_count ? 1 : 0;
	if (0 && result == 0 && strcmp(name, "最终审判") == 0) { // 最终审判再判断有否冷却时间数字
		result = m_pGame->m_pPrintScreen->GetPixelCount(0xFFFFFFFF, 0x003A3A3A) >= need_count ? 1 : 0;
	}
	return result;
}