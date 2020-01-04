#pragma once
#include <Windows.h>
#include <My/Common/Link.hpp>
#include <vector>
#include <list>

using namespace std;

typedef unsigned char      u_char;
typedef unsigned int       u_int;
typedef unsigned long long qword;

#define STEP_IS_MOVE(step) (step.OpCode==OP_MOVE)
#define STEP_IS_CLICK(step) (step.OpCode==OP_CLICK)
#define STEP_IS_SELECT(step) (step.OpCode==OP_SELECT)
#define STEP_IS_MOUMOVE(step) (step.OpCode==OP_MOUMOVE)
#define STEP_IS_WAIT(step) (step.OpCode==OP_KEY)
#define STEP_IS_WAIT(step) (step.OpCode==OP_WAIT)

#define USE_MY_LINK 1
#define MAX_STEP_FILES 16

enum STEP_CODE
{
	OP_UNKNOW  = 0x00,  // 未知
	OP_MOVE,            // 移动
	OP_MOVEFAR,         // 传送
	OP_MOVERAND,        // 随移
	OP_NPC,             // NPC对话
	OP_SELECT,          // 选择对话选项
	OP_MAGIC,           // 技能
	OP_MAGIC_PET,       // 技能-宠物
	OP_KAWEI,           // 卡位打BOSS
	OP_CRAZY,           // 狂甩
	OP_CLEAR,           // 清怪
	OP_KAIRUI,          // 凯瑞
	OP_PICKUP,          // 捡物
	OP_CHECKIN,         // 存物
	OP_USEITEM,         // 使用物品
	OP_DROPITEM,        // 丢物
	OP_SELL,            // 卖东西
	OP_BUTTON,          // 点击按钮
	OP_CLICK,           // 点击
	OP_CLICKRAND,       // 随点
	OP_CLICKCRAZ,       // 狂点
	OP_FIND,            // 查找
	OP_WAIT,            // 等待
	OP_WAIT_GW,         // 等怪
	OP_SMALL,           // 小号操作
	OP_RECORD,          // 记录
};

struct Point
{
	int x;
	int y;
	int flag; // 0-屏幕坐标不需要转换 1-基于游戏坐标需要转换
};
// 游戏执行步骤
struct _step_
{
	char      Cmd[128];    // 原始字符串
	STEP_CODE OpCode;      // 操作码

	DWORD     X;           // 要操作的位置X
	DWORD     Y;           // 要操作的位置Y
	DWORD     X2;
	DWORD     Y2;
	DWORD     NPCId;       // 要对话的NPCID 
	CHAR      NPCName[32]; // 要对话的NPC名称
	DWORD     SelectNo;    // 对话选择索引 0开始
	CHAR      Name[128];   // 名称 根据操作码来区别
	CHAR      Magic[128];  // 技能
	DWORD     WaitMs;      // 等待多少毫秒或是否等待技能冷却或技能可以有多少秒冷却
	DWORD     OpCount;     // 操作次数
	DWORD     ButtonId;    // 按钮ID
	int       SmallV;      // 小号操作值
	DWORD     Extra[8];    // 扩展
	__int64   ExecTime;    // 执行时间
	bool      Exec;        // 已在执行
	int       Index;       // 索引
};

class Explode;
class GameStep
{
public:
	// ...
	GameStep();

	// 获取正在执行的步骤
	_step_* Current(Link<_step_*>& link);
	// 上一个链表指针
	_step_* Prev(Link<_step_*>& link);
	// 完成正在执行步骤 返回下一个
	_step_* CompleteExec(Link<_step_*>& link);
	// 获得当前步骤操作码
	STEP_CODE CurrentCode(Link<_step_*>& link);
	// 获得下一步骤操作码
	STEP_CODE NextCode(Link<_step_*>& link);

	// 设置
	void SetConsoleTle(const char* cmd);
	// 随机选择步骤文件
	void SelectRandStep();
	// 初始化步骤
	bool InitSteps(const char* path, const char* file);
	// 初始化去雷鸣步骤
	int InitGoLeiMingSteps();
	// 解析步骤
	int ParseStep(const char* data, Link<_step_*>& link);
	// 重置执行步骤索引
	void ResetStep(int index = 0, int flag = 0xff);
private:
	// 转成实际指令
	STEP_CODE TransFormOP(const char* data);
	// 转成实际坐标
	bool TransFormPos(const char* str, _step_& step);
	// 转成实际坐标
	bool TransFormPos(const char* str, DWORD& x, DWORD& y);
	// 转成实际技能
	bool TransFormMagic(Explode& line, _step_& step);
	// 转成Wait数据
	bool    TransFormWait(Explode& line, _step_& step);
	// 小号
	void TransFormSmall(Explode& line, _step_& step);
	// 添加步骤
	void AddStep(_step_& step);
public:
	// 所有执行步骤文件数量
	int m_nStepCount = 0;
	// 流程文件列表
	char m_chStepFiles[MAX_STEP_FILES][64];
	// m_nStepRandListIndex的索引 会先循环完 再次打乱
	int m_nStepRandIndex = -1;
	// 随机索引列表, 会先打乱把索引存在此数组里面
	int  m_nStepRandListIndex[MAX_STEP_FILES];
	// 所有执行步骤文件
	Link<_step_*> m_StepList[MAX_STEP_FILES];
	// 游戏步骤链表(当前正在执行)
	Link<_step_*> m_Step;
	// 神殿去雷鸣步骤链表
	Link<_step_*> m_GoLeiMingStep;

	// 游戏步骤数量
	int m_iStepCount = 0;
	// 当前执行步骤索引
	int m_iStepIndex = 0;
	// 当前执行步骤索引[m_GoLeiMingStep]
	int m_iStepGoLMIndex = 0;
	// 游戏按此步骤进行
	_step_* m_Steps;
	// 步骤开始索引
	int m_iStepStartIndex = 0;
};