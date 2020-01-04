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

// ��ʼ������
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

// ��ʼ������
bool GameProc::InitSteps()
{
	m_pGameStep->InitGoLeiMingSteps(); // ��ʼ�����ȥ��������
	for (int i = 0; i < m_pGame->m_pGameConf->m_Setting.FBFileCount; i++) {
		m_pGameStep->InitSteps(m_pGame->m_chPath, m_pGame->m_pGameConf->m_Setting.FBFile[i]);
	}
	return m_pGame->m_pGameConf->m_Setting.FBFileCount > 0;
}

// �л���Ϸ����
void GameProc::SwitchGameWnd(HWND hwnd)
{
	m_hWndGame = hwnd;
}

// �л���Ϸ�ʺ�
void GameProc::SwitchGameAccount(_account_ * account)
{
	m_pAccount = account;
}

// ���ȥ������½����
void GameProc::GoLeiMing()
{
	m_pGameStep->ResetStep(0, 0x02);
	while (ExecStep(m_pGameStep->m_GoLeiMingStep)); // �����
}

// ȥ��ȡ����
void GameProc::GoGetXiangLian()
{
}

// ѯ����������
_account_* GameProc::AskXiangLian()
{
	Account* account = nullptr;
	int max = 0;
	m_pGame->m_pEmulator->List2();
	for (int i = 0; i < m_pGame->m_pEmulator->GetCount(); i++) {
		MNQ* m = m_pGame->m_pEmulator->GetMNQ(i);
		if (!m || !m->Account) // û���໥��
			continue;

		if (!m->Account->IsReadXL) { // û�ж�ȡ����
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

		printf("%s������%d�� %08X\n", m->Account->Name, m->Account->XL, m->Wnd);
	}

	return account;
}

// ȥ�����ſ�
void GameProc::GoFBDoor(_account_* account)
{
	if (m_pGame->m_pGameData->IsInFBDoor(account))
		return;
		
	while (true) {
		while (m_bPause) Sleep(500);

		m_pGame->m_pItem->SwitchQuickBar(2);
		Sleep(500);
		printf("%sʹ��ȥ�����ſ��ǳ�֮��\n", account->Name);
		m_pGame->m_pItem->GoFBDoor();
		Sleep(1000);
		if (m_pGame->m_pGameData->IsInFBDoor(account)) {
			Sleep(2000);
			m_pGame->m_pItem->SwitchQuickBar(1);
			break;
		}	
	}
}

// ���븱��
_account_* GameProc::OpenFB()
{
	_account_* account = m_pGame->m_pBig;
	if (!account) {
		printf("û����������, �޷����븱��\n");
		return nullptr;
	}
	printf("%sȥ������[%s], ������������:%d\n", account->Name, account->IsBig?"���":"С��", account->XL);

	GoFBDoor(account);
	int i;
_start_:
	SwitchGameWnd(account->Mnq->Wnd);
	printf("%sȥ�����ſ�\n", account->Name);
	m_pGame->m_pMove->RunEnd(863, 500, account); //872, 495 �������������������
	Sleep(1000);
	printf("%s���������\n", account->Name);

	CloseTipBox();
	Click(800, 323); //800, 323 ���������
	Sleep(500);
	//return account;

	while (m_bPause) Sleep(500);
	if (!m_pGame->m_pTalk->WaitTalkOpen(0x00)) {
		if (m_pGame->m_pTalk->IsNeedCheckNPC()) {
			printf("ѡ���һ��NPC\n");
			m_pGame->m_pTalk->SelectNPC(0x00);
		}
		if (!m_pGame->m_pTalk->WaitTalkOpen(0x00)) {
			printf("NPC�Ի���δ��, �����ٴζԻ�\n");
			goto _start_;
		}
	}
	Sleep(1000);
	m_pGame->m_pTalk->Select(account->IsBig ? 0x00 : 0x01); // �����Կ��, С��������
	Sleep(1000);
	m_pGame->m_pTalk->Select(0x00); // ȷ����������
	Sleep(2000);

	for (i = 0; true; i++) {
		while (m_bPause) Sleep(500);
		if (!m_pGame->m_pGameData->IsInFBDoor()) {
			printf("%s�Ѿ����븱��\n", account->Name);
			break;
		}
		if (i == 10)
			goto _start_;

		printf("%s�ȴ����븱��\n", account->Name);
		Sleep(1000);
	}

	return account;
}

// ��ȡ���������ʺ�
_account_ * GameProc::GetOpenFBAccount()
{
	return IsBigOpenFB() ? m_pGame->GetBigAccount() : AskXiangLian();
}

// �Ƿ��ɴ�ſ�������
bool GameProc::IsBigOpenFB()
{
	SYSTEMTIME stLocal;
	::GetLocalTime(&stLocal);

	if (stLocal.wHour < 20) // 20�㵽24�������ѽ�
		return false;

	return stLocal.wHour == 23 ? stLocal.wMinute < 59 : true; // ����Ҫ1����ʱ��׼��
}

// ִ��
void GameProc::Exec(_account_* account)
{
}

// ����
void GameProc::Run(_account_* account)
{
	SwitchGameWnd(account->Mnq->Wnd);
	SwitchGameAccount(account);
	printf("ˢ�����ʺ�:%s(%08X )\n", account->Name, m_hWndGame);
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

// ȥ�������
void GameProc::GoInTeamPos(_account_* account)
{
	if (!account->Mnq) {
		LOGVARN2(64, "red", "�ʺ�:%sδ��ģ����", account->Name);
		return;
	}

	SwitchGameWnd(account->Mnq->Wnd);
	SwitchGameAccount(account);

	if (m_pGame->m_pGameData->IsInShenDian(account)) // ���뿪���
		GoLeiMing();

	if (!m_pGame->m_pGameData->IsInFBDoor(account)) {
#if 1
		GoFBDoor(account);
#else
		Click(950, 35, 960, 40);     // ��������
		Sleep(2000);
		Click(450, 680, 500, 690);   // ���������ť
		Sleep(1500);
		Click(150, 562, 200, 570);   // ����Ǽ�����
		Sleep(1000);
		Click(150, 510, 200, 520);   // ���������Ŀ���
		Sleep(1000);
		Click(950, 590, 960, 600);   // �����ʼ��ս
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
	if (account->IsBig) { // ��Ŵ�������
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
		//872, 495 ������������
		//425, 235 ���������
	}
	SwitchGameAccount(m_pGame->m_pBig);
}

// ��������
void GameProc::CreateTeam()
{
	printf("��������...\n");
	Click(5, 360, 25, 390);     // ���������
	Sleep(1000);
	Click(80, 345, 200, 360);   // �����������
	Sleep(1500);
	Click(1160, 60, 1166, 66);  // ����رհ�ť
	Sleep(1000);
	Click(5, 463, 10, 466);     // ����������
	printf("�����������\n");
}

// ������� ������� 
void GameProc::ViteInTeam()
{
	printf("׼���������\n");
	Click(766, 235);           // �������ģ��
	Sleep(1000);
	Click(410, 35, 415, 40);   // �������ͷ��
	Sleep(1000);
	Click(886, 236, 900, 245); // ����������
	printf("����������\n");
}

// ���
void GameProc::InTeam(_account_* account)
{
	printf("%s׼��ͬ�����\n", account->Name);
	SwitchGameWnd(account->Mnq->Wnd);
	SetForegroundWindow(account->Mnq->WndTop);
	AgreenMsg("�������ͼ��");
	printf("%s���ͬ�����\n", account->Name);
}

void GameProc::InFB(_account_* account)
{
	printf("%sͬ�������\n", account->Name);
	int max = 10;
	for (int i = 1; i <= max; i++) {
		if (i > 1) {
			printf("%s��(%d/%d)�γ���ͬ�������\n", account->Name, i, max);
		}

		SwitchGameWnd(account->Mnq->Wnd);
		SetForegroundWindow(account->Mnq->WndTop);
		Sleep(500);
		AgreenMsg("������ͼ��");
		Sleep(5000);

		if (IsInFB(account))
			break;
	}
	
	printf("%s���ͬ�������\n", account->Name);
}

// �����˽�����
void GameProc::AllInFB(_account_* account_open)
{
	int count = m_pGame->m_pEmulator->List2();
	for (int i = 0; i < count; i++) {
		MNQ* m = m_pGame->m_pEmulator->GetMNQ(i);
		if (!m || !m->Account) // û���໥��
			continue;
		if (account_open == m->Account)
			continue;

		InFB(m->Account);
	}
}

// ͬ��ϵͳ��Ϣ
void GameProc::AgreenMsg(const char* name, HWND hwnd)
{
	for (int i = 0; i < 3; i++) { // �������濪ʼ��
		if (m_bPause)
			break;

		int result = AgreenMsg(name, i, false, hwnd);
		if (result == 1)
			return;
		if (result == -1)
			break;

		Sleep(1000);
	} 
	AgreenMsg(name, 0, true, hwnd); // �Ҳ���ǿ�Ƶ�һ�µ�һ��
}

// ͬ��ϵͳ��Ϣ
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
		printf("%d.����罻ͼ��(%d)չ����Ϣ\n", i, icon_index);
		Click(x, y, x2, y2); // ����罻��Ϣͼ�� 580
		if (m_pGame->m_pTalk->WaitForSheJiaoBox()) {
			is_open = true;
			break;
		}

		printf("��Ϣ��δ��, �ٴδ�\n");
	}
	if (!is_open)
		return -1;

	Sleep(1000);

	// ��ȡ��Ϣͼ��
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(hwnd ? hwnd : m_hWndGame, 360, 225, 406, 276, 100, true);
	if (!m_pGame->m_pPrintScreen->ComparePixel(name, nullptr, 1)) { // ���������Ϣ
		printf("��Ϣ�򲻷���\n");
		if (!click) {
			printf("�ر�չ������Ϣ��\n");
			Click(933, 166, 935, 170); // �ر�
			return 0;
		}
	}

	printf("������ܻ�ͬ�ⰴť\n");
	Click(835, 475, 850, 490); // ������ܰ�ťͬ��
	return 1;
}

// �Ƿ��ڸ���
bool GameProc::IsInFB(_account_* account)
{
    //return true;
	DWORD x = 0, y = 0;
	m_pGame->m_pGameData->ReadCoor(&x, &y, account);
	if (x >= 882 && x <= 930 && y >= 1055 && y < 1115) // �ս���������
		return true;

	// ��ȡ��ͼ���Ƶ�һ����
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_hWndGame, 1090, 5, 1255, 23, 0, true);
	return m_pGame->m_pPrintScreen->CompareImage(CIN_InFB, nullptr, 1) > 0;
}

// ִ�и�������
void GameProc::ExecInFB()
{
	if (1) {
		printf("ִ�и�������\n");

		InitData();
		m_pGameStep->SelectRandStep();
		//m_pGameStep->ResetStep();
		//SendMsg("��ʼˢ����");
		int start_time = time(nullptr);
		m_nStartFBTime = start_time;
		m_nUpdateFBTimeLongTime = start_time;

#if 0
		/* ֻ�е���˸�����ͼ, �޸�����ſ����Զ�Ѱ· */
		m_pGame->m_pMove->OpenMap(m_pGame->m_pBig);
		Sleep(100);
		Click(250, 600, 255, 605, 0xff, m_pGame->m_pBig->Mnq->Wnd); // �����ͼ��
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
		// �л������ܿ����
		m_pGame->m_pItem->SwitchMagicQuickBar();
		Sleep(500);
		// ����������
		Click(5, 463, 10, 466);

		while (ExecStep(m_pGameStep->m_Step, true)); // ���ˢ����
		m_bLockGoFB = false;

		int end_time = time(nullptr);
		int second = end_time - start_time;
		printf("ִ�и���������ɣ�����ʱ:%02d��%02d��\n", second / 60, second % 60);
		char log[64];
		sprintf_s(log, "���ˢ����������ʱ:%02d��%02d��", second / 60, second % 60);
		//SendMsg(log);
		m_pGameStep->ResetStep();

		m_pGame->UpdateFBCountText(++m_nPlayFBCount, true);
		m_pGame->UpdateFBTimeLongText(end_time - m_nStartFBTime + m_nFBTimeLong, end_time - m_nUpdateFBTimeLongTime); // ����ʱ��
		m_nUpdateFBTimeLongTime = end_time;
		m_nFBTimeLong = end_time - m_nStartFBTime + m_nFBTimeLong;
		//GoFBDoor();
		Sleep(1000);
		while (!m_pGame->m_pGameData->IsInFBDoor(m_pGame->GetBigAccount())) { // �ȴ���������
			Sleep(1000);
		}
		m_pGame->m_pTalk->WaitForInGamePic();
		Sleep(500);

		int wait_s = 60;
		int out_time = time(nullptr);
		SellItem();

		//if (!GetOpenFBAccount()) // û���ʺ���
		//	return;
		while (true) {
			int c = (time(nullptr) - out_time);
			if (c > wait_s)
				break;

			printf("�ȴ�%d���ٴο�������, ��ʣ%d��\n", wait_s, wait_s - c);
			Sleep(1000);
		}

		m_pGame->m_pServer->AskXLCount();
		//Wait((wait_s - (time(nullptr) - out_time)) * 1000);
	}
}

// ִ������
bool GameProc::ExecStep(Link<_step_*>& link, bool isfb)
{
	m_pStep = m_pGameStep->Current(link);
	if (!m_pStep) {
		return false;
	}

	// ���ÿ���̨����
	m_pGameStep->SetConsoleTle(m_pStep->Cmd);
	_step_* m_pTmpStep = m_pStep; // ��ʱ��

	if (isfb) {
		int now_time = time(nullptr);
		m_pGame->UpdateFBTimeLongText(now_time - m_nStartFBTime + m_nFBTimeLong, now_time - m_nUpdateFBTimeLongTime); // ����ʱ��
		m_nUpdateFBTimeLongTime = now_time;

		if (m_pStepCopy) { // �Ѿ�ִ�е��Ĳ���
			if (m_pStep == m_pStepCopy) {
				m_pStepCopy = nullptr;
			}
			else {
				if (m_pStep->OpCode != OP_MOVE && m_pStep->OpCode != OP_MAGIC) { // �����ƶ���ż���
					return m_pGameStep->CompleteExec(link) != nullptr;
				}
			}
		}

		bool check_box = true;

		if (m_pStep->OpCode == OP_MOVE || m_pStep->OpCode == OP_NPC || m_pStep->OpCode == OP_WAIT
			|| m_pStep->OpCode == OP_PICKUP || m_pStep->OpCode == OP_KAIRUI) {
			CloseTipBox(); // �رյ�����
			CloseSystemViteBox(); // �ر�ϵͳ�����

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
		printf("����->�ƶ�:%d.%d��%d.%d\n", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		if (m_pStep->X2 && m_pStep->Y2) {
			m_pStep->X = MyRand(m_pStep->X, m_pStep->X2);
			m_pStep->Y = MyRand(m_pStep->Y, m_pStep->Y2);

			m_pStep->X2 = 0;
			m_pStep->Y2 = 0;

			printf("ʵ���ƶ�λ��:%d,%d\n", m_pStep->X, m_pStep->Y);
		}
		Move(true);
		break;
	case OP_MOVEFAR:
		printf("����->����:%d.%d\n", m_pStep->X, m_pStep->Y);
		SMSG_DP(msg, "����->����.%d,%d", m_pStep->X, m_pStep->Y);
		Move();
		break;
	case OP_MOVERAND:
		printf("����->����ƶ�:%d.%d��%d.%d\n", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		if (MyRand(1, 2, MyRand(m_pStep->X, m_pStep->Y)) == 1) {
			if (m_pStep->X2 && m_pStep->Y2) {
				m_pStep->X = MyRand(m_pStep->X, m_pStep->X2);
				m_pStep->Y = MyRand(m_pStep->Y, m_pStep->Y2);

				m_pStep->X2 = 0;
				m_pStep->Y2 = 0;

				printf("ʵ���ƶ�λ��:%d,%d\n", m_pStep->X, m_pStep->Y);
				Move(true);
			}
		}
		else {
			printf("����ƶ�����\n\n");
			bk = true;
		}
		break;
	case OP_NPC:
		printf("����->NPC:%s(���:%d,%d �� %d,%d)\n", m_pStep->NPCName, m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		//SMSG_DP(msg, "����->NPC.%s ����{%d,%d}", m_pStep->NPCName, m_stLastStepInfo.MvX, m_stLastStepInfo.MvY);
		m_stLast.NPCOpCode = OP_NPC;
		NPC();
		break;
	case OP_SELECT:
		printf("����->ѡ��:%d\n", m_pStep->SelectNo);
		SMSG_DP(msg, "����->ѡ��:%d", m_pStep->SelectNo);
		Select();
		break;
	case OP_MAGIC:
		printf("����->����.%s ʹ�ô���:%d\n", m_pStep->Magic, m_pStep->OpCount);
		Magic();
		break;
	case OP_MAGIC_PET:
		printf("����->���＼��:%s\n", m_pStep->Magic);
		SMSG_DP(msg, "����->���＼��:%s", m_pStep->Magic);
		//MagicPet();
		break;
	case OP_KAWEI:
		printf("��λ->ʹ�ü���:%s �ȴ�:%d��\n", m_pStep->Magic, m_pStep->WaitMs / 1000);
		KaWei();
		break;
	case OP_CRAZY:
		SMSG_D("����->���");
		//Crazy();
		break;
	case OP_CLEAR:
		SMSG_D("����->����");
		//Clear();
		break;
	case OP_KAIRUI:
		printf("����->���� �ƶ�:(%d,%d) ����:(%d,%d)-(%d,%d)\n", m_pStep->X, m_pStep->Y, 
			m_pStep->Extra[0], m_pStep->Extra[1], m_pStep->Extra[2], m_pStep->Extra[3]);
		KaiRui();
		break;
	case OP_PICKUP:
		printf("����->��ʰ��Ʒ:%s, ����:(%d,%d)-(%d,%d)\n", m_pStep->Name, m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		PickUp();
		break;
	case OP_CHECKIN:
		printf("����->������Ʒ\n");
		SMSG_D("����->������Ʒ");
		CheckIn();
		break;
	case OP_USEITEM:
		printf("����->ʹ����Ʒ\n");
		SMSG_D("����->ʹ����Ʒ");
		//UseItem();
		break;
	case OP_DROPITEM:
		printf("����->������Ʒ:%s\n", m_pStep->Name);
		SMSG_DP(msg, "����->������Ʒ:%s", m_pStep->Name);
		DropItem();
		break;
	case OP_SELL:
		printf("����->������Ʒ\n");
		SMSG_D("����->������Ʒ");
		//SellItem();
		break;
	case OP_BUTTON:
		printf("����->ȷ��\n");
		Button();
		break;
	case OP_CLICK:
		m_stLast.NPCOpCode = OP_CLICK;
		m_stLast.ClickX = m_pStep->X;
		m_stLast.ClickY = m_pStep->Y;
		m_stLast.ClickX2 = 0;
		m_stLast.ClickY2 = 0;
		printf("����->���:%d,%d %d,%d\n", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
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
		printf("������:%d,%d %d,%d ����:%d\n", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2, m_pStep->OpCount);
		ClickRand();
		break;
	case OP_CLICKCRAZ:
		printf("�������:%d,%d %d,%d ����:%d\n", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2, m_pStep->OpCount);
		m_ClickCrazy.X = m_pStep->X;
		m_ClickCrazy.Y = m_pStep->Y;
		m_ClickCrazy.X2 = m_pStep->X2;
		m_ClickCrazy.Y2 = m_pStep->Y2;
		m_ClickCrazy.Count = m_pStep->OpCount;
		bk = true;
		break;
	case OP_WAIT:
		printf("����->�ȴ�:%d %s %d\n", m_pStep->WaitMs / 1000, m_pStep->Magic, m_pStep->Extra[0]);
		Wait();
		break;
	case OP_SMALL:
		if (m_pStep->SmallV == -1) {
			printf("����->С��:δ֪����\n");
		}
		else {
			printf("����->С��:%d %s\n", m_pStep->SmallV, m_pStep->SmallV == 0 ? "������" : "������");
			Small();
		}
		break;
	case OP_RECORD:
		printf("\n--------׼����¼��һ����--------\n\n");
		m_bIsRecordStep = true;
		bk = true;
		break;
	default:
		SMSG_D("����->δ֪");
		break;
	}

	SMSG_D("����->ִ�����");
	if (bk) {
		return m_pGameStep->CompleteExec(link) != nullptr;
	}
	if (m_bIsResetRecordStep) {
		printf("\n--------���õ��Ѽ�¼����------\n\n");
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
				CloseTipBox(); // �رյ�����
			}
			if (mov_i > 0 && (mov_i % 20) == 0) {
				m_pGame->m_pItem->GetQuickYaoOrBaoNum(m_nYaoBao, m_nYao);
				if (m_bIsFirstMove) {
					m_pGame->m_pItem->SwitchMagicQuickBar(); // �л������ܿ����
				}
			}
			if (mov_i > 10 && m_nReMoveCount == 0) {
				if ((GetTickCount() & 0xbf) == 0) {
					Click(298, 40, 327, 68);
				}
			}

			if ((mov_i % 5) == 0) {
				// ����·����ҩ
				if (m_nYaoBao > m_nLiveYaoBao) { // ҩ������6
					if (m_nYao < 2) { // ҩС��2
						m_pGame->m_pItem->UseYaoBao(); // ��ҩ��
						m_nYaoBao--;
						m_nYao += 6;
					}
					else {
						m_pGame->m_pItem->UseYao(1, false, 100); // ��ҩ
						m_nYao--;
					}

					use_yao_bao = true;
				}
				else {
					if (m_nYao > m_nLiveYao) { // ҩ����6
						m_pGame->m_pItem->UseYao(1, false, 100);
						m_nYao--;

						use_yao_bao = true;
					}
				}
			}

			// ���������
			if (m_ClickCrazy.Count) {
				ClickCrazy();
			}
		}

		if (m_pStep->OpCode != OP_DROPITEM) {
			SMSG_D("�жϼ�Ѫ");
			int life_flag = IsNeedAddLife();
			if (life_flag == 1) { // ��Ҫ��Ѫ
				SMSG_D("��Ѫ");
				m_pGame->m_pItem->UseYao();
				SMSG_D("��Ѫ->���");
			}
			else if (life_flag == -1) { // ��Ҫ����
				ReBorn(); // ����
				m_pStepCopy = m_pStep; // ���浱ǰִ�и���
				m_pGameStep->ResetStep(0); // ���õ���һ��
				printf("���õ���һ��\n");
				return true;
			}
			SMSG_D("�жϼ�Ѫ->���");
		}

		//SMSG_D("�ж�����");
		bool complete = StepIsComplete();
		//SMSG_D("�ж�����->���");
		if (m_pStep->OpCode == OP_MOVEFAR) {
			if (++move_far_i == 300) {
				printf("���ͳ�ʱ\n");
				complete = true;
			}
		}

		if (m_nReMoveCount > 20) {
			printf("--------�����ѿ�ס--------\n");
			if (m_nReMoveCount < 25 && (m_stLast.OpCode == OP_NPC || m_stLast.OpCode == OP_SELECT)) {
				printf("--------�����ٴζԻ�NPC--------\n");
				NPCLast(1);
				m_pGame->m_pTalk->WaitTalkOpen(0x00);
				Sleep(1000);
				m_pGame->m_pTalk->Select(0x00);
				Sleep(500);
				printf("--------���ԶԻ�NPC��ɣ���Ҫ�ƶ���:(%d,%d)-------\n", m_pStep->X, m_pStep->Y);
				m_nReMoveCount++;
			}
			printf("--------%d�κ����õ���¼����--------\n", 25 - m_nReMoveCount);
			if (m_nReMoveCount >= 25 && m_pStepRecord) {
				printf("--------���õ��Ѽ�¼����------\n");
				m_pGameStep->ResetStep(m_pStepRecord->Index, 0x01);
				m_nReMoveCount = 0;
				return true;
			}
		}

		if (complete) { // ����ɴ˲���
			printf("����->����ɴ˲���\n\n");
			if (m_bIsRecordStep) { // ��¼�˲���
				m_pStepRecord = m_pStep;
				m_bIsRecordStep = false;
				printf("--------�Ѽ�¼�˲���--------\n\n");
			}

			m_stLast.OpCode = m_pStep->OpCode;
			m_pStepLast = m_pStep;
			
			if (m_pStep->OpCode != OP_NPC && m_pStep->OpCode != OP_SELECT && m_pStep->OpCode != OP_WAIT) {
				//if (m_pGame->m_pTalk->NPCTalkStatus()) // �Ի����Ǵ򿪵�
				//	m_pGame->m_pTalk->NPCTalk(0xff);   // �ص���
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

// �����Ƿ���ִ�����
bool GameProc::StepIsComplete()
{
	bool result = false;
	switch (m_pStep->OpCode)
	{
	case OP_MOVE:
	case OP_MOVERAND:
		//result = true;
		SMSG_D("�ж��Ƿ��ƶ����յ�&�Ƿ��ƶ�", true);
		if (m_pGame->m_pMove->IsMoveEnd(m_pAccount)) { // �ѵ�ָ��λ��
			m_stLast.MvX = m_pStep->X;
			m_stLast.MvY = m_pStep->Y;
			m_nReMoveCount = 0;
			result = true;

			if (m_bIsFirstMove) {
				// �л������ܿ����
				m_pGame->m_pItem->SwitchMagicQuickBar();
				m_bIsFirstMove = false;
			}
			goto end;
		}
		SMSG_D("�ж��Ƿ��ƶ����յ�->���", true);
		if (!m_pGame->m_pMove->IsMove(m_pAccount)) {   // �Ѿ�ֹͣ�ƶ�
			Move(false);
			m_nReMoveCount++;
		}
		SMSG_D("�ж��Ƿ��ƶ�->���", true);
		break;
	case OP_MOVEFAR:
		if (m_pGame->m_pGameData->IsNotInArea(m_pStep->X, m_pStep->Y, 50, m_pAccount)) { // �Ѳ��ڴ�����
			result = true;
			m_nReMoveCount = 0;
			goto end;
		}
		if (m_pGame->m_pMove->IsMoveEnd(m_pAccount)) { // �ѵ�ָ��λ��
			m_pGame->m_pMove->Run(m_pStep->X - 5, m_pStep->Y, m_pAccount, m_pStep->X2, m_pStep->Y2);
			m_nReMoveCount = 0;
		}
		if (!m_pGame->m_pMove->IsMove(m_pAccount)) {   // �Ѿ�ֹͣ�ƶ�
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

// �ƶ�
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

// �Ի�
void GameProc::NPC()
{
	if (strcmp("��Ѫ��ʿ", m_pStep->NPCName) == 0) {
		m_pGame->m_pItem->SwitchMagicQuickBar();
	}
	else if (strcmp("��ħ������̳", m_pStep->NPCName) == 0) {
		m_nLiveYaoBao = 2;
		m_nLiveYao = 1;
	}
	else if (strcmp("������", m_pStep->NPCName) == 0) {
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

// NPC���
void GameProc::NPC(const char* name, int x, int y, int x2, int y2)
{
	CloseItemUseTipBox();

	bool click_btn = true;
	if (name && strstr(name, "֮��"))
		click_btn = false;

	if (click_btn && m_pGame->m_pTalk->TalkBtnIsOpen()) {
		printf("����Ի���ť\n");
		m_pGame->m_pTalk->NPC();
		Sleep(100);
	}
	else {
		if (!x || !y) { // û��д����
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
		printf("���NPC:%d,%d\n", x, y);
	}
}

// ���һ���Ի���NPC
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

				printf("�����ƶ���:(%d,%d) ����λ��:(%d,%d)\n", m_stLast.MvX, m_stLast.MvY, pos_x, pos_y);
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
		printf("�ٴε���Ի���NPC(%s)\n", m_stLast.NPCName);
		NPC(m_stLast.NPCName, m_stLast.ClickX, m_stLast.ClickY, m_stLast.ClickX2, m_stLast.ClickY2);
	}
	else if (m_stLast.NPCOpCode == OP_CLICK) {
		if (0 && m_stLast.ClickX2 && m_stLast.ClickY2) {
			Click(m_stLast.ClickX, m_stLast.ClickY);
			printf("�ٴε��(%d,%d)-(%d,%d)��NPC(%s)\n", m_stLast.ClickX, m_stLast.ClickY, m_stLast.ClickX2, m_stLast.ClickY2, m_stLast.NPCName);
		}
		else {
			Click(m_stLast.ClickX, m_stLast.ClickY);
			printf("�ٴε��(%d,%d)��NPC(%s)\n", m_stLast.ClickX, m_stLast.ClickY, m_stLast.NPCName);
		}
	}

	return is_move;
}

// ѡ��
void GameProc::Select()
{
	if (strcmp("������", m_stLast.NPCName) == 0) {
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

				bool check_pos = i < 6 && strcmp("������", m_stLast.NPCName) != 0;
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

			printf("�ȴ��Ի���򿪳���%d��,����. ���:%d\n", max_j, _clk);
			goto _check_;
		}

		Sleep(MyRand(300, 350, i));

		printf("��(%d)��ѡ�� ѡ��:%d\n", i, m_pStep->SelectNo);
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
		printf("����Ѫ\n");
		m_pGame->m_pItem->AddFullLife();
	}

_check_:
	if (m_pStep->OpCount == 1 || m_pStep->OpCount >= 10) {
		if (strcmp(m_stLast.NPCName, "����ʿ��̳") == 0 || strcmp(m_stLast.NPCName, "Ů������̳") == 0) {
			Sleep(500);
			if (CloseTipBox()) {
				printf("--------�з�ӡδ�⿪����Ҫ���õ���¼����--------\n");
				m_bIsResetRecordStep = true;
				return;
			}
			else {
				printf("--------��ӡ��ȫ���⿪--------\n");
			}
		}

		if (IsCheckNPC(m_stLast.NPCName)) {
			printf("���������NPC:%s������\n", m_stLast.NPCName);
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

				printf("%d.NPCδ�Ի����, ���µ���Ի���ť\n", i);
				NPCLast(true);
				if (m_pGame->m_pTalk->WaitTalkOpen(m_pStep->SelectNo)) {
					Sleep(MyRand(300, 350, i));
					if (i >= 3 && m_pStep->OpCount >= 10) {
						no = 0x00;
						i--;
					}

					printf("%d.ѡ��:%d\n", i, no);
					m_pGame->m_pTalk->Select(no);
					m_pGame->m_pTalk->WaitTalkClose(m_pStep->SelectNo);

					if (strcmp(m_stLast.NPCName, "����ʿ��̳") == 0 || strcmp(m_stLast.NPCName, "Ů������̳") == 0) {
						Sleep(500);
						if (CloseTipBox()) {
							printf("--------�з�ӡδ�⿪����Ҫ���õ���¼����--------\n");
							m_bIsResetRecordStep = true;
							return;
						}
						else {
							printf("--------��ӡ��ȫ���⿪--------\n");
						}
					}
				}
			}
		}
		else if (IsCheckNPCTipBox(m_stLast.NPCName)) {
			printf("���������NPC����:%s������\n", m_stLast.NPCName);
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

				printf("%d.δ���ֵ���, ���µ���Ի���ť\n", i);
				NPCLast(true);
				if (m_pGame->m_pTalk->WaitTalkOpen(m_pStep->SelectNo)) {
					Sleep(MyRand(300, 350, i));
					m_pGame->m_pTalk->Select(m_pStep->SelectNo);
					m_pGame->m_pTalk->WaitTalkClose(m_pStep->SelectNo);
				}
			}
			if (!result && strcmp("�������Թ��", m_stLast.NPCName) == 0) { // ���BOSSδ���ӡ
				printf("--------�з�ӡδ�⿪����Ҫ���õ���¼����--------\n");
				m_bIsResetRecordStep = true;
				return;
			}
		}
	}
}

// ����
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

// ��λ
void GameProc::KaWei()
{
	int click_x = 0, click_y = 0;
	m_pGame->m_pMagic->GetMagicClickPos(m_pStep->Magic, click_x, click_y);
	if (click_x && click_y) {
		int mv_x = m_pStep->X, mv_y = m_pStep->Y;
		printf("ʹ�ü���:%s(%d,%d) ����(%d,%d)\n", m_pStep->Name, click_x, click_y, mv_x, mv_y);
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
				printf("��λ�ȴ�%02d�룬��ʣ%02d��.\n", m_pStep->WaitMs / 1000, ls);
			}
			n = ls;

			m_pGame->m_pGameProc->MouseMove(click_x + mv_x, click_y + mv_y);
			Sleep(500);
		}
		m_pGame->m_pGameProc->Click(click_x + mv_x, click_y + mv_y, 0x02);
	}
	else {
		printf("KaWei��%d,%d\n", click_x, click_y);
	}
}

// ����
void GameProc::KaiRui()
{
	if (m_bClearKaiRui) {
		printf("========�����ѱ����������========\n");
		return;
	}
	// �ƶ���ָ��λ��
	m_pGame->m_pMove->RunEnd(m_pStep->X, m_pStep->Y, m_pAccount);
	CloseTipBox();
	Sleep(100);
	// ��ȡ����ȷ����ťͼƬ
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_hWndGame, 
		m_pStep->Extra[0], m_pStep->Extra[1], m_pStep->Extra[2], m_pStep->Extra[3], 0, true);

	int red_count = m_pGame->m_pPrintScreen->GetPixelCount(0xffff0000, 0x00101010);
	int yellow_count = m_pGame->m_pPrintScreen->GetPixelCount(0xffffff00, 0x00020202);
	printf("��ɫ: ��ɫ(%d), ��ɫ(%d)\n", red_count, yellow_count);
	if ((red_count > 10 && yellow_count > 50) || yellow_count > 200) {
		printf("========���ֿ���========\n");
		int max = m_pStep->Extra[4] ? 1 : 5;
		for (int i = 1; i <= max; i++) {
			printf("========ʹ�ü���->����þ�========\n");
			m_pGame->m_pMagic->UseMagic("����þ�");
			if (IsNeedAddLife() == -1)
				m_pGame->m_pItem->UseYao(2);

			Sleep(500);

			if (i & 0x01) {
				int pickup_count = m_pGame->m_pItem->PickUpItem("��Чʥ����ҩ", m_pStep->Extra[0], m_pStep->Extra[1], m_pStep->Extra[2], m_pStep->Extra[3], 2);
				if (pickup_count > 0) {
					if (pickup_count == 1) { // ����������, ��ɨ��һ��
						m_pGame->m_pItem->PickUpItem("��Чʥ����ҩ", 450, 300, 780, 475, 2);
					}
					break;
				}
			}
		}
		m_bClearKaiRui = true;
	}
	else if (m_pStep->Extra[4]) { // ɨ��ʥ����Ʒ
		int pickup_count = m_pGame->m_pItem->PickUpItem("��Чʥ����ҩ", m_pStep->Extra[0], m_pStep->Extra[1], m_pStep->Extra[2], m_pStep->Extra[3], 2);
		if (pickup_count > 0) {
			printf("========���ֿ���, ���ѱ����========\n");
			m_bClearKaiRui = true;
		}
	}
}

// ����
void GameProc::PickUp()
{
	if (CloseTipBox())
		Sleep(500);

	bool to_big = false; // �Ƿ�Ŵ���Ļ
	if (strcmp(m_pStep->Name, "30��������Ƭ+3") == 0)
		to_big = true;

	if (to_big) {
		MouseWheel(240);
		Sleep(1000);
	}

	int pickup_count = m_pGame->m_pItem->PickUpItem(m_pStep->Name, m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);

	if (strcmp(m_pStep->Name, "30��������Ƭ+3") == 0) {
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

// ����
DWORD GameProc::CheckIn(bool in)
{
	CloseTipBox(); // �رյ�����
	m_pGame->m_pItem->CheckIn(m_pGame->m_pGameConf->m_stCheckIn.CheckIns, m_pGame->m_pGameConf->m_stCheckIn.Length);
	return 0;
}

// ʹ����
void GameProc::UseItem()
{
}

// ����
void GameProc::DropItem()
{
	CloseTipBox(); // �رյ�����
	m_pGame->m_pItem->DropItem(CIN_YaoBao);
}

// ������Ʒ
void GameProc::SellItem()
{
	printf("\n-----------------------------\n");
	printf("׼��ȥ������\n");
	int pos_x = 265, pos_y = 370;
	DWORD _tm = GetTickCount();
	if (!m_pGame->m_pGameData->IsInArea(pos_x, pos_y, 15)) { // �����̵�����
		int i = 0;
	use_pos_item:
		printf("(%d)�л������\n", i + 1);
		m_pGame->m_pItem->SwitchQuickBar(2); // �л������
		Sleep(500);
		printf("(%d)ʹ���ǳ�֮��\n", i + 1);
		m_pGame->m_pItem->GoShop();         // ȥ�̵��Ա�
		Sleep(500);
		for (; i < 100;) {
			if (i > 0 && (i % 10) == 0) {
				i++;
				goto use_pos_item;
			}
			if (m_pGame->m_pGameData->IsInArea(pos_x, pos_y, 15)) { // �����̵��Ա�
				Sleep(100);
				break;
			}

			i++;
			Sleep(1000);
		}
		m_pGame->m_pItem->SwitchQuickBar(1); // �лؿ����
		Sleep(300);
	}

	m_pGame->m_pMove->RunEnd(pos_x, pos_y, m_pGame->m_pBig); // �ƶ����̶���õ��
	//Sleep(500);
	//MouseWheel(-260);
	//m_pGame->m_pMove->RunEnd(pos_x, pos_y, m_pGame->m_pBig); // �ƶ����̶���õ��
	Sleep(1000);

	int rand_v = GetTickCount() % 2;
	int clk_x, clk_y, clk_x2, clk_y2;
	if (rand_v == 0) { // װ����
		clk_x = 563, clk_y = 545;
		clk_x2 = 565, clk_y2 = 546;
	}
	else { // ������
		clk_x = 306, clk_y = 450;
		clk_x2 = 335, clk_y2 = 500;
	}
	Click(clk_x, clk_y, clk_x2, clk_y2);      // �Ի��̵�����
	m_pGame->m_pTalk->WaitTalkOpen(0x00);
	Sleep(1000);
	m_pGame->m_pTalk->Select(0x00); // ������Ʒ
	Sleep(1500);

	m_pGame->m_pItem->SellItem(m_pGame->m_pGameConf->m_stSell.Sells, m_pGame->m_pGameConf->m_stSell.Length);
	Sleep(500);
	m_pGame->m_pItem->SetBag();
	Sleep(500);
	m_pGame->m_pItem->CloseShop();
	Sleep(500);
	if (m_pGame->m_pItem->CheckOut(m_pGame->m_pGameConf->m_stSell.Sells, m_pGame->m_pGameConf->m_stSell.Length)) {
		Sleep(500);
		m_pGame->m_pMove->RunEnd(pos_x, pos_y, m_pGame->m_pBig); // �ƶ����̶���õ��
		Sleep(500);
		Click(clk_x, clk_y, clk_x2, clk_y2);      // �Ի��̵�����
		Sleep(1000);
		m_pGame->m_pTalk->Select(0x00); // ������Ʒ
		m_pGame->m_pTalk->WaitTalkOpen(0x00);
		Sleep(1000);
		m_pGame->m_pItem->SellItem(m_pGame->m_pGameConf->m_stSell.Sells, m_pGame->m_pGameConf->m_stSell.Length);
		Sleep(500);
#if 1
		printf("����������װ��\n");
		Click(8, 260, 35, 358); // �������װ��
		Sleep(1000);
		printf("���ȫ������\n");
		Click(390, 645, 500, 666); // ��ťȫ������
		if (m_pGame->m_pTalk->WaitForConfirmBtnOpen(1500)) {
			Sleep(150);
			printf("ȷ������װ��\n");
			m_pGame->m_pTalk->ClickConfirmBtn(1); // ȷ��
			Sleep(500);
			Click(16, 150, 20, 186); // չ�������б�
			Sleep(350);
		}
		else {
			printf("��������װ��\n");
		}
#endif
		m_pGame->m_pItem->CloseShop();
		Sleep(500);
	}

	m_pGame->m_pTalk->CloseAllBox();

	_tm = GetTickCount() - _tm;
	printf("��������ʱ%.2f��, %d����\n", (float)_tm / 1000.0f, _tm);
	printf("\n-----------------------------\n");
}

// ��ť
void GameProc::Button()
{
	// ȷ����ť 597,445 606,450
	CloseTipBox();
}

// �رյ���
bool GameProc::CloseTipBox()
{
	// ��ȡ����ȷ����ťͼƬ
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_hWndGame, 585, 450, 600, 465, 0, true);
	if (m_pGame->m_pPrintScreen->ComparePixel("��ʾ��", nullptr, 1)) {
		printf("�ر���ʾ��\n");
		Click(576, 440, 695, 468);
		Sleep(100);

		return true;
	}

	return false;
}

// �ر���Ʒʹ����ʾ��
bool GameProc::CloseItemUseTipBox()
{
	// ��ȡ��Ʒʹ����ʾ��ťǰ��ͼƬ
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_hWndGame, 855, 376, 865, 386, 0, true);
	if (m_pGame->m_pPrintScreen->ComparePixel("��Ʒʹ����ʾ��", nullptr, 1)) {
		printf("�ر���Ʒʹ����ʾ��\n");
		Click(1032, 180);
		Sleep(300);

		return true;
	}

	return false;
}

// �ر�ϵͳ������ʾ��
bool GameProc::CloseSystemViteBox()
{
	// ��ȡ��������������ɫ
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_hWndGame, 765, 187, 775, 197, 0, true);
	if (m_pGame->m_pPrintScreen->ComparePixel("����ȷ��", nullptr, 1)) {
		printf("�ر���Ʒʹ����ʾ��\n");
		Click(450, 435, 550, 450);
		Sleep(300);

		return true;
	}

	return false;
}

// ������
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
		printf("������:%d,%d\n", x, y);
		Sleep(MyRand(300, 1000, i * 20));
	}
}

// �񱩵��
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
		printf("�񱩵��:%d,%d ��ʣ%d��\n", x, y, m_ClickCrazy.Count - 1);
		Sleep(MyRand(300, 500, m_ClickCrazy.Count));
	}
	m_ClickCrazy.Count--;
}

// �ȴ�
void GameProc::Wait()
{
	Wait(m_pStep->WaitMs, m_pStep->Extra[0]);
}

// �ȴ�
void GameProc::Wait(DWORD ms, int no_open)
{
	if (ms < 1000) {
		printf("�ȴ�%d����\n", ms);
		Sleep(ms);
		return;
	}

	if (ms >= 2000 && !no_open) {
		DWORD start_ms = GetTickCount();
		if ((start_ms & 0x0f) < 3) {
			Click(16, 26, 66, 80); // ���ͷ��
			Sleep(1500);
		
			int click_count = MyRand(0, 2 + (ms / 1000 / 3), ms);
			for (int i = 0; i < click_count; i++) { // ���Ҳ���һ��
				Click(315, 100, 1160, 500);
				Sleep(MyRand(500, 1000, i));
			}

			Click(1155, 55, 1160, 60); // �ر�
			Sleep(500);

			DWORD use_ms = GetTickCount() - start_ms;
			if (use_ms > ms)
				ms = 0;
			else
				ms -= use_ms;
		}
	}

	if (ms >= 12000 && IsNeedAddLife() != -1) {
		printf("�ȴ�ʱ��ﵽ%d��(%d����), ��������\n", ms / 1000, ms);
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
			printf("�ȴ�%02d�룬��ʣ%02d��.\n", ms / 1000, ls);
		}
		n = ls;
		Sleep(100);
	}
}

