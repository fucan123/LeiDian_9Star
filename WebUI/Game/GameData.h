#pragma once
#include <Windows.h>

#define NAME_OFFSET     0x0FC // 人物的名称
#define X_OFFSET        0x028 // X坐标相对于人物首地址的偏移
#define Y_OFFSET        0x02C // Y坐标相对于人物首地址的偏移
#define LIFE_OFFSET     0xF58 // 血量相对于人物首地址的偏移
#define LIFE_MAX_OFFSET 0xD38 // 血量最大值相对于人物首地址的偏移

#define CLICK_X_OPEN_MAP  1120 // 打开地图坐标X
#define CLICK_Y_OPEN_MAP  66   // 打开地图坐标Y
#define CLICK_X2_OPEN_MAP 1220 // 打开地图坐标X2
#define CLICK_Y2_OPEN_MAP 125  // 打开地图坐标Y2

#define CLICK_X_GOWAY     725  // 自动寻路坐标X
#define CLICK_Y_GOWAY     620  // 自动寻路坐标Y
#define CLICK_X2_GOWAY    790  // 自动寻路坐标X2
#define CLICK_Y2_GOWAY    630  // 自动寻路坐标Y2

// 游戏物品类型
enum ITEM_TYPE
{
	未知物品 = 0x00,       // 不知道什么物品
	速效治疗包 = 0x000B5593, // 可以开出几瓶速效治疗药水
	神恩治疗药水 = 0x000B783C,
	速效治疗药水 = 0x000F6982, // +2500生命值
	速效圣兽灵药 = 0x000F943E, // 复活宝宝的
	三十星神兽碎片加1 = 0x000F90E4,
	三十星神兽碎片加2 = 0x000F90E5,
	魔魂晶石 = 0x000FD35E,
	灵魂晶石 = 0x000FD368,
	幻魔晶石 = 0x000FD372,
};

// 自动捡拾物品信息
typedef struct conf_item_info
{
	CHAR      Name[32];
	DWORD     Type;
	DWORD     Level;
} ConfItemInfo;

typedef struct game_data_addr
{
	DWORD Player;  // 人物首地址
	DWORD Name;    // 角色的名字
	DWORD CoorX;   // X坐标
	DWORD CoorY;   // Y坐标
	DWORD MoveX;   // X坐标(目的地)
	DWORD MoveY;   // Y坐标(目的地)
	DWORD Life;    // 血量
	DWORD LifeMax; // 血量最大值
} GameDataAddr;

// 共享写入目的地xy地址数据
typedef struct share_write_xy_data
{
	DWORD InDll;   // 是否已注入DLL
	DWORD AddrX;   // X地址
	DWORD AddrY;   // Y地址
	DWORD X;       // X数值
	DWORD Y;       // Y数值
	DWORD Flag;    // 0-主程序写入数据 1-Dll程序写入数据
} ShareWriteXYData;

class Game;
class GameData
{
public:
	GameData(Game* p);

	
	// 监听游戏
	void WatchGame();
	// 获取游戏窗口
	void FindGameWnd();
	// 枚举窗口
	static BOOL CALLBACK EnumProc(HWND hWnd, LPARAM lParam);

	// 是否在指定区域坐标 allow=误差
	bool IsInArea(int x, int y, int allow, _account_* account=nullptr);
	// 是否不在指定区域坐标 allow=误差
	bool IsNotInArea(int x, int y, int allow, _account_* account=nullptr);

	// 是否在神殿
	bool IsInShenDian(_account_* account=nullptr);
	// 是否在副本门口
	bool IsInFBDoor(_account_* account=nullptr);

	// 获取人物首地址
	bool FindPlayerAddr();
	// 获取目的地坐标地址
	bool FindMoveCoorAddr();

	// 读取角色
	bool ReadName(char* name, _account_* account=nullptr);
	// 读取坐标
	bool ReadCoor(DWORD* x=nullptr, DWORD* y=nullptr, _account_* account=nullptr);
	// 读取生命值
	DWORD ReadLife(DWORD* life=nullptr, DWORD* life_max=nullptr, _account_* account=nullptr);

	// 创建共享内存
	void CreateShareMemory();
	// 写入目的地
	void WriteMoveCoor(DWORD x, DWORD y, _account_* account=nullptr);

	// 搜索特征码
	DWORD SearchCode(DWORD* codes, DWORD length, DWORD* save, DWORD save_length = 1, DWORD step = 4);
	// 搜索特征码
	DWORD SearchByteCode(BYTE* codes, DWORD length);
	// 读取四字节内容
	bool ReadDwordMemory(DWORD addr, DWORD& v, _account_* account=nullptr);
	// 读取内存
	bool ReadMemory(PVOID addr, PVOID save, DWORD length, _account_* account=nullptr);
	// 读取游戏内存
	bool ReadGameMemory(DWORD flag = 0x01);

public:
	Game* m_pGame;

	// 当前X坐标
	DWORD m_dwX = 0;
	// 当前Y坐标
	DWORD m_dwY = 0;
	// 当前血量
	DWORD m_dwLife = 0;
	// 血量最大值
	DWORD m_dwLifeMax = 0;

	// 数据
	GameDataAddr m_DataAddr;
	// 数据大号
	GameDataAddr m_DataAddrBig;
public:
	// 游戏大号窗口
	HWND m_hWndBig = NULL;
	// 游戏句柄
	HANDLE m_hProcessBig = NULL;
	// 大号
	_account_* m_pAccoutBig = NULL;

	// 游戏权限句柄
	HANDLE   m_hGameProcess = NULL;
	// 是否读取完毕
	bool  m_bIsReadEnd;
	// 读取内存块的大小
	DWORD m_dwReadSize;
	// 读取基地址
	DWORD m_dwReadBase;
	// 读取内容临时内存
	BYTE* m_pReadBuffer;

	HANDLE m_hShareMap;    // 共享内存
	ShareWriteXYData* m_pShareBuffer; // 共享内存
	bool m_bInDll = false;
};