#pragma once
#include <Windows.h>

class Game;
class Talk
{
public:
	Talk(Game* p);

	// ���ж��NPC��һ��ʱ, ����NPC�б�ѡ��
	bool IsNeedCheckNPC();
	// ѡ��NPC
	void SelectNPC(int no);
	// NPC
	DWORD NPC(const char* name=nullptr);
	// NPC�Ի�ѡ����
	void Select(DWORD no);
	// NPC�Ի�״̬[�Ի����Ƿ��]
	bool NPCTalkStatus(int index);
	// �ȴ��Ի����
	bool WaitTalkOpen(int index, DWORD ms=0);
	// �ȴ��Ի���ر�
	bool WaitTalkClose(int index, DWORD ms = 0);
	// �Ի���ť�Ƿ��Ѵ�, ����NPC����ֶԻ���ť
	bool TalkBtnIsOpen();
	// �����������Ƿ��Ѵ�
	bool LianGongChangIsOpen();
	// �ر�����������
	void CloseLianGongChangBox();
	// ���촰���Ƿ��Ѵ�
	bool SpeakIsOpen();
	// �ر����촰��
	void CloseSpeakBox();
	// ȷ����ť�Ƿ��
	bool SureBtnIsOpen();
	// ȷ�ϰ�ť�Ƿ��
	bool ConfirmBtnIsOpen();
	// ����ȷ�ϰ�ťflag=0ȡ�� flag=1ȷ��
	void ClickConfirmBtn(int flag);
	// �ȴ�ȷ�ϰ�ť��
	bool WaitForConfirmBtnOpen(DWORD ms=1000);
	// �ر���������ȡ�����
	void CloseLGCBox();

	// �ȴ��罻��Ϣ���
	bool WaitForSheJiaoBox(DWORD ms = 2000);

	// �Ƿ�����Ϸ���� û����������
	bool IsInGamePic();
	// �ȴ�������Ϸ����
	bool WaitForInGamePic(DWORD ms=15000);
	// �������Ƿ��
	bool CommonBoxIsOpen();
	// �ر�ͨ�ò�
	void CloseCommonBox();

	// �ر������ڵ���
	void CloseAllBox();
public:
	// Game��
	Game* m_pGame;
};