#pragma once
#include <Windows.h>

class Game;
class Talk
{
public:
	Talk(Game* p);

	// 当有多个NPC在一起时, 会有NPC列表选择
	bool IsNeedCheckNPC();
	// 选择NPC
	void SelectNPC(int no);
	// NPC
	DWORD NPC(const char* name=nullptr);
	// NPC对话选择项
	void Select(DWORD no);
	// NPC对话状态[对话框是否打开]
	bool NPCTalkStatus(int index);
	// 等待对话框打开
	bool WaitTalkOpen(int index, DWORD ms=0);
	// 等待对话框关闭
	bool WaitTalkClose(int index, DWORD ms = 0);
	// 对话按钮是否已打开, 靠近NPC会出现对话按钮
	bool TalkBtnIsOpen();
	// 练功场窗口是否已打开
	bool LianGongChangIsOpen();
	// 关闭练功场窗口
	void CloseLianGongChangBox();
	// 聊天窗口是否已打开
	bool SpeakIsOpen();
	// 关闭聊天窗口
	void CloseSpeakBox();
	// 确定按钮是否打开
	bool SureBtnIsOpen();
	// 确认按钮是否打开
	bool ConfirmBtnIsOpen();
	// 操作确认按钮flag=0取消 flag=1确定
	void ClickConfirmBtn(int flag);
	// 等待确认按钮打开
	bool WaitForConfirmBtnOpen(DWORD ms=1000);
	// 关闭练功场领取经验框
	void CloseLGCBox();

	// 社交信息框打开否
	bool SheJiaoBoxIsOpen();
	// 等待社交信息框打开
	bool WaitForSheJiaoBox(DWORD ms = 2000);
	// 关闭社交信息框
	void CloseSheJiaoBox(bool is_check=false);

	// 是否在登录画面
	bool IsInLoginPic(HWND hwnd=NULL);
	// 等待进入登录画面
	bool WaitForInLoginPic(HWND hwnd=NULL, DWORD ms = 15000);

	// 是否在游戏画面 没有其他弹层
	bool IsInGamePic();
	// 等待进入游戏画面
	bool WaitForInGamePic(DWORD ms=15000);
	// 公共框是否打开
	bool CommonBoxIsOpen();
	// 关闭通用层
	void CloseCommonBox();

	// 关闭所有遮挡层
	void CloseAllBox();
public:
	// Game类
	Game* m_pGame;
};