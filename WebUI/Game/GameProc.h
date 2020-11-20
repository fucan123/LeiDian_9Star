#pragma once
#include "GameStep.h"
#include <My/Common/Link.hpp>
#include <Windows.h>
#include <vector>
#include <fstream>

using namespace std;

#define ONE_COOR_PIX     50   // 一坐标占用像数
#define ONE_MAX_MOV_COOR 8    // 一次最大可移动8距离坐标
#define ONE_MOV_MAX_MS   1000 // 一次移动最大允许时间(毫秒)

#define BAG_NUM         40       // 包包容量
#define WUPIN_YAOBAO    0x0B5593 // 药包
#define WUPIN_XIANGLIAN 0x0B5B24 // 爱娜祈祷项链
#define WUPIN_YAO       0x0F6982 // 药
#define WUPIN_LINGYAO   0x0F943E // 速效圣兽灵药

#define RUNRUN 1

struct _step_;
struct _account_;

enum ComImgIndex;

class Game;
class GameStep;
class GameProc
{
public:
	// !!!
	GameProc(Game* p);

	// 初始化数据
	void InitData();
	// 初始化流程
	bool InitSteps();
	// 切换游戏窗口
	void SwitchGameWnd(HWND);
	// 切换游戏帐号
	void SwitchGameAccount(_account_* account);
	// 窗口置前
	void SetForegroundWindow(HWND hwnd);
	// 执行
	void Exec(_account_* account);
	// 运行
	void Run(_account_* account);
	// 去入队坐标
	void GoInTeamPos(_account_* account);
	// 创建队伍
	void CreateTeam();
	// 邀请入队
	void ViteInTeam();
	// 入队
	void InTeam(_account_* account);
	// 进副本
	void InFB(_account_* account);
	// 所有人进副本
	void AllInFB(_account_* account_open);
	// 同意系统信息
	void AgreenMsg(const char* name, HWND hwnd=NULL);
	// 同意系统信息
	int AgreenMsg(const char* name, int icon_index, bool click=false, HWND hwnd = NULL);
	// 神殿去雷鸣大陆流程
	void GoLeiMing();
	// 去领取项链
	void GoGetXiangLian();
	// 询问项链数量
	_account_*  AskXiangLian();
	// 去副本门口
	void GoFBDoor(_account_* account);
	// 开启副本
	_account_* OpenFB();
	// 出副本
	void OutFB(_account_* account);
	// 获取开启副本帐号
	_account_* GetOpenFBAccount();
	// 是否由大号开启副本
	bool IsBigOpenFB();
	// 继续副本
	void ContinueInFB();
	// 是否在副本
	bool IsInFB(_account_* account=nullptr);
	// 执行副本流程
	void ExecInFB();
	// 执行流程
	bool ExecStep(Link<_step_*>& link, bool isfb=false);
	// 步骤是否已执行完毕
	bool StepIsComplete();
	// 移动
	void Move(bool rand_click=false);
	// NCP
	void NPC();
	// NPC
	void NPC(const char* name, int x, int y, int x2 = 0, int y2 = 0);
	// 最后一个对话的NPC
	bool NPCLast(bool check_pos=true, DWORD mov_sleep_ms=0);
	// 选择
	void Select();
	// 技能
	void Magic();
	// 卡位
	void KaWei();
	// 清怪
	void Clear();
	// 凯瑞
	void KaiRui();
	// 捡物
	void PickUp();
	// 存钱
	void SaveMoney();
	// 存物
	DWORD CheckIn(bool in=true);
	// 使用物品
	void UseItem();
	// 扔物品
	void DropItem();
	// 售卖物品
	void SellItem();
	// 按钮
	void Button();
	// 按钮
	bool Button(int button_id, DWORD sleep_ms=0, const char* name=nullptr);
	// 关闭弹框
	bool CloseTipBox();
	// 关闭物品使用提示框
	bool CloseItemUseTipBox();
	// 关闭系统邀请提示框
	bool CloseSystemViteBox();
	// 查找
	void Find(Link<_step_*>& link);
	// 随机点击
	void ClickRand();
	// 狂暴点击
	void ClickCrazy();
	// 等待
	void Wait();
	// 等待
	void Wait(DWORD ms, int no_open=1);
	// 小号
	void Small();
	// 复活
	void ReBorn();
	// 是否检查此NPC对话完成
	bool IsCheckNPC(const char* name);
	// 是否检查此NPC弹框出来
	bool IsCheckNPCTipBox(const char* name);
	// 鼠标移动[相对于x或y移动rx或ry距离]
	void MouseMove(int x, int y, int rx, int ry, HWND hwnd=NULL);
	// 鼠标移动
	void MouseMove(int x, int y, HWND hwnd=NULL);
	// 鼠标滚轮
	void MouseWheel(int x, int y, int z, HWND hwnd=NULL);
	// 鼠标滚轮
	void MouseWheel(int z, HWND hwnd=NULL);
	// 鼠标左键点击
	void Click(int x, int y, int ex, int ey, int flag = 0xff, HWND hwnd=NULL);
	// 鼠标左键点击
	void Click(int x, int y, int flag = 0xff, HWND hwnd=NULL);
	// 鼠标左键点击[不包括此帐号]
	void ClickOther(int x, int y, int ex, int ey, _account_* account_no);
	// 鼠标左键双击
	void DBClick(int x, int y, HWND hwnd=NULL);
	// 按键
	void Keyboard(char key, int flag=0xff, HWND hwnd=NULL);
	// 读取人物坐标
	bool ReadCoor();
	// 读取人物血量
	bool ReadLife();
	// 读取快捷键上面物品数量
	bool ReadQuickKey2Num();
	// 读取包包物品
	bool ReadBag();
	// 是否需要加血量
	int  IsNeedAddLife();
	// 是否最前窗口
	bool IsForegroundWindow();
	// 加血
	void AddLife();
	// 发送信息给服务端
	void SendMsg(const char* v, const char* v2=nullptr);
	// 停止
	void Stop(bool v=true);
	// 打开日记文件
	void OpenLogFile();
	// 写入日记
	void WriteLog(const char* log, bool flush=true);
public:
	// 要操作的游戏窗口
	HWND m_hWndGame;
	// 当前操作的帐号
	_account_* m_pAccount;
	// 游戏指针
	Game*     m_pGame;
	// 游戏步骤指针
	GameStep* m_pGameStep;
	// 正在执行的步骤
	_step_* m_pStep;
	// 正在执行的步骤[副本]
	_step_* m_pStepCopy;
	// 上次执行的步骤
	_step_* m_pStepLast;
	// 已记录步骤[卡住重回此步骤]
	_step_* m_pStepRecord;
	// 是否记录步骤
	bool m_bIsRecordStep;
	// 是否停止
	bool  m_bStop = false;
	// 是否暂停
	bool  m_bPause = false;
	// 是否重新开始
	bool  m_bReStart = false;
	// 是否狂甩
	bool m_bIsCrazy = false;
	// 狂甩技能
	char m_CrazyMagic[32];

	// 是否锁定进入副本
	bool m_bLockGoFB = false;
	// 是否在副本里面
	bool m_bAtFB = false;
	// 是否已清凯瑞
	bool m_bClearKaiRui = false;
	// 是否重置到记录步骤
	bool m_bIsResetRecordStep = false;
	// 是否第一次移动, 需要点一下地图
	bool m_bIsFirstMove = true;
	// 是否重开副本
	bool m_bReOpenFB = false;

	// 重新移动次数
	int m_nReMoveCount = 0;
	// 药包数量
	int m_nYaoBao = 0;
	// 药数量
	int m_nYao = 0;
	// 药包留存数量
	int m_nLiveYaoBao = 6;
	// 药留存数量
	int m_nLiveYao = 3;
	// 重开副本次数
	int m_nReOpenFBCount = 0;
	// 刷副本次数
	int m_nPlayFBCount = 0;
	// 已经计数时间
	int m_nFBTimeLong = 0;
	// 开始副本时间
	int m_nStartFBTime = 0;
	// 更新副本时间
	int m_nUpdateFBTimeLongTime = 0;

	// 上次的数据
	struct {
		DWORD OpCode;
		DWORD MvX;
		DWORD MvY;
		DWORD NPCOpCode; // 是对话还是点击
		char  NPCName[32];  // 名称
		DWORD ClickX;
		DWORD ClickY;
		DWORD ClickX2;
		DWORD ClickY2;
	} m_stLast;

	// 狂点的数据
	struct {
		int X;
		int Y;
		int X2;
		int Y2;
		int Count;
	} m_ClickCrazy;
};
