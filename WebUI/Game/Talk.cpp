#include "Game.h"
#include "Talk.h"
#include "PrintScreen.h"
#include "GameProc.h"
#include <My/Common/C.h>

Talk::Talk(Game* p)
{
	m_pGame = p;
}

// 当有多个NPC在一起时, 会有NPC列表选择
bool Talk::IsNeedCheckNPC()
{
	// 多个NPC
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 503, 40, 513, 50, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("多个NPC", nullptr, 1) > 0;
}

// 选择NPC
void Talk::SelectNPC(int no)
{
	int x = 450, x2 = 600, y = 0, y2 = 0;
	if (no == 0)
		y = 90, y2 = 125;
	if (no == 1)
		y = 175, y2 = 205;
	if (no == 2)
		y = 260, y2 = 286;

	m_pGame->m_pGameProc->Click(x, y, x2, y2);
}

// NPC
DWORD Talk::NPC(const char* name)
{
	// 对话按钮坐标[875,340 912,388]
	m_pGame->m_pGameProc->Click(875, 350, 912, 376);
	return 0;
}

// NPC对话选择项
void Talk::Select(DWORD no)
{
	// 第一个 67,360 456,393
	// 第二个 67,425 456,460
	// 第三个 67,425 456,460
	if (no == 0) {
		m_pGame->m_pGameProc->Click(67, 360, 368, 393);
		return;
	}
	if (no == 1) {
		m_pGame->m_pGameProc->Click(67, 425, 368, 460);
		return;
	}
	if (no == 2) {
		m_pGame->m_pGameProc->Click(67, 503, 368, 535);
		return;
	}
}

// NPC对话状态[对话框是否打开]
bool Talk::NPCTalkStatus(int index)
{
	if (index >= 0 && index < 5) {
		int x = 30, y = 372 + (index * 72);
		int x2 = 40, y2 = 382 + (index * 72);
		//printf("NPCTalkStatus:%d %d,%d %d,%d\n", index, x, y, x2, y2);
		m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, x, y, x2, y2, 0, true);
		//m_pGame->m_pPrintScreen->GetPixelCount(0xFFFF0000, 0x00505050, true);
		return m_pGame->m_pPrintScreen->ComparePixel("对话选择", nullptr, 1) > 0;
	}
	else {
		m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 100, 320, 168, 323, 0, true);
		return m_pGame->m_pPrintScreen->CheckPixel(0xFF161616);
	}
}

// 等待对话框打开
bool Talk::WaitTalkOpen(int index, DWORD ms)
{
	if (ms == 0)
		ms = 1500;

	for (int i = 0; i < ms; i+=100) {
		if (NPCTalkStatus(index))
			return true;

		Sleep(100);
	}
	return false;
}

// 等待对话框打开
bool Talk::WaitTalkClose(int index, DWORD ms)
{
	if (ms == 0)
		ms = 1500;

	for (int i = 0; i < ms; i += 100) {
		if (!NPCTalkStatus(index))
			return true;

		Sleep(100);
	}
	return false;
}

// 对话按钮是否已打开, 靠近NPC会出现对话按钮
bool Talk::TalkBtnIsOpen()
{
	// 截取的是对话按钮上面像数
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 886, 355, 896, 365, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("对话按钮", nullptr, 1) > 0;
}

// 练功场窗口是否已打开
bool Talk::LianGongChangIsOpen()
{
	// 截取的是练功场标题那里
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 575, 70, 585, 80, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("练功场", nullptr, 1) > 0;
}

// 关闭练功场窗口
void Talk::CloseLianGongChangBox()
{
	m_pGame->m_pGameProc->Click(906, 66, 910, 68);
}

// 聊天窗口是否已打开
bool Talk::SpeakIsOpen()
{
	// 截取的是左下角鸽子图标
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 42, 682, 52, 692, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("聊天已打开", nullptr, 1) > 0;
}

// 关闭聊天窗口
void Talk::CloseSpeakBox()
{
	m_pGame->m_pGameProc->Click(563, 327, 580, 350);
}

// 确定按钮是否打开
bool Talk::SureBtnIsOpen()
{
	// 截取弹框确定按钮图片
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 585, 450, 600, 465, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("提示框", nullptr, 1) > 0;
}

// 确认按钮是否打开
bool Talk::ConfirmBtnIsOpen()
{
	// 截取确认框取消按钮
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 466, 430, 476, 440, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("确认框", nullptr, 1) > 0;
}

// 操作确认按钮flag=0取消 flag=1确定
void Talk::ClickConfirmBtn(int flag)
{
	for (int i = 0; i < 5; i++) {
		if (flag == 0)
			m_pGame->m_pGameProc->Click(700, 430, 750, 450); // 取消按钮
		if (flag == 1)
			m_pGame->m_pGameProc->Click(700, 430, 750, 450); // 确定按钮

		for (int j = 0; j < 1000; j += 100) {
			Sleep(100);
			if (!ConfirmBtnIsOpen())
				return;
		}
		printf("%d.尝试再次点击确认框按钮\n", i + 1);
	}
}

// 等待确认按钮打开
bool Talk::WaitForConfirmBtnOpen(DWORD ms)
{
	for (int i = 0; i < ms; i += 100) {
		if (ConfirmBtnIsOpen())
			return true;

		Sleep(100);
	}
	return false;
}

// 关闭练功场领取经验框
void Talk::CloseLGCBox()
{
	// 截取确认框取消按钮
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 575, 70, 585, 80, 0, true);
	if (m_pGame->m_pPrintScreen->ComparePixel("练功场", nullptr, 1) > 0) {
		m_pGame->m_pGameProc->Click(908, 65, 912, 68);
		Sleep(500);
	}
}

// 等待社交信息框打开
bool Talk::WaitForSheJiaoBox(DWORD ms)
{
	for (DWORD i = 0; i < ms; i += 100) {
		m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 366, 466, 376, 476, 0, true);
		if (m_pGame->m_pPrintScreen->ComparePixel("社交信息框", nullptr, 1) > 0)
			return true;

		Sleep(100);
	}
	return false;
}

// 是否在游戏画面 没有其他弹层
bool Talk::IsInGamePic()
{
	// 左下角时间或电池
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 60, 690, 186, 706, 0, true);
	return m_pGame->m_pPrintScreen->GetPixelCount(0xFFFFFFFF, 0x00101010) >= 100;
}

// 等待进入游戏画面
bool Talk::WaitForInGamePic(DWORD ms)
{
	for (DWORD i = 0; i < ms; i += 100) {
		if (IsInGamePic())
			return true;

		Sleep(200);
	}
	return false;
}

// 公共框是否打开
bool Talk::CommonBoxIsOpen()
{
	// 公共框
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 460, 50, 470, 60, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("公共框", nullptr, 1) > 0
		|| m_pGame->m_pPrintScreen->ComparePixel("公共框2", nullptr, 1) > 0;
}

// 关闭系统邀请提示框
void Talk::CloseCommonBox()
{
	printf("\n--------------关闭遮挡层--------------\n\n");
	m_pGame->m_pGameProc->Click(1150, 50, 1175, 68);
}

// 关闭所有遮挡层
void Talk::CloseAllBox()
{
	while (!IsInGamePic()) { // 有其他遮挡层
		if (CommonBoxIsOpen()) {
			CloseCommonBox();
		}
		else if (SpeakIsOpen()) {
			CloseSpeakBox();
		}
		else {
			// 关闭按钮
			m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 350, 0, 1280, 350, 0, true);
			ComPoint cp[32];
			if (m_pGame->m_pPrintScreen->ComparePixel("关闭按钮", cp, sizeof(cp)/sizeof(ComPoint))) {
				m_pGame->m_pGameProc->Click(cp[0].x, cp[0].y);
			}
			else {
				break;
			}
		}
		Sleep(800);
	}
}
