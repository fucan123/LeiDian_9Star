#include "Game.h"
#include "GameConf.h"
#include "GameData.h"
#include "GameClient.h"
#include "GameProc.h"
#include "Move.h"
#include "Item.h"
#include "Magic.h"
#include "Talk.h"
#include "Pet.h"
#include "PrintScreen.h"

#include <ShlObj_core.h>
#include <My/Common/mystring.h>
#include <My/Common/func.h>
#include <My/Common/C.h>
#include <My/Common/Explode.h>
#include <psapi.h>
#include <stdio.h>
#include <time.h>

#if 1
#if 0
#define SMSG_D(v) m_pGame->m_pClient->SendMsg2(v)
#else
#define SMSG_D(v)
#endif
#define SMSG_DP(p,...) { sprintf_s(p,__VA_ARGS__);SMSG_D(p); }
#else
#define SMSG_D(v) 
#define SMSG_DP(v) 
#endif
#define SMSG(v) SendMsg(v)
#define SMSG_P(p,...) { sprintf_s(p,__VA_ARGS__);SMSG(p); }
#define SMSG_N(n,...) {char _s[n]; SMSG_P(_s,__VA_ARGS__); }

// !!!
GameProc::GameProc(Game* p)
{
	m_pGame = p;
	m_pGameStep = new GameStep;
}

// 初始化数据
void GameProc::InitData()
{
	m_bStop = false;
	m_bPause = false;
	m_bReStart = false;
	m_bIsCrazy = false;
	m_pStepCopy = nullptr;
	m_pStepLast = nullptr;
	m_pStepRecord = nullptr;

	m_bIsRecordStep = false;
	m_bClearKaiRui = false;
	m_bIsResetRecordStep = false;
	m_bIsFirstMove = true;

	m_nReMoveCount = 0;
	m_nYaoBao = 0;
	m_nYao = 0;
	m_nLiveYaoBao = 6;
	m_nLiveYao = 6;

	ZeroMemory(&m_stLast, sizeof(m_stLast));
	ZeroMemory(&m_ClickCrazy, sizeof(m_ClickCrazy));
}

// 初始化流程
bool GameProc::InitSteps()
{
	m_pGameStep->InitGoLeiMingSteps(); // 初始化神殿去雷鸣步骤
	for (int i = 0; i < m_pGame->m_pGameConf->m_Setting.FBFileCount; i++) {
		m_pGameStep->InitSteps(m_pGame->m_chPath, m_pGame->m_pGameConf->m_Setting.FBFile[i]);
	}
	return m_pGame->m_pGameConf->m_Setting.FBFileCount > 0;
}

// 切换游戏窗口
void GameProc::SwitchGameWnd(HWND hwnd)
{
	m_hWndGame = hwnd;
}

// 切换游戏帐号
void GameProc::SwitchGameAccount(_account_ * account)
{
	m_pAccount = account;
}

// 神殿去雷鸣大陆流程
void GameProc::GoLeiMing()
{
	m_pGameStep->ResetStep(0, 0x02);
	while (ExecStep(m_pGameStep->m_GoLeiMingStep)); // 出神殿
}

// 去领取项链
void GameProc::GoGetXiangLian()
{
}

// 询问项链数量
_account_* GameProc::AskXiangLian()
{
	Account* account = nullptr;
	int max = 0;
	m_pGame->m_pEmulator->List2();
	for (int i = 0; i < m_pGame->m_pEmulator->GetCount(); i++) {
		MNQ* m = m_pGame->m_pEmulator->GetMNQ(i);
		if (!m || !m->Account) // 没有相互绑定
			continue;

		if (!m->Account->IsReadXL) { // 没有读取项链
			SwitchGameWnd(m->Wnd);
			SetForegroundWindow(m->WndTop);
			Sleep(500);
			m->Account->XL = m_pGame->m_pItem->GetBagCount(CIN_XiangLian);
			m->Account->IsReadXL = 1;
		}
		
		if (m->Account->XL > max) {
			max = m->Account->XL;
			account = m->Account;
		}

		printf("%s有项链%d条 %08X\n", m->Account->Name, m->Account->XL, m->Wnd);
	}

	return account;
}

// 去副本门口
void GameProc::GoFBDoor(_account_* account)
{
	if (m_pGame->m_pGameData->IsInFBDoor(account))
		return;
		
	while (true) {
		while (m_bPause) Sleep(500);

		m_pGame->m_pItem->SwitchQuickBar(2);
		Sleep(500);
		printf("%s使用去副本门口星辰之眼\n", account->Name);
		m_pGame->m_pItem->GoFBDoor();
		Sleep(1000);
		if (m_pGame->m_pGameData->IsInFBDoor(account)) {
			Sleep(2000);
			m_pGame->m_pItem->SwitchQuickBar(1);
			break;
		}	
	}
}

// 进入副本
_account_* GameProc::OpenFB()
{
	_account_* account = m_pGame->m_pBig;
	if (!account) {
		printf("没有人有项链, 无法进入副本\n");
		return nullptr;
	}
	printf("%s去开副本[%s], 背包项链数量:%d\n", account->Name, account->IsBig?"大号":"小号", account->XL);

	GoFBDoor(account);
	int i;
_start_:
	SwitchGameWnd(account->Mnq->Wnd);
	printf("%s去副本门口\n", account->Name);
	m_pGame->m_pMove->RunEnd(863, 500, account); //872, 495 这个坐标这里点击副本门
	Sleep(1000);
	printf("%s点击副本门\n", account->Name);

	CloseTipBox();
	Click(800, 323); //800, 323 点击副本门
	Sleep(500);
	//return account;

	while (m_bPause) Sleep(500);
	if (!m_pGame->m_pTalk->WaitTalkOpen(0x00)) {
		if (m_pGame->m_pTalk->IsNeedCheckNPC()) {
			printf("选择第一个NPC\n");
			m_pGame->m_pTalk->SelectNPC(0x00);
		}
		if (!m_pGame->m_pTalk->WaitTalkOpen(0x00)) {
			printf("NPC对话框未打开, 重新再次对话\n");
			goto _start_;
		}
	}
	Sleep(1000);
	m_pGame->m_pTalk->Select(account->IsBig ? 0x00 : 0x01); // 大号用钥匙, 小号用项链
	Sleep(1000);
	m_pGame->m_pTalk->Select(0x00); // 确定进入里面
	Sleep(2000);

	for (i = 0; true; i++) {
		while (m_bPause) Sleep(500);
		if (!m_pGame->m_pGameData->IsInFBDoor()) {
			printf("%s已经进入副本\n", account->Name);
			break;
		}
		if (i == 10)
			goto _start_;

		printf("%s等待进入副本\n", account->Name);
		Sleep(1000);
	}

	return account;
}

// 获取开启副本帐号
_account_ * GameProc::GetOpenFBAccount()
{
	return IsBigOpenFB() ? m_pGame->GetBigAccount() : AskXiangLian();
}

// 是否由大号开启副本
bool GameProc::IsBigOpenFB()
{
	SYSTEMTIME stLocal;
	::GetLocalTime(&stLocal);

	if (stLocal.wHour < 20) // 20点到24点可以免费进
		return false;

	return stLocal.wHour == 23 ? stLocal.wMinute < 59 : true; // 至少要1分钟时间准备
}

// 执行
void GameProc::Exec(_account_* account)
{
}

// 运行
void GameProc::Run(_account_* account)
{
	SwitchGameWnd(account->Mnq->Wnd);
	SwitchGameAccount(account);
	printf("刷副本帐号:%s(%08X )\n", account->Name, m_hWndGame);
	m_pGame->m_pItem->UseYao();
	//m_pGame->m_pItem->DropItem(CIN_YaoBao, 3);
	//Wait(20 * 1000);
	//m_pGame->m_pPet->Revive();
	//SellItem();
	//m_pGame->m_pItem->CheckIn(m_pGame->m_pGameConf->m_stCheckIn.CheckIns, m_pGame->m_pGameConf->m_stCheckIn.Length);
	//m_pGame->m_pItem->CheckOut(m_pGame->m_pGameConf->m_stUse.Uses, m_pGame->m_pGameConf->m_stUse.Length);
	//return;
	char log[64];

start:
	Exec(account);
	while (true) {
		if (IsInFB()) {
			ExecInFB();
		}
		Sleep(2000);
	}
}

// 去入队坐标
void GameProc::GoInTeamPos(_account_* account)
{
	if (!account->Mnq) {
		LOGVARN2(64, "red", "帐号:%s未绑定模拟器", account->Name);
		return;
	}

	SwitchGameWnd(account->Mnq->Wnd);
	SwitchGameAccount(account);

	if (m_pGame->m_pGameData->IsInShenDian(account)) // 先离开神殿
		GoLeiMing();

	if (!m_pGame->m_pGameData->IsInFBDoor(account)) {
#if 1
		GoFBDoor(account);
#else
		Click(950, 35, 960, 40);     // 点击活动中心
		Sleep(2000);
		Click(450, 680, 500, 690);   // 点击副本按钮
		Sleep(1500);
		Click(150, 562, 200, 570);   // 点击星级副本
		Sleep(1000);
		Click(150, 510, 200, 520);   // 点击阿拉玛的哭泣
		Sleep(1000);
		Click(950, 590, 960, 600);   // 点击开始挑战
		Sleep(1000);
		while (!m_pGame->m_pGameData->IsInFBDoor(account))
			Sleep(1000);
#endif	
	}
	Sleep(1000);

	int x = 880, y = 500;
	if (account->IsBig) {
		x = 882;
		y = 507;
	}
	m_pGame->m_pMove->RunEnd(x, y, account);

	Sleep(2000);
	if (account->IsBig) { // 大号创建队伍
		CreateTeam();
	}
	else {
		//return;
		SwitchGameWnd(m_pGame->GetBigAccount()->Mnq->Wnd);
		ViteInTeam();
		Sleep(2000);
		SwitchGameWnd(account->Mnq->Wnd);
		InTeam(account);
		Sleep(1000);
		m_pGame->m_pMove->Run(872, 495, 0, 0, false, account);
		//Sleep(1000);
		//Click(425, 235);
		//872, 495 这里点击副本门
		//425, 235 点击副本门
	}
	SwitchGameAccount(m_pGame->m_pBig);
}

// 创建队伍
void GameProc::CreateTeam()
{
	printf("创建队伍...\n");
	Click(5, 360, 25, 390);     // 点击左侧组队
	Sleep(1000);
	Click(80, 345, 200, 360);   // 点击创建队伍
	Sleep(1500);
	Click(1160, 60, 1166, 66);  // 点击关闭按钮
	Sleep(1000);
	Click(5, 463, 10, 466);     // 收起左侧组队
	printf("创建队伍完成\n");
}

// 邀请入队 大号邀请 
void GameProc::ViteInTeam()
{
	printf("准备邀请入队\n");
	Click(766, 235);           // 点击人物模型
	Sleep(1000);
	Click(410, 35, 415, 40);   // 点击人物头像
	Sleep(1000);
	Click(886, 236, 900, 245); // 点击邀请入队
	printf("完成邀请入队\n");
}

// 入队
void GameProc::InTeam(_account_* account)
{
	printf("%s准备同意入队\n", account->Name);
	SwitchGameWnd(account->Mnq->Wnd);
	SetForegroundWindow(account->Mnq->WndTop);
	AgreenMsg("入队旗帜图标");
	printf("%s完成同意入队\n", account->Name);
}

void GameProc::InFB(_account_* account)
{
	printf("%s同意进副本\n", account->Name);
	int max = 10;
	for (int i = 1; i <= max; i++) {
		if (i > 1) {
			printf("%s第(%d/%d)次尝试同意进副本\n", account->Name, i, max);
		}

		SwitchGameWnd(account->Mnq->Wnd);
		SetForegroundWindow(account->Mnq->WndTop);
		Sleep(500);
		AgreenMsg("进副本图标");
		Sleep(5000);

		if (IsInFB(account))
			break;
	}
	
	printf("%s完成同意进副本\n", account->Name);
}

// 所有人进副本
void GameProc::AllInFB(_account_* account_open)
{
	int count = m_pGame->m_pEmulator->List2();
	for (int i = 0; i < count; i++) {
		MNQ* m = m_pGame->m_pEmulator->GetMNQ(i);
		if (!m || !m->Account) // 没有相互绑定
			continue;
		if (account_open == m->Account)
			continue;

		InFB(m->Account);
	}
}

// 同意系统信息
void GameProc::AgreenMsg(const char* name, HWND hwnd)
{
	for (int i = 0; i < 3; i++) { // 从最下面开始算
		if (m_bPause)
			break;

		int result = AgreenMsg(name, i, false, hwnd);
		if (result == 1)
			return;
		if (result == -1)
			break;

		Sleep(1000);
	} 
	AgreenMsg(name, 0, true, hwnd); // 找不到强制点一下第一个
}

// 同意系统信息
int GameProc::AgreenMsg(const char* name, int icon_index, bool click, HWND hwnd)
{
	int x = 320, x2 = 350;
	int y = 0, y2 = 0;
	if (icon_index == 0) {
		y = 670;
		y2 = 680;
	}
	if (icon_index == 1) {
		y = 580;
		y2 = 585;
	}
	if (icon_index == 2) {
		y = 480;
		y2 = 485;
	}

	bool is_open = false;
	for (int i = 1; i <= 3; i++) {
		printf("%d.点击社交图标(%d)展开信息\n", i, icon_index);
		Click(x, y, x2, y2); // 点击社交信息图标 580
		if (m_pGame->m_pTalk->WaitForSheJiaoBox()) {
			is_open = true;
			break;
		}

		printf("信息框未打开, 再次打开\n");
	}
	if (!is_open)
		return -1;

	Sleep(1000);

	// 截取信息图标
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(hwnd ? hwnd : m_hWndGame, 360, 225, 406, 276, 100, true);
	if (!m_pGame->m_pPrintScreen->ComparePixel(name, nullptr, 1)) { // 不是这个信息
		printf("信息框不符合\n");
		if (!click) {
			printf("关闭展开的信息框\n");
			Click(933, 166, 935, 170); // 关闭
			return 0;
		}
	}

	printf("点击接受或同意按钮\n");
	Click(835, 475, 850, 490); // 点击接受按钮同意
	return 1;
}

// 是否在副本
bool GameProc::IsInFB(_account_* account)
{
    //return true;
	DWORD x = 0, y = 0;
	m_pGame->m_pGameData->ReadCoor(&x, &y, account);
	if (x >= 882 && x <= 930 && y >= 1055 && y < 1115) // 刚进副本区域
		return true;

	// 截取地图名称第一个字
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_hWndGame, 1090, 5, 1255, 23, 0, true);
	return m_pGame->m_pPrintScreen->CompareImage(CIN_InFB, nullptr, 1) > 0;
}

// 执行副本流程
void GameProc::ExecInFB()
{
	if (1) {
		printf("执行副本流程\n");

		InitData();
		m_pGameStep->SelectRandStep();
		//m_pGameStep->ResetStep();
		//SendMsg("开始刷副本");
		int start_time = time(nullptr);
		m_nStartFBTime = start_time;
		m_nUpdateFBTimeLongTime = start_time;

#if 0
		/* 只有点击了副本地图, 修改坐标才可以自动寻路 */
		m_pGame->m_pMove->OpenMap(m_pGame->m_pBig);
		Sleep(100);
		Click(250, 600, 255, 605, 0xff, m_pGame->m_pBig->Mnq->Wnd); // 点击地图上
		Sleep(100);
		m_pGame->m_pMove->CloseMap(m_pGame->m_pBig->Mnq->Wnd);
		Wait(1 * 1000);
		if (m_pGame->m_pMove->IsOpenMap()) {
			m_pGame->m_pMove->CloseMap(m_pGame->m_pBig->Mnq->Wnd);
			Sleep(1000);
		}
#endif
		m_pGame->m_pTalk->WaitForInGamePic();
		Sleep(500);
		// 切换到技能快捷栏
		m_pGame->m_pItem->SwitchMagicQuickBar();
		Sleep(500);
		// 收起左侧组队
		Click(5, 463, 10, 466);

		while (ExecStep(m_pGameStep->m_Step, true)); // 大号刷副本
		m_bLockGoFB = false;

		int end_time = time(nullptr);
		int second = end_time - start_time;
		printf("执行副本流程完成，总用时:%02d分%02d秒\n", second / 60, second % 60);
		char log[64];
		sprintf_s(log, "完成刷副本，总用时:%02d分%02d秒", second / 60, second % 60);
		//SendMsg(log);
		m_pGameStep->ResetStep();

		m_pGame->UpdateFBCountText(++m_nPlayFBCount, true);
		m_pGame->UpdateFBTimeLongText(end_time - m_nStartFBTime + m_nFBTimeLong, end_time - m_nUpdateFBTimeLongTime); // 更新时间
		m_nUpdateFBTimeLongTime = end_time;
		m_nFBTimeLong = end_time - m_nStartFBTime + m_nFBTimeLong;
		//GoFBDoor();
		Sleep(1000);
		while (!m_pGame->m_pGameData->IsInFBDoor(m_pGame->GetBigAccount())) { // 等待出来副本
			Sleep(1000);
		}
		m_pGame->m_pTalk->WaitForInGamePic();
		Sleep(500);

		int wait_s = 60;
		int out_time = time(nullptr);
		SellItem();

		//if (!GetOpenFBAccount()) // 没有帐号了
		//	return;
		while (true) {
			int c = (time(nullptr) - out_time);
			if (c > wait_s)
				break;

			printf("等待%d秒再次开启副本, 还剩%d秒\n", wait_s, wait_s - c);
			Sleep(1000);
		}

		m_pGame->m_pServer->AskXLCount();
		//Wait((wait_s - (time(nullptr) - out_time)) * 1000);
	}
}

// 执行流程
bool GameProc::ExecStep(Link<_step_*>& link, bool isfb)
{
	m_pStep = m_pGameStep->Current(link);
	if (!m_pStep) {
		return false;
	}

	// 设置控制台标题
	m_pGameStep->SetConsoleTle(m_pStep->Cmd);
	_step_* m_pTmpStep = m_pStep; // 临时的

	if (isfb) {
		int now_time = time(nullptr);
		m_pGame->UpdateFBTimeLongText(now_time - m_nStartFBTime + m_nFBTimeLong, now_time - m_nUpdateFBTimeLongTime); // 更新时间
		m_nUpdateFBTimeLongTime = now_time;

		if (m_pStepCopy) { // 已经执行到的步骤
			if (m_pStep == m_pStepCopy) {
				m_pStepCopy = nullptr;
			}
			else {
				if (m_pStep->OpCode != OP_MOVE && m_pStep->OpCode != OP_MAGIC) { // 不是移动或放技能
					return m_pGameStep->CompleteExec(link) != nullptr;
				}
			}
		}

		bool check_box = true;

		if (m_pStep->OpCode == OP_MOVE || m_pStep->OpCode == OP_NPC || m_pStep->OpCode == OP_WAIT
			|| m_pStep->OpCode == OP_PICKUP || m_pStep->OpCode == OP_KAIRUI) {
			CloseTipBox(); // 关闭弹出框
			CloseSystemViteBox(); // 关闭系统邀请框

			if (m_pStep->OpCode != OP_NPC) {
				m_pGame->m_pPet->Revive();
			}
			if (m_pGame->m_pTalk->SpeakIsOpen()) {
				m_pGame->m_pTalk->CloseSpeakBox();
				Sleep(500);
			}
			if (m_pGame->m_pItem->BagIsOpen()) {
				m_pGame->m_pItem->CloseBag();
				check_box = false;
				Sleep(500);
			}
		}

		if (check_box) {
			if (m_pGame->m_pTalk->CommonBoxIsOpen()) {
				m_pGame->m_pTalk->CloseCommonBox();
				Sleep(500);
				
				check_box = false;
			}
		}
	}

	bool bk = false;
	char msg[128];
	switch (m_pStep->OpCode)
	{
	case OP_MOVE:
		printf("流程->移动:%d.%d至%d.%d\n", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		if (m_pStep->X2 && m_pStep->Y2) {
			m_pStep->X = MyRand(m_pStep->X, m_pStep->X2);
			m_pStep->Y = MyRand(m_pStep->Y, m_pStep->Y2);

			m_pStep->X2 = 0;
			m_pStep->Y2 = 0;

			printf("实际移动位置:%d,%d\n", m_pStep->X, m_pStep->Y);
		}
		Move(true);
		break;
	case OP_MOVEFAR:
		printf("流程->传送:%d.%d\n", m_pStep->X, m_pStep->Y);
		SMSG_DP(msg, "流程->传送.%d,%d", m_pStep->X, m_pStep->Y);
		Move();
		break;
	case OP_MOVERAND:
		printf("流程->随机移动:%d.%d至%d.%d\n", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		if (MyRand(1, 2, MyRand(m_pStep->X, m_pStep->Y)) == 1) {
			if (m_pStep->X2 && m_pStep->Y2) {
				m_pStep->X = MyRand(m_pStep->X, m_pStep->X2);
				m_pStep->Y = MyRand(m_pStep->Y, m_pStep->Y2);

				m_pStep->X2 = 0;
				m_pStep->Y2 = 0;

				printf("实际移动位置:%d,%d\n", m_pStep->X, m_pStep->Y);
				Move(true);
			}
		}
		else {
			printf("随机移动跳过\n\n");
			bk = true;
		}
		break;
	case OP_NPC:
		printf("流程->NPC:%s(点击:%d,%d 至 %d,%d)\n", m_pStep->NPCName, m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		//SMSG_DP(msg, "流程->NPC.%s 坐标{%d,%d}", m_pStep->NPCName, m_stLastStepInfo.MvX, m_stLastStepInfo.MvY);
		m_stLast.NPCOpCode = OP_NPC;
		NPC();
		break;
	case OP_SELECT:
		printf("流程->选项:%d\n", m_pStep->SelectNo);
		SMSG_DP(msg, "流程->选项:%d", m_pStep->SelectNo);
		Select();
		break;
	case OP_MAGIC:
		printf("流程->技能.%s 使用次数:%d\n", m_pStep->Magic, m_pStep->OpCount);
		Magic();
		break;
	case OP_MAGIC_PET:
		printf("流程->宠物技能:%s\n", m_pStep->Magic);
		SMSG_DP(msg, "流程->宠物技能:%s", m_pStep->Magic);
		//MagicPet();
		break;
	case OP_KAWEI:
		printf("卡位->使用技能:%s 等待:%d秒\n", m_pStep->Magic, m_pStep->WaitMs / 1000);
		KaWei();
		break;
	case OP_CRAZY:
		SMSG_D("流程->疯狂");
		//Crazy();
		break;
	case OP_CLEAR:
		SMSG_D("流程->清理");
		//Clear();
		break;
	case OP_KAIRUI:
		printf("流程->凯瑞 移动:(%d,%d) 截屏:(%d,%d)-(%d,%d)\n", m_pStep->X, m_pStep->Y, 
			m_pStep->Extra[0], m_pStep->Extra[1], m_pStep->Extra[2], m_pStep->Extra[3]);
		KaiRui();
		break;
	case OP_PICKUP:
		printf("流程->捡拾物品:%s, 截屏:(%d,%d)-(%d,%d)\n", m_pStep->Name, m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		PickUp();
		break;
	case OP_CHECKIN:
		printf("流程->存入物品\n");
		SMSG_D("流程->存入物品");
		CheckIn();
		break;
	case OP_USEITEM:
		printf("流程->使用物品\n");
		SMSG_D("流程->使用物品");
		//UseItem();
		break;
	case OP_DROPITEM:
		printf("流程->丢弃物品:%s\n", m_pStep->Name);
		SMSG_DP(msg, "流程->丢弃物品:%s", m_pStep->Name);
		DropItem();
		break;
	case OP_SELL:
		printf("流程->售卖物品\n");
		SMSG_D("流程->售卖物品");
		//SellItem();
		break;
	case OP_BUTTON:
		printf("流程->确定\n");
		Button();
		break;
	case OP_CLICK:
		m_stLast.NPCOpCode = OP_CLICK;
		m_stLast.ClickX = m_pStep->X;
		m_stLast.ClickY = m_pStep->Y;
		m_stLast.ClickX2 = 0;
		m_stLast.ClickY2 = 0;
		printf("流程->点击:%d,%d %d,%d\n", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		if (m_pStep->X2 && m_pStep->Y2) {
			m_stLast.ClickX2 = m_pStep->X2;
			m_stLast.ClickY2 = m_pStep->Y2;
			Click(m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		}
		else {
			Click(m_pStep->X, m_pStep->Y);
		}
		break;
	case OP_CLICKRAND:
		printf("随机点击:%d,%d %d,%d 次数:%d\n", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2, m_pStep->OpCount);
		ClickRand();
		break;
	case OP_CLICKCRAZ:
		printf("开启狂点:%d,%d %d,%d 次数:%d\n", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2, m_pStep->OpCount);
		m_ClickCrazy.X = m_pStep->X;
		m_ClickCrazy.Y = m_pStep->Y;
		m_ClickCrazy.X2 = m_pStep->X2;
		m_ClickCrazy.Y2 = m_pStep->Y2;
		m_ClickCrazy.Count = m_pStep->OpCount;
		bk = true;
		break;
	case OP_WAIT:
		printf("流程->等待:%d %s %d\n", m_pStep->WaitMs / 1000, m_pStep->Magic, m_pStep->Extra[0]);
		Wait();
		break;
	case OP_SMALL:
		if (m_pStep->SmallV == -1) {
			printf("流程->小号:未知操作\n");
		}
		else {
			printf("流程->小号:%d %s\n", m_pStep->SmallV, m_pStep->SmallV == 0 ? "出副本" : "进副本");
			Small();
		}
		break;
	case OP_RECORD:
		printf("\n--------准备记录下一步骤--------\n\n");
		m_bIsRecordStep = true;
		bk = true;
		break;
	default:
		SMSG_D("流程->未知");
		break;
	}

	SMSG_D("流程->执行完成");
	if (bk) {
		return m_pGameStep->CompleteExec(link) != nullptr;
	}
	if (m_bIsResetRecordStep) {
		printf("\n--------重置到已记录步骤------\n\n");
		m_pGameStep->ResetStep(m_pStepRecord->Index, 0x01);
		m_bIsResetRecordStep = false;
		return true;
	}

	int mov_i = 0;
	int drop_i = 0;
	int move_far_i = 0;
	do {
		do {
			if (m_bStop || m_bReStart)
				return false;
			if (m_bPause)
				Sleep(500);
		} while (m_bPause);

		Sleep(100);

		bool use_yao_bao = false;
		if (m_pStep->OpCode == OP_MOVE || m_pStep->OpCode == OP_MOVERAND) {
			mov_i++;
			if (mov_i == 3 && mov_i <= 6) {
				CloseTipBox(); // 关闭弹出框
			}
			if (mov_i > 0 && (mov_i % 20) == 0) {
				m_pGame->m_pItem->GetQuickYaoOrBaoNum(m_nYaoBao, m_nYao);
				if (m_bIsFirstMove) {
					m_pGame->m_pItem->SwitchMagicQuickBar(); // 切换到技能快捷栏
				}
			}
			if (mov_i > 10 && m_nReMoveCount == 0) {
				if ((GetTickCount() & 0xbf) == 0) {
					Click(298, 40, 327, 68);
				}
			}

			if ((mov_i % 5) == 0) {
				// 边走路边用药
				if (m_nYaoBao > m_nLiveYaoBao) { // 药包大于6
					if (m_nYao < 2) { // 药小于2
						m_pGame->m_pItem->UseYaoBao(); // 用药包
						m_nYaoBao--;
						m_nYao += 6;
					}
					else {
						m_pGame->m_pItem->UseYao(1, false, 100); // 用药
						m_nYao--;
					}

					use_yao_bao = true;
				}
				else {
					if (m_nYao > m_nLiveYao) { // 药多于6
						m_pGame->m_pItem->UseYao(1, false, 100);
						m_nYao--;

						use_yao_bao = true;
					}
				}
			}

			// 疯狂点击坐标
			if (m_ClickCrazy.Count) {
				ClickCrazy();
			}
		}

		if (m_pStep->OpCode != OP_DROPITEM) {
			SMSG_D("判断加血");
			int life_flag = IsNeedAddLife();
			if (life_flag == 1) { // 需要加血
				SMSG_D("加血");
				m_pGame->m_pItem->UseYao();
				SMSG_D("加血->完成");
			}
			else if (life_flag == -1) { // 需要复活
				ReBorn(); // 复活
				m_pStepCopy = m_pStep; // 保存当前执行副本
				m_pGameStep->ResetStep(0); // 重置到第一步
				printf("重置到第一步\n");
				return true;
			}
			SMSG_D("判断加血->完成");
		}

		//SMSG_D("判断流程");
		bool complete = StepIsComplete();
		//SMSG_D("判断流程->完成");
		if (m_pStep->OpCode == OP_MOVEFAR) {
			if (++move_far_i == 300) {
				printf("传送超时\n");
				complete = true;
			}
		}

		if (m_nReMoveCount > 20) {
			printf("--------可能已卡住--------\n");
			if (m_nReMoveCount < 25 && (m_stLast.OpCode == OP_NPC || m_stLast.OpCode == OP_SELECT)) {
				printf("--------尝试再次对话NPC--------\n");
				NPCLast(1);
				m_pGame->m_pTalk->WaitTalkOpen(0x00);
				Sleep(1000);
				m_pGame->m_pTalk->Select(0x00);
				Sleep(500);
				printf("--------尝试对话NPC完成，需要移动到:(%d,%d)-------\n", m_pStep->X, m_pStep->Y);
				m_nReMoveCount++;
			}
			printf("--------%d次后重置到记录步骤--------\n", 25 - m_nReMoveCount);
			if (m_nReMoveCount >= 25 && m_pStepRecord) {
				printf("--------重置到已记录步骤------\n");
				m_pGameStep->ResetStep(m_pStepRecord->Index, 0x01);
				m_nReMoveCount = 0;
				return true;
			}
		}

		if (complete) { // 已完成此步骤
			printf("流程->已完成此步骤\n\n");
			if (m_bIsRecordStep) { // 记录此步骤
				m_pStepRecord = m_pStep;
				m_bIsRecordStep = false;
				printf("--------已记录此步骤--------\n\n");
			}

			m_stLast.OpCode = m_pStep->OpCode;
			m_pStepLast = m_pStep;
			
			if (m_pStep->OpCode != OP_NPC && m_pStep->OpCode != OP_SELECT && m_pStep->OpCode != OP_WAIT) {
				//if (m_pGame->m_pTalk->NPCTalkStatus()) // 对话框还是打开的
				//	m_pGame->m_pTalk->NPCTalk(0xff);   // 关掉它
			}

			_step_* next = m_pGameStep->CompleteExec(link);
			if (m_pStep->OpCode != OP_WAIT && next) {
				Sleep(use_yao_bao ? 100 : 50);
			}
			return next != nullptr;
		}
	} while (true);

	return false;
}

// 步骤是否已执行完毕
bool GameProc::StepIsComplete()
{
	bool result = false;
	switch (m_pStep->OpCode)
	{
	case OP_MOVE:
	case OP_MOVERAND:
		//result = true;
		SMSG_D("判断是否移动到终点&是否移动", true);
		if (m_pGame->m_pMove->IsMoveEnd(m_pAccount)) { // 已到指定位置
			m_stLast.MvX = m_pStep->X;
			m_stLast.MvY = m_pStep->Y;
			m_nReMoveCount = 0;
			result = true;

			if (m_bIsFirstMove) {
				// 切换到技能快捷栏
				m_pGame->m_pItem->SwitchMagicQuickBar();
				m_bIsFirstMove = false;
			}
			goto end;
		}
		SMSG_D("判断是否移动到终点->完成", true);
		if (!m_pGame->m_pMove->IsMove(m_pAccount)) {   // 已经停止移动
			Move(false);
			m_nReMoveCount++;
		}
		SMSG_D("判断是否移动->完成", true);
		break;
	case OP_MOVEFAR:
		if (m_pGame->m_pGameData->IsNotInArea(m_pStep->X, m_pStep->Y, 50, m_pAccount)) { // 已不在此区域
			result = true;
			m_nReMoveCount = 0;
			goto end;
		}
		if (m_pGame->m_pMove->IsMoveEnd(m_pAccount)) { // 已到指定位置
			m_pGame->m_pMove->Run(m_pStep->X - 5, m_pStep->Y, m_pAccount, m_pStep->X2, m_pStep->Y2);
			m_nReMoveCount = 0;
		}
		if (!m_pGame->m_pMove->IsMove(m_pAccount)) {   // 已经停止移动
			m_pGame->m_pMove->Run(m_pStep->X, m_pStep->Y, m_pAccount, m_pStep->X2, m_pStep->Y2);
			m_nReMoveCount++;
		}
		break;
	default:
		result = true;
		break;
	}
end:
	return result;
}

// 移动
void GameProc::Move(bool rand_click)
{
	if (m_bIsFirstMove) {
		DWORD x = MyRand(162, 900);
		DWORD y = MyRand(162, 550);
		m_pGame->m_pMove->Run(m_pStep->X, m_pStep->Y, m_pAccount, x, y, true, rand_click);
	}
	else {
		m_pGame->m_pMove->Run(m_pStep->X, m_pStep->Y, m_pAccount, m_pStep->X2, m_pStep->Y2, false, rand_click);
	}
	//m_pGame->m_pItem->UseYao();
	//OpenMap();
	//Sleep(500);
	//Click(215, 473, NULL);
	//Sleep(100);
	//GoWay();
}

// 对话
void GameProc::NPC()
{
	if (strcmp("嗜血骑士", m_pStep->NPCName) == 0) {
		m_pGame->m_pItem->SwitchMagicQuickBar();
	}
	else if (strcmp("炎魔督军祭坛", m_pStep->NPCName) == 0) {
		m_nLiveYaoBao = 2;
		m_nLiveYao = 1;
	}
	else if (strcmp("传送门", m_pStep->NPCName) == 0) {
		MouseWheel(-240);
		Sleep(1000);
	}

	NPC(m_pStep->NPCName, m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);

	strcpy(m_stLast.NPCName, m_pStep->NPCName);
	m_stLast.ClickX = m_pStep->X;
	m_stLast.ClickY = m_pStep->Y;
	m_stLast.ClickX2 = m_pStep->X2;
	m_stLast.ClickY2 = m_pStep->Y2;
}

// NPC点击
void GameProc::NPC(const char* name, int x, int y, int x2, int y2)
{
	CloseItemUseTipBox();

	bool click_btn = true;
	if (name && strstr(name, "之书"))
		click_btn = false;

	if (click_btn && m_pGame->m_pTalk->TalkBtnIsOpen()) {
		printf("点击对话按钮\n");
		m_pGame->m_pTalk->NPC();
		Sleep(100);
	}
	else {
		if (!x || !y) { // 没有写坐标
			int count = 0;
			while (!m_pGame->m_pTalk->TalkBtnIsOpen()) {
				if (++count == 3)
					break;
				Sleep(500);
			}
			m_pGame->m_pTalk->NPC();
		}

		if (x2) {
			x = MyRand(x, x2);
			//printf("x2:%d\n", x2);
		}
			
		if (y2) {
			y = MyRand(y, y2);
			//printf("y2:%d\n", y2);
		}
			
		Click(x, y);
		printf("点击NPC:%d,%d\n", x, y);
	}
}

// 最后一个对话的NPC
bool GameProc::NPCLast(bool check_pos, DWORD mov_sleep_ms)
{
	CloseTipBox();

	bool is_move = false;
	if (check_pos) {
		bool need_move = true;
		if (m_pStep->OpCount == 1) {
			need_move = !m_pGame->m_pTalk->TalkBtnIsOpen();
		}
		if (need_move) {
			DWORD pos_x = 0, pos_y = 0;
			m_pGame->m_pGameData->ReadCoor(&pos_x, &pos_y, m_pAccount);
			if (pos_x != m_stLast.MvX || pos_y != m_stLast.MvY) {
				while (m_bPause) Sleep(500);

				printf("重新移动到:(%d,%d) 现在位置:(%d,%d)\n", m_stLast.MvX, m_stLast.MvY, pos_x, pos_y);
				if (mov_sleep_ms) {
					Sleep(mov_sleep_ms);
				}
				m_pGame->m_pMove->RunEnd(m_stLast.MvX, m_stLast.MvY, m_pAccount);
				Wait(1 * 1000);
				is_move = true;
			}
		}
		
	}

	if (m_stLast.NPCOpCode == OP_NPC) {
		printf("再次点击对话打开NPC(%s)\n", m_stLast.NPCName);
		NPC(m_stLast.NPCName, m_stLast.ClickX, m_stLast.ClickY, m_stLast.ClickX2, m_stLast.ClickY2);
	}
	else if (m_stLast.NPCOpCode == OP_CLICK) {
		if (0 && m_stLast.ClickX2 && m_stLast.ClickY2) {
			Click(m_stLast.ClickX, m_stLast.ClickY);
			printf("再次点击(%d,%d)-(%d,%d)打开NPC(%s)\n", m_stLast.ClickX, m_stLast.ClickY, m_stLast.ClickX2, m_stLast.ClickY2, m_stLast.NPCName);
		}
		else {
			Click(m_stLast.ClickX, m_stLast.ClickY);
			printf("再次点击(%d,%d)打开NPC(%s)\n", m_stLast.ClickX, m_stLast.ClickY, m_stLast.NPCName);
		}
	}

	return is_move;
}

// 选择
void GameProc::Select()
{
	if (strcmp("传送门", m_stLast.NPCName) == 0) {
		int n = 0;
		while (true) {
			if (m_pGame->m_pTalk->WaitTalkOpen(m_pStep->SelectNo)) {
				Sleep(500);
				m_pGame->m_pTalk->Select(m_pStep->SelectNo);
				Sleep(1500);
				m_pGame->m_pTalk->Select(m_pStep->SelectNo);
				Sleep(1500);
			}
			
			if (m_pGame->m_pGameData->IsInFBDoor(m_pAccount))
				break;

			if (n++ >= 10) {
				m_bIsResetRecordStep = true;
				break;
			}

			NPCLast(true, 1000);
		}
		return;
	}

	int add_count = m_pStep->OpCount == 1 ? 0 : 0;
	for (DWORD i = 1; i <= m_pStep->OpCount; i++) {
		int max_j = 2;
		bool open = false;
		for (int j = 0; j < max_j; j++) {
			while (m_bPause) Sleep(500);

			if (m_pGame->m_pTalk->WaitTalkOpen(m_pStep->SelectNo)) {
				open = true;
				break;
			}
			else {
				CloseItemUseTipBox();

				bool check_pos = i < 6 && strcmp("传送门", m_stLast.NPCName) != 0;
				if (NPCLast(check_pos, 600 + j * 300)) {
					max_j += add_count;
					add_count = 0;
				}
			}
		}
		if (!open) {
			int _clk = 0;
			if (0 && !IsForegroundWindow()) {
				Sleep(MyRand(680, 800, i));
				m_pGame->m_pTalk->Select(m_pStep->SelectNo);
				_clk = 1;
			}

			if (_clk) {
				m_pGame->m_pTalk->Select(m_pStep->SelectNo);
			}

			printf("等待对话框打开超过%d次,跳过. 点击:%d\n", max_j, _clk);
			goto _check_;
		}

		Sleep(MyRand(300, 350, i));

		printf("第(%d)次选择 选项:%d\n", i, m_pStep->SelectNo);
		m_pGame->m_pTalk->Select(m_pStep->SelectNo);
		if (!m_pGame->m_pTalk->WaitTalkClose(m_pStep->SelectNo)) {
			//m_pGame->m_pTalk->Select(m_pStep->SelectNo);
			//m_pGame->m_pTalk->WaitTalkClose(m_pStep->SelectNo);
		}

		if (i > 1) {
			Sleep(150);
			if (IsNeedAddLife() == 1) {
				Sleep(300);
				m_pGame->m_pItem->UseYao(3);
				NPCLast();
				i--;
				printf("i--:%d\n", i);
			}
		}
	}
	if (m_pStep->OpCount >= 10) {
		printf("加满血\n");
		m_pGame->m_pItem->AddFullLife();
	}

_check_:
	if (m_pStep->OpCount == 1 || m_pStep->OpCount >= 10) {
		if (strcmp(m_stLast.NPCName, "四骑士祭坛") == 0 || strcmp(m_stLast.NPCName, "女伯爵祭坛") == 0) {
			Sleep(500);
			if (CloseTipBox()) {
				printf("--------有封印未解开，需要重置到记录步骤--------\n");
				m_bIsResetRecordStep = true;
				return;
			}
			else {
				printf("--------封印已全部解开--------\n");
			}
		}

		if (IsCheckNPC(m_stLast.NPCName)) {
			printf("↑↑↑检查NPC:%s↑↑↑\n", m_stLast.NPCName);
			int no = m_pStep->SelectNo;
			int i_max = no >= 10 ? 5 : 3;
			for (int i = 1; i <= i_max; i++) {
				if (m_pStep->SelectNo == 0x02)
					break;

				Sleep(i & 0x01 ? 300 : 300);
				if (!m_pGame->m_pTalk->TalkBtnIsOpen())
					break;

				if (i == 1)
					continue;

				printf("%d.NPC未对话完成, 重新点击对话按钮\n", i);
				NPCLast(true);
				if (m_pGame->m_pTalk->WaitTalkOpen(m_pStep->SelectNo)) {
					Sleep(MyRand(300, 350, i));
					if (i >= 3 && m_pStep->OpCount >= 10) {
						no = 0x00;
						i--;
					}

					printf("%d.选项:%d\n", i, no);
					m_pGame->m_pTalk->Select(no);
					m_pGame->m_pTalk->WaitTalkClose(m_pStep->SelectNo);

					if (strcmp(m_stLast.NPCName, "四骑士祭坛") == 0 || strcmp(m_stLast.NPCName, "女伯爵祭坛") == 0) {
						Sleep(500);
						if (CloseTipBox()) {
							printf("--------有封印未解开，需要重置到记录步骤--------\n");
							m_bIsResetRecordStep = true;
							return;
						}
						else {
							printf("--------封印已全部解开--------\n");
						}
					}
				}
			}
		}
		else if (IsCheckNPCTipBox(m_stLast.NPCName)) {
			printf("↑↑↑检查NPC弹框:%s↑↑↑\n", m_stLast.NPCName);
			bool result = false;
			for (int i = 0; i < 3; i++) {
				for (int ms = 0; ms < 1500; ms += 100) {
					if (m_pGame->m_pTalk->SureBtnIsOpen()) {
						result = true;
						break;
					}
					Sleep(100);
				}
				if (result)
					break;

				printf("%d.未发现弹框, 重新点击对话按钮\n", i);
				NPCLast(true);
				if (m_pGame->m_pTalk->WaitTalkOpen(m_pStep->SelectNo)) {
					Sleep(MyRand(300, 350, i));
					m_pGame->m_pTalk->Select(m_pStep->SelectNo);
					m_pGame->m_pTalk->WaitTalkClose(m_pStep->SelectNo);
				}
			}
			if (!result && strcmp("阿拉玛的怨念", m_stLast.NPCName) == 0) { // 最后BOSS未解封印
				printf("--------有封印未解开，需要重置到记录步骤--------\n");
				m_bIsResetRecordStep = true;
				return;
			}
		}
	}
}

// 技能
void GameProc::Magic()
{
	for (int i = 1; i <= m_pStep->OpCount; i++) {
		int result = m_pGame->m_pMagic->UseMagic(m_pStep->Magic, m_pStep->X, m_pStep->Y);
		if (m_pStep->OpCount == 1) {
			Sleep(result == -1 ? 500 : 10);
		}
		else {
			Sleep(500);
		}
	}
}

// 卡位
void GameProc::KaWei()
{
	int click_x = 0, click_y = 0;
	m_pGame->m_pMagic->GetMagicClickPos(m_pStep->Magic, click_x, click_y);
	if (click_x && click_y) {
		int mv_x = m_pStep->X, mv_y = m_pStep->Y;
		printf("使用技能:%s(%d,%d) 滑动(%d,%d)\n", m_pStep->Name, click_x, click_y, mv_x, mv_y);
		m_pGame->m_pGameProc->Click(click_x, click_y, 0x01);
		Sleep(500);
		for (int i = 0; i < 1500; i += 50) {
			m_pGame->m_pGameProc->MouseMove(click_x, click_y);
			Sleep(50);
		}
		m_pGame->m_pGameProc->MouseMove(click_x, click_y, mv_x, mv_y);

		int n = 0;
		for (int i = 0; i < m_pStep->WaitMs; i += 500) {
			if (m_bPause)
				break;

			int ls = (m_pStep->WaitMs - i) / 1000;

			if (n != ls) {
				printf("卡位等待%02d秒，还剩%02d秒.\n", m_pStep->WaitMs / 1000, ls);
			}
			n = ls;

			m_pGame->m_pGameProc->MouseMove(click_x + mv_x, click_y + mv_y);
			Sleep(500);
		}
		m_pGame->m_pGameProc->Click(click_x + mv_x, click_y + mv_y, 0x02);
	}
	else {
		printf("KaWei：%d,%d\n", click_x, click_y);
	}
}

// 凯瑞
void GameProc::KaiRui()
{
	if (m_bClearKaiRui) {
		printf("========凯瑞已被清除，跳过========\n");
		return;
	}
	// 移动到指定位置
	m_pGame->m_pMove->RunEnd(m_pStep->X, m_pStep->Y, m_pAccount);
	CloseTipBox();
	Sleep(100);
	// 截取弹框确定按钮图片
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_hWndGame, 
		m_pStep->Extra[0], m_pStep->Extra[1], m_pStep->Extra[2], m_pStep->Extra[3], 0, true);

	int red_count = m_pGame->m_pPrintScreen->GetPixelCount(0xffff0000, 0x00101010);
	int yellow_count = m_pGame->m_pPrintScreen->GetPixelCount(0xffffff00, 0x00020202);
	printf("颜色: 红色(%d), 黄色(%d)\n", red_count, yellow_count);
	if ((red_count > 10 && yellow_count > 50) || yellow_count > 200) {
		printf("========发现凯瑞========\n");
		int max = m_pStep->Extra[4] ? 1 : 5;
		for (int i = 1; i <= max; i++) {
			printf("========使用技能->诸神裁决========\n");
			m_pGame->m_pMagic->UseMagic("诸神裁决");
			if (IsNeedAddLife() == -1)
				m_pGame->m_pItem->UseYao(2);

			Sleep(500);

			if (i & 0x01) {
				int pickup_count = m_pGame->m_pItem->PickUpItem("速效圣兽灵药", m_pStep->Extra[0], m_pStep->Extra[1], m_pStep->Extra[2], m_pStep->Extra[3], 2);
				if (pickup_count > 0) {
					if (pickup_count == 1) { // 可能有两个, 再扫描一次
						m_pGame->m_pItem->PickUpItem("速效圣兽灵药", 450, 300, 780, 475, 2);
					}
					break;
				}
			}
		}
		m_bClearKaiRui = true;
	}
	else if (m_pStep->Extra[4]) { // 扫描圣兽物品
		int pickup_count = m_pGame->m_pItem->PickUpItem("速效圣兽灵药", m_pStep->Extra[0], m_pStep->Extra[1], m_pStep->Extra[2], m_pStep->Extra[3], 2);
		if (pickup_count > 0) {
			printf("========发现凯瑞, 但已被清除========\n");
			m_bClearKaiRui = true;
		}
	}
}

// 捡物
void GameProc::PickUp()
{
	if (CloseTipBox())
		Sleep(500);

	bool to_big = false; // 是否放大屏幕
	if (strcmp(m_pStep->Name, "30星神兽碎片+3") == 0)
		to_big = true;

	if (to_big) {
		MouseWheel(240);
		Sleep(1000);
	}

	int pickup_count = m_pGame->m_pItem->PickUpItem(m_pStep->Name, m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);

	if (strcmp(m_pStep->Name, "30星神兽碎片+3") == 0) {
		if (pickup_count > 5)
			pickup_count = 5;

		int wait_s = 15 - (3 * pickup_count);
		Wait(wait_s * 1000);
	}

	if (to_big) {
		MouseWheel(-240);
		Sleep(1000);
	}
}

// 存物
DWORD GameProc::CheckIn(bool in)
{
	CloseTipBox(); // 关闭弹出框
	m_pGame->m_pItem->CheckIn(m_pGame->m_pGameConf->m_stCheckIn.CheckIns, m_pGame->m_pGameConf->m_stCheckIn.Length);
	return 0;
}

// 使用物
void GameProc::UseItem()
{
}

// 丢物
void GameProc::DropItem()
{
	CloseTipBox(); // 关闭弹出框
	m_pGame->m_pItem->DropItem(CIN_YaoBao);
}

// 售卖物品
void GameProc::SellItem()
{
	printf("\n-----------------------------\n");
	printf("准备去卖东西\n");
	int pos_x = 265, pos_y = 370;
	DWORD _tm = GetTickCount();
	if (!m_pGame->m_pGameData->IsInArea(pos_x, pos_y, 15)) { // 不在商店那里
		int i = 0;
	use_pos_item:
		printf("(%d)切换快捷栏\n", i + 1);
		m_pGame->m_pItem->SwitchQuickBar(2); // 切换快捷栏
		Sleep(500);
		printf("(%d)使用星辰之眼\n", i + 1);
		m_pGame->m_pItem->GoShop();         // 去商店旁边
		Sleep(500);
		for (; i < 100;) {
			if (i > 0 && (i % 10) == 0) {
				i++;
				goto use_pos_item;
			}
			if (m_pGame->m_pGameData->IsInArea(pos_x, pos_y, 15)) { // 已在商店旁边
				Sleep(100);
				break;
			}

			i++;
			Sleep(1000);
		}
		m_pGame->m_pItem->SwitchQuickBar(1); // 切回快捷栏
		Sleep(300);
	}

	m_pGame->m_pMove->RunEnd(pos_x, pos_y, m_pGame->m_pBig); // 移动到固定点好点击
	//Sleep(500);
	//MouseWheel(-260);
	//m_pGame->m_pMove->RunEnd(pos_x, pos_y, m_pGame->m_pBig); // 移动到固定点好点击
	Sleep(1000);

	int rand_v = GetTickCount() % 2;
	int clk_x, clk_y, clk_x2, clk_y2;
	if (rand_v == 0) { // 装备商
		clk_x = 563, clk_y = 545;
		clk_x2 = 565, clk_y2 = 546;
	}
	else { // 武器商
		clk_x = 306, clk_y = 450;
		clk_x2 = 335, clk_y2 = 500;
	}
	Click(clk_x, clk_y, clk_x2, clk_y2);      // 对话商店人物
	m_pGame->m_pTalk->WaitTalkOpen(0x00);
	Sleep(1000);
	m_pGame->m_pTalk->Select(0x00); // 购买物品
	Sleep(1500);

	m_pGame->m_pItem->SellItem(m_pGame->m_pGameConf->m_stSell.Sells, m_pGame->m_pGameConf->m_stSell.Length);
	Sleep(500);
	m_pGame->m_pItem->SetBag();
	Sleep(500);
	m_pGame->m_pItem->CloseShop();
	Sleep(500);
	if (m_pGame->m_pItem->CheckOut(m_pGame->m_pGameConf->m_stSell.Sells, m_pGame->m_pGameConf->m_stSell.Length)) {
		Sleep(500);
		m_pGame->m_pMove->RunEnd(pos_x, pos_y, m_pGame->m_pBig); // 移动到固定点好点击
		Sleep(500);
		Click(clk_x, clk_y, clk_x2, clk_y2);      // 对话商店人物
		Sleep(1000);
		m_pGame->m_pTalk->Select(0x00); // 购买物品
		m_pGame->m_pTalk->WaitTalkOpen(0x00);
		Sleep(1000);
		m_pGame->m_pItem->SellItem(m_pGame->m_pGameConf->m_stSell.Sells, m_pGame->m_pGameConf->m_stSell.Length);
		Sleep(500);
#if 1
		printf("点击左侧修理装备\n");
		Click(8, 260, 35, 358); // 左侧修理装备
		Sleep(1000);
		printf("点击全部修理\n");
		Click(390, 645, 500, 666); // 按钮全部修理
		if (m_pGame->m_pTalk->WaitForConfirmBtnOpen(1500)) {
			Sleep(150);
			printf("确定修理装备\n");
			m_pGame->m_pTalk->ClickConfirmBtn(1); // 确定
			Sleep(500);
			Click(16, 150, 20, 186); // 展开宠物列表
			Sleep(350);
		}
		else {
			printf("无须修理装备\n");
		}
#endif
		m_pGame->m_pItem->CloseShop();
		Sleep(500);
	}

	m_pGame->m_pTalk->CloseAllBox();

	_tm = GetTickCount() - _tm;
	printf("卖东西用时%.2f秒, %d毫秒\n", (float)_tm / 1000.0f, _tm);
	printf("\n-----------------------------\n");
}

// 按钮
void GameProc::Button()
{
	// 确定按钮 597,445 606,450
	CloseTipBox();
}

// 关闭弹框
bool GameProc::CloseTipBox()
{
	// 截取弹框确定按钮图片
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_hWndGame, 585, 450, 600, 465, 0, true);
	if (m_pGame->m_pPrintScreen->ComparePixel("提示框", nullptr, 1)) {
		printf("关闭提示框\n");
		Click(576, 440, 695, 468);
		Sleep(100);

		return true;
	}

	return false;
}

// 关闭物品使用提示框
bool GameProc::CloseItemUseTipBox()
{
	// 截取物品使用提示框按钮前面图片
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_hWndGame, 855, 376, 865, 386, 0, true);
	if (m_pGame->m_pPrintScreen->ComparePixel("物品使用提示框", nullptr, 1)) {
		printf("关闭物品使用提示框\n");
		Click(1032, 180);
		Sleep(300);

		return true;
	}

	return false;
}

// 关闭系统邀请提示框
bool GameProc::CloseSystemViteBox()
{
	// 截取邀请框标题栏背景色
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_hWndGame, 765, 187, 775, 197, 0, true);
	if (m_pGame->m_pPrintScreen->ComparePixel("邀请确认", nullptr, 1)) {
		printf("关闭物品使用提示框\n");
		Click(450, 435, 550, 450);
		Sleep(300);

		return true;
	}

	return false;
}

// 随机点击
void GameProc::ClickRand()
{
	int n = MyRand(2, 3, GetTickCount());
	for (int i = 0; i < m_pStep->OpCount; i++) {
		if (MyRand(1, n, i) == 1)
			continue;

		int x = m_pStep->X, y = m_pStep->Y;
		if (m_pStep->X2)
			x = MyRand(m_pStep->X, m_pStep->X2, i * 20);
		if (m_pStep->Y2)
			y = MyRand(m_pStep->Y, m_pStep->Y2, i * 20);

		Click(x, y);
		printf("随机点击:%d,%d\n", x, y);
		Sleep(MyRand(300, 1000, i * 20));
	}
}

// 狂暴点击
void GameProc::ClickCrazy()
{
	if (m_ClickCrazy.Count <= 0)
		return;

	if (MyRand(1, 2, m_ClickCrazy.Count) == 1) {
		int x = m_ClickCrazy.X, y = m_ClickCrazy.Y;
		if (m_pStep->X2)
			x = MyRand(m_ClickCrazy.X, m_ClickCrazy.X2, m_ClickCrazy.Count * 20);
		if (m_pStep->Y2)
			y = MyRand(m_ClickCrazy.Y, m_ClickCrazy.Y2, m_ClickCrazy.Count * 20);

		Click(x, y);
		printf("狂暴点击:%d,%d 还剩%d次\n", x, y, m_ClickCrazy.Count - 1);
		Sleep(MyRand(300, 500, m_ClickCrazy.Count));
	}
	m_ClickCrazy.Count--;
}

// 等待
void GameProc::Wait()
{
	Wait(m_pStep->WaitMs, m_pStep->Extra[0]);
}

// 等待
void GameProc::Wait(DWORD ms, int no_open)
{
	if (ms < 1000) {
		printf("等待%d毫秒\n", ms);
		Sleep(ms);
		return;
	}

	if (ms >= 2000 && !no_open) {
		DWORD start_ms = GetTickCount();
		if ((start_ms & 0x0f) < 3) {
			Click(16, 26, 66, 80); // 点击头像
			Sleep(1500);
		
			int click_count = MyRand(0, 2 + (ms / 1000 / 3), ms);
			for (int i = 0; i < click_count; i++) { // 胡乱操作一下
				Click(315, 100, 1160, 500);
				Sleep(MyRand(500, 1000, i));
			}

			Click(1155, 55, 1160, 60); // 关闭
			Sleep(500);

			DWORD use_ms = GetTickCount() - start_ms;
			if (use_ms > ms)
				ms = 0;
			else
				ms -= use_ms;
		}
	}

	if (ms >= 12000 && IsNeedAddLife() != -1) {
		printf("等待时间达到%d秒(%d毫秒), 先整理背包\n", ms / 1000, ms);
		DWORD _tm = 0;
		m_pGame->m_pItem->DropItem(CIN_YaoBao, 6, &_tm);
		if (_tm > ms)
			ms = 0;
		else
			ms -= _tm;
	}

	int n = 0;
	for (int i = 0; i < ms; i += 100) {
		int ls = (ms - i) / 1000;

		if (n != ls) {
			printf("等待%02d秒，还剩%02d秒.\n", ms / 1000, ls);
		}
		n = ls;
		Sleep(100);
	}
}

// 小号
void GameProc::Small()
{
#if 1
	if (m_pStep->SmallV == 0) { // 出副本
		m_pGame->m_pServer->SmallOutFB(m_pGame->m_pBig, nullptr, 0);
	}
	if (m_pStep->SmallV == 1) { // 进副本
		m_pGame->m_pServer->SmallInFB(m_pGame->m_pBig, nullptr, 0);
	}
#else
	if (m_pStep->SmallV == 0) { // 出副本
		Click(597, 445, 606, 450); // 关闭提示框
		Sleep(500);
		Click(597, 445, 606, 450); // 点击复活人物
		Sleep(2000);

		int count = m_pGame->m_pEmulator->List2();
		for (int i = 0; i < count; i++) {
			MNQ* m = m_pGame->m_pEmulator->GetMNQ(i);
			if (m && m->Account && !m->Account->IsBig) {
				SwitchGameWnd(m->Wnd);
				SetForegroundWindow(m->WndTop);
				Sleep(100);
				m_pGame->m_pMove->Run(890, 1100, 0, 0, false, m->Account); // 移动到固定地点
			}
		}
		Sleep(1000);

		ClickOther(190, 503, 195, 505, m_pGame->m_pBig); // 点击NPC
		Sleep(2000);
		ClickOther(67, 360, 260, 393, m_pGame->m_pBig);  // 选项0 出副本
		Sleep(1000);
		ClickOther(67, 360, 260, 393, m_pGame->m_pBig);  // 选项0 我要离开这里

		SwitchGameWnd(m_pGame->m_pBig->Mnq->Wnd);
		SetForegroundWindow(m_pGame->m_pBig->Mnq->WndTop);
	}
#endif
}

// 复活
void GameProc::ReBorn()
{
	Wait(26 * 1000);
	CloseTipBox();
	if (m_pGame->m_pItem->BagIsOpen()) {
		m_pGame->m_pItem->CloseBag();
		Sleep(500);
	}
	if (m_pGame->m_pTalk->CommonBoxIsOpen()) {
		m_pGame->m_pTalk->CloseCommonBox();
		Sleep(500);
	}

	while (true) {
		Click(537, 415, 550, 425); // 点复活
		Sleep(2000);

		if (IsNeedAddLife() != -1)
			break;
	}

	Wait(5000);
	m_pGame->m_pPet->PetOut(-1);
	Sleep(1000);
}

// 是否检查此NPC对话完成
bool GameProc::IsCheckNPC(const char* name)
{
	return strcmp(name, "魔封障壁") == 0 || strcmp(name, "地狱火障壁") == 0 || strcmp(name, "爆雷障壁") == 0
		|| strcmp(name, "仇之缚灵柱") == 0 || strcmp(name, "怨之缚灵柱") == 0 || strcmp(name, "嗜血骑士") == 0
		|| strcmp(name, "四骑士祭坛") == 0 || strcmp(name, "女伯爵祭坛") == 0
		|| strcmp(name, "往昔之书") == 0
		|| strstr(name, "元素障壁") != nullptr;
}

// 是否检查此NPC对话完成
bool GameProc::IsCheckNPCTipBox(const char* name)
{
	return strstr(name, "封印机关") != nullptr
		|| strstr(name, "图腾") != nullptr
		|| strstr(name, "印记") != nullptr
		|| strcmp(name, "炎魔督军祭坛") == 0
		|| strcmp(name, "爱娜的灵魂") == 0
		|| strcmp(name, "献身之书") == 0
		|| strcmp(name, "复仇之书") == 0
		|| strcmp(name, "阿拉玛的怨念") == 0;
}


// 鼠标移动[相对于x或y移动rx或ry距离]
void GameProc::MouseMove(int x, int y, int rx, int ry, HWND hwnd)
{
	int dx = abs(rx), dy = abs(ry);
	for (int i = 1; true;) {
		if (i > dx && i > dy)
			break;

		int mv_x = 0, mv_y = 0;
		if (i < dx) {
			mv_x = rx > 0 ? i : -i;
		}
		else {
			mv_x = rx;
		}
		if (i < dy) {
			mv_y = ry > 0 ? i : -i;
		}
		else {
			mv_y = ry;
		}

		Sleep(50);
		MouseMove(x + mv_x, y + mv_y, hwnd);

		printf("MouseMove:%d,%d\n", x + mv_x, y + mv_y);
		i += MyRand(1, 2);
	}
}

// 鼠标移动
void GameProc::MouseMove(int x, int y, HWND hwnd)
{
	if (!hwnd)
		hwnd = m_hWndGame;

	::SendMessage(hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x, y));
	//printf("鼠标移动:%d,%d\n", x, y);
}

// 鼠标滚轮
void GameProc::MouseWheel(int x, int y, int z, HWND hwnd)
{
	if (!hwnd)
		hwnd = m_hWndGame;

	::SendMessage(hwnd, WM_MOUSEWHEEL, MAKEWPARAM(MK_CONTROL, z), MAKELPARAM(x, y));
}

// 鼠标滚轮
void GameProc::MouseWheel(int z, HWND hwnd)
{
	if (!hwnd)
		hwnd = m_hWndGame;

	RECT rect;
	::GetWindowRect(hwnd, &rect);
	MouseWheel(MyRand(rect.left+500, rect.left+800), MyRand(rect.top+236, rect.top+500), z, hwnd);

}

// 鼠标左键点击
void GameProc::Click(int x, int y, int ex, int ey, int flag, HWND hwnd)
{
	Click(MyRand(x, ex), MyRand(y, ey), flag, hwnd);
}

// 鼠标左键点击
void GameProc::Click(int x, int y, int flag, HWND hwnd)
{
#if 0
	m_pGame->m_pEmulator->Tap(x, y);
	return;
#endif

	if (!hwnd)
		hwnd = m_hWndGame;

	if (flag & 0x01) {
		::PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
	}
	if (flag & 0x02) {
		::PostMessage(hwnd, WM_LBUTTONUP, 0, MAKELPARAM(x, y));
	}
	Sleep(15);
	//printf("===点击:%d,%d===\n", x, y);
}

// 鼠标左键点击[不包括此帐号]
void GameProc::ClickOther(int x, int y, int ex, int ey, _account_* account_no)
{
	int count = m_pGame->m_pEmulator->List2();
	for (int i = 0; i < count; i++) {
		MNQ* m = m_pGame->m_pEmulator->GetMNQ(i);
		if (!m || !m->Account) // 没有相互绑定
			continue;
		if (m->Account == account_no) // 不包活此帐号
			continue;

		printf("GameProc::ClickOther %s\n", m->Account->Name);
		Click(x, y, ex, ey, 0xff, m->Wnd);
	}
}

// 鼠标左键双击
void GameProc::DBClick(int x, int y, HWND hwnd)
{
	if (!hwnd)
		hwnd = m_hWndGame;

	//::PostMessage(hwnd, WM_LBUTTONDBLCLK, MK_LBUTTON, MAKELPARAM(x, y));
	Click(x, y, 0xff, hwnd);
	Click(x, y, 0xff, hwnd);
}

// 按键
void GameProc::Keyboard(char key, int flag, HWND hwnd)
{
	if (!hwnd)
		hwnd = m_hWndGame;

	if (flag & 0x01)
		::PostMessage(hwnd, WM_KEYDOWN, key, 0);
	if (flag & 0x02)
		::PostMessage(hwnd, WM_KEYUP, key, 0);
}

int GameProc::IsNeedAddLife()
{
	m_pGame->m_pGameData->ReadLife();
	if (m_pGame->m_pGameData->m_dwLife == 0)
		return -1;
	if (m_pGame->m_pGameData->m_dwLife < 8000)
		return 1;
	return 0;
}

// 是否最前窗口
bool GameProc::IsForegroundWindow()
{
	if (!m_pGame->m_pBig->Mnq)
		return false;

	HWND hWndTop = ::GetForegroundWindow();
	return hWndTop == m_pGame->m_pBig->Mnq->WndTop || hWndTop == m_pGame->m_pBig->Mnq->Wnd;
}