// С��
void GameProc::Small()
{
#if 1
	if (m_pStep->SmallV == 0) { // ������
		m_pGame->m_pServer->SmallOutFB(m_pGame->m_pBig, nullptr, 0);
	}
	if (m_pStep->SmallV == 1) { // ������
		m_pGame->m_pServer->SmallInFB(m_pGame->m_pBig, nullptr, 0);
	}
#else
	if (m_pStep->SmallV == 0) { // ������
		Click(597, 445, 606, 450); // �ر���ʾ��
		Sleep(500);
		Click(597, 445, 606, 450); // �����������
		Sleep(2000);

		int count = m_pGame->m_pEmulator->List2();
		for (int i = 0; i < count; i++) {
			MNQ* m = m_pGame->m_pEmulator->GetMNQ(i);
			if (m && m->Account && !m->Account->IsBig) {
				SwitchGameWnd(m->Wnd);
				SetForegroundWindow(m->WndTop);
				Sleep(100);
				m_pGame->m_pMove->Run(890, 1100, 0, 0, false, m->Account); // �ƶ����̶��ص�
			}
		}
		Sleep(1000);

		ClickOther(190, 503, 195, 505, m_pGame->m_pBig); // ���NPC
		Sleep(2000);
		ClickOther(67, 360, 260, 393, m_pGame->m_pBig);  // ѡ��0 ������
		Sleep(1000);
		ClickOther(67, 360, 260, 393, m_pGame->m_pBig);  // ѡ��0 ��Ҫ�뿪����

		SwitchGameWnd(m_pGame->m_pBig->Mnq->Wnd);
		SetForegroundWindow(m_pGame->m_pBig->Mnq->WndTop);
	}
#endif
}

// ����
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
		Click(537, 415, 550, 425); // �㸴��
		Sleep(2000);

		if (IsNeedAddLife() != -1)
			break;
	}

	Wait(5000);
	m_pGame->m_pPet->PetOut(-1);
	Sleep(1000);
}

// �Ƿ����NPC�Ի����
bool GameProc::IsCheckNPC(const char* name)
{
	return strcmp(name, "ħ���ϱ�") == 0 || strcmp(name, "�������ϱ�") == 0 || strcmp(name, "�����ϱ�") == 0
		|| strcmp(name, "��֮������") == 0 || strcmp(name, "Թ֮������") == 0 || strcmp(name, "��Ѫ��ʿ") == 0
		|| strcmp(name, "����ʿ��̳") == 0 || strcmp(name, "Ů������̳") == 0
		|| strcmp(name, "����֮��") == 0
		|| strstr(name, "Ԫ���ϱ�") != nullptr;
}

// �Ƿ����NPC�Ի����
bool GameProc::IsCheckNPCTipBox(const char* name)
{
	return strstr(name, "��ӡ����") != nullptr
		|| strstr(name, "ͼ��") != nullptr
		|| strstr(name, "ӡ��") != nullptr
		|| strcmp(name, "��ħ������̳") == 0
		|| strcmp(name, "���ȵ����") == 0
		|| strcmp(name, "����֮��") == 0
		|| strcmp(name, "����֮��") == 0
		|| strcmp(name, "�������Թ��") == 0;
}


// ����ƶ�[�����x��y�ƶ�rx��ry����]
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

// ����ƶ�
void GameProc::MouseMove(int x, int y, HWND hwnd)
{
	if (!hwnd)
		hwnd = m_hWndGame;

	::SendMessage(hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x, y));
	//printf("����ƶ�:%d,%d\n", x, y);
}

// ������
void GameProc::MouseWheel(int x, int y, int z, HWND hwnd)
{
	if (!hwnd)
		hwnd = m_hWndGame;

	::SendMessage(hwnd, WM_MOUSEWHEEL, MAKEWPARAM(MK_CONTROL, z), MAKELPARAM(x, y));
}

// ������
void GameProc::MouseWheel(int z, HWND hwnd)
{
	if (!hwnd)
		hwnd = m_hWndGame;

	RECT rect;
	::GetWindowRect(hwnd, &rect);
	MouseWheel(MyRand(rect.left+500, rect.left+800), MyRand(rect.top+236, rect.top+500), z, hwnd);

}

// ���������
void GameProc::Click(int x, int y, int ex, int ey, int flag, HWND hwnd)
{
	Click(MyRand(x, ex), MyRand(y, ey), flag, hwnd);
}

// ���������
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
	//printf("===���:%d,%d===\n", x, y);
}

// ���������[���������ʺ�]
void GameProc::ClickOther(int x, int y, int ex, int ey, _account_* account_no)
{
	int count = m_pGame->m_pEmulator->List2();
	for (int i = 0; i < count; i++) {
		MNQ* m = m_pGame->m_pEmulator->GetMNQ(i);
		if (!m || !m->Account) // û���໥��
			continue;
		if (m->Account == account_no) // ��������ʺ�
			continue;

		printf("GameProc::ClickOther %s\n", m->Account->Name);
		Click(x, y, ex, ey, 0xff, m->Wnd);
	}
}

// ������˫��
void GameProc::DBClick(int x, int y, HWND hwnd)
{
	if (!hwnd)
		hwnd = m_hWndGame;

	//::PostMessage(hwnd, WM_LBUTTONDBLCLK, MK_LBUTTON, MAKELPARAM(x, y));
	Click(x, y, 0xff, hwnd);
	Click(x, y, 0xff, hwnd);
}

// ����
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

// �Ƿ���ǰ����
bool GameProc::IsForegroundWindow()
{
	if (!m_pGame->m_pBig->Mnq)
		return false;

	HWND hWndTop = ::GetForegroundWindow();
	return hWndTop == m_pGame->m_pBig->Mnq->WndTop || hWndTop == m_pGame->m_pBig->Mnq->Wnd;
}
