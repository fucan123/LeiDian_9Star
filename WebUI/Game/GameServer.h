#pragma once
#include <My/Socket/MServer.h>

enum SOCKET_OPCODE
{
	SCK_PING = 0x00,
	SCK_MSG,         // 信息
	SCK_MSG2,        // 信息
	SCK_OPEN,        // 游戏窗口已打开
	SCK_ACCOUNT,     // 帐号信息 0-3位=选择哪个角色, 4-7=是否是大号
	SCK_LOGIN,       // 可以登录帐号了
	SCK_INGAME,      // 已进入游戏
	SCK_CANTEAM,     // 可以组队了
	SCK_INTEAM,      // 入队
	SCK_CANINFB,     // 可以进副本了
	SCK_OPENFB,      // 开启副本
	SCK_INFB,        // 进入副本
	SCK_OUTFB,       // 出去副本
	SCK_ATFB,        // 是否已在副本
	SCK_SMALLOUTFB,  // 小号出副本
	SCK_SMALLINFB,   // 小号进副本
	SCK_GETXL,       // 获取进副本项链数量
	SCK_NOYAOSI,     // 大号没有钥匙
	SCK_PICKUPITEM,  // 捡物
	SCK_REBORN,      // 复活
	SCK_QIAZHUS,     // 卡住多久重启
	SCK_OFFLINE,     // 游戏下线了
	SCK_CLOSE,       // 关闭游戏

	SCK_DEBUG,       // 是否调试
	SCK_NOAUOTOSLT,  // 不自动选区
	SCK_TALK,        // 喊话
	SCK_TALK_OPEN,   // 喊话开启

	SCK_EXPIRE_OUT,  // 过期
};

struct _account_;

class JsCall;
class Game;
class GameServer
{
public:
	// ...
	GameServer(Game* p);

    // 
	void SetSelf(GameServer* p);
	// 监听
	void Listen(USHORT port);

	// 游戏窗口打开响应
	void Open(const char* data, int len);
	// 登录
	void Login(_account_* p);
	// 信息
	void Msg(_account_* p, const char* data, int len);
	// 已进入游戏
	void InGame(_account_* p, const char* data, int len);
	// 可以组队
	void CanTeam(_account_* p, const char* data, int len);
	// 入队
	void InTeam(_account_* p, const char* data, int len);
	// 可以进副本了
	void CanInFB(_account_* p, const char* data, int len);
	// 开启副本
	void OpenFB(_account_* p, const char* data, int len);
	// 进入副本
	void InFB(_account_* p, const char* data, int len);
	// 出去副本
	void OutFB(_account_* p, const char* data, int len);
	// 已在副本
	void AtFB(_account_* p, const char* data, int len);
	// 小号进入副本
	void SmallInFB(_account_* p, const char* data, int len);
	// 小号出去副本
	void SmallOutFB(_account_* p, const char* data, int len);
	// 获取进副本项链数量 
	void GetXL(_account_* p, const char* data, int len);
	// 捡物
	void PickUpItem(_account_* p, const char* data, int len);
	// 复活
	void ReBorn(_account_*p, const char* data, int lne);
	// 游戏已下线
	void OffLine(_account_* p, const char* data, int len);

	// 询问项链数量
	void AskXLCount();
	// 发送数据
	int Send(SOCKET s=0);
	// 发送数据
	int Send(SOCKET s, SOCKET_OPCODE opcode, bool clear = false);
	// 发送给其他帐号
	int SendToOther(SOCKET s, SOCKET_OPCODE opcode, bool clear = false);

	static void OnConnect(SOCKET client, SOCKADDR_IN& info, int index);
	static void OnRead(SOCKET client, int index, int opcode, const char* data, int len);
	static void OnClose(SOCKET client, int index);
public:
	static GameServer* self;

	static JsCall* m_pJsCall;
	static Game*   m_pGame;
	MServer m_Server;

	// 正在操作的SOCKET
	SOCKET m_Socket;

	// 是否已经创建了队伍
	int m_iCreateTeam = 0;
	// 大号是否没有钥匙
	int m_iNoYaosi = 0;
	// 询问了多少个帐号项链数量
	int m_iSendXL = 0;
	// 应答项链数量
	int m_iRecvXL = 0;
	// 出副本数量
	int m_iOutFBCount = 0;
	// 连续刷了多少趟副本
	int m_iPlayFB = 0;
	// 掉了多少次线
	int m_iOffLine = 0;
	// 复活了多少次
	int m_iReBorn = 0;
};