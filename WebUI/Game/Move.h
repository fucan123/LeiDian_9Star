#pragma once
#include <Windows.h>

struct _account_;
class Game;
class Move
{
public:
	// ...
	Move(Game* p);

	// 初始化数据
	void InitData();
	// 移动到终点[达到终点返回]
	void RunEnd(DWORD x, DWORD y, _account_* account);
	// 移动
	int  Run(DWORD x, DWORD y, _account_* account, DWORD click_x, DWORD click_y, bool is_click=false, bool rand_click=false);
	// 打开游戏地图
	void OpenMap(_account_* account);
	// 关闭游戏地图
	void CloseMap(HWND hwnd=NULL);
	// 地图是否打开
	bool IsOpenMap();
	// 等待地图打开
	void WaitMapOpen(DWORD ms=1500);
	// 点击自动寻路
    void GoWay();
	// 设置移动位置
	void SetMove(DWORD x, DWORD y, _account_* account);
	// 清除移动数据
	void ClearMove();
	// 是否达到终点
	bool IsMoveEnd(_account_* account);
	// 是否移动
	bool IsMove(_account_* account);
public:
	// 游戏类
	Game* m_pGame;
	// 当前位置X
	DWORD m_dwX = 0;
	// 当前位置Y
	DWORD m_dwY = 0;
	// 上次位置X
	DWORD m_dwLastX = 0;
	// 上次位置Y
	DWORD m_dwLastY = 0;
	// 移动位置X
	DWORD m_dwMvX;
	// 移动位置Y
	DWORD m_dwMvY;
	// 上次移动位置X
	DWORD m_dwLastMvX = 0;
	// 上次移动位置Y
	DWORD m_dwLastMvY = 0;
	// 判断移动结束时间
	DWORD m_dwIsEndTime;
	// 判断是否移动时间
	DWORD m_dwIsMvTime;
	// 用于比较的X
	DWORD m_dwCmpX;
	// 用于比较的Y
	DWORD m_dwCmpY;
	// 移动时间
	DWORD m_dwMvTime;
	// 获取坐标时间
	int   m_iGetPosTime;
	// 获取坐标标记时间
	int   m_iFlagPosTime;
};