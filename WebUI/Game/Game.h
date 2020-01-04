#include "GameServer.h"
#include "Emulator.h"
#include "GameData.h"
#include <Windows.h>
#include <vector>
#include <fstream>

#define P2DW(v) (*(DWORD*)(v))       // ת��DWORD��ֵ
#define P2INT(v) (*(int*)(v))        // ת��int��ֵ

#define LOG(v) m_pGame->AddUILog(v,nullptr)
#define LOGVARP(p,...) { sprintf_s(p,__VA_ARGS__);LOG(p); }
#define LOGVARN(n,...) {char _s[n]; _s[n-1]=0; LOGVARP(_s,__VA_ARGS__); }

#define LOG2(v,cla) m_pGame->AddUILog(v,cla)
#define LOGVARP2(p,cla,...) { sprintf_s(p,__VA_ARGS__);LOG2(p,cla); }
#define LOGVARN2(n,cla,...) {char _s[n]; _s[n-1]=0; LOGVARP2(_s,cla,__VA_ARGS__); }

#define MAKESPOS(v) (int(v/m_fScale))

#define ACCSTA_INIT       0x1000 // �ʺŻ�δ��¼
#define ACCSTA_READY      0x0001 // ׼����¼
#define ACCSTA_LOGIN      0x0002 // ���ڵ�¼
#define ACCSTA_ONLINE     0x0004 // �ʺ�����
#define ACCSTA_OPENFB     0x0008 // ���ڿ�������
#define ACCSTA_ATFB       0x0010 // �Ƿ��ڸ���
#define ACCSTA_COMPLETED  0x0200 // �Ѿ�û������ˢ��
#define ACCSTA_OFFLINE    0x0100 // �ʺ�����

#define MAX_ACCOUNT_LOGIN 5      // ���֧�ֶ����˺�����

struct my_msg2 {
	char id[32];
	char text[64];
	char cla[32];
	int  value;
	int  value2;
};

typedef struct _account_
{
	char   Name[32];       // �ʺ�
	char   Password[32];   // ����
	char   Role[16];       // ��ɫ����
	char   SerBig[32];     // ��Ϸ����
	char   SerSmall[32];   // ��ϷС��
	int    RoleNo;
	int    XL;             // �������� 
	int    IsGetXL;        // �Ƿ��Ѿ���ȡ������
	int    IsReadXL;       // �Ƿ��Ѿ���ȡ����������
	int    IsReady;        // �Ƿ���׼��
	int    IsBig;          // �Ƿ���
	int    IsLogin;        // �Ƿ��������ʺ������¼
	int    OfflineLogin;   // �Ƿ��������
	float  Scale;
	HWND   GameWnd;
	RECT   GameRect;
	DWORD  GamePid;        // ��Ϸ����ID
	int    Status;
	char   StatusStr[16];
	SOCKET Socket;
	int    LoginTime;     // ��¼ʱ��
	int    PlayTime;      // ����ʱ��
	int    AtFBTime;      // ���븱��ʱ��
	int    LastTime;      // ���ͨ��ʱ��
	int    Flag;          // 
	int    Index;

	MNQ*   Mnq;           // ģ������Ϣ, һ��ģ�����ͻ��
	GameDataAddr Addr;    // ���ݵ�ַ
	int    IsGetAddr;     // �Ƿ��ѻ�ȡ��ַ
	DWORD  MvX;           // Ҫ�ƶ�����x
	DWORD  MvY;           // Ҫ�ƶ�����y
	DWORD  LastX;         // �ϴε�λ��x
	DWORD  LastY;         // �ϴε�λ��y
	DWORD  MvTime;        // �ϴ��ƶ�ʱ��
} Account;

using namespace std;

class Sqlite;
class JsCall;
class WebList;
class Home;
class Driver;
class Emulator;
class GameServer;
class GameConf;
class GameData;
class GameProc;
class Move;
class Item;
class Magic;
class Talk;
class Pet;
class PrintScreen;

class Game
{
public:
	// ...
	Game(JsCall* p, char* account_ctrl_id, char* log_ctrl_id, const char* conf_path, const char* title, HWND hWnd);
	// ...
	~Game();

	// ����
	void Listen(USHORT port);
	// ִ��
	void Run();
	// ��¼
	void Login(Account* p);
	// ���뵽��¼����
	void GoLoginUI(int left, int top);
	// �˳�
	void LogOut(Account* account);
	// �����ʺ������¼
	void Input(Account* p);
	// �Զ��Ǻ�
	bool AutoLogin();
	// ȫ������
	void LoginCompleted();
	// ���õǺ�����
	void SetLoginFlag(int flag);
	// ��ȡ��Ҫ��¼������
	int GetLoginCount();
	// ����¼��ʱ���ʺ�
	int CheckLoginTimeOut();
	// �����Ϸ����ID�Ƿ����
	bool CheckGamePid(DWORD pid);
	// ������Ϸ����
	bool SetGameWnd(HWND hwnd, bool* isbig=nullptr);
	// ������Ϸ����ID
	bool SetGameAddr(Account* p, GameDataAddr* addr);
	// ����״̬-ȫ��
	void SetAllStatus(int status);
	// ����׼��
	void SetReady(Account* p, int v);
	// ����ģ��������
	void SetMNQName(Account* p, char* name);
	// ����״̬
	void SetStatus(Account* p, int status, bool update_text=false);
	// ����SOKCET
	void SetSocket(Account* p, SOCKET s);
	// ����Flag
	void SetFlag(Account* p, int flag);
	// ���������
	void SetCompleted(Account* p);
	// ��ȡ�ʺ�����
	int GetAccountCount();
	// ��ȡ����ˢ�����ʺ�����
	int GetAtFBCount();
	// ��ȡ���������˺�����
	int GetOnLineCount();
	// �ʺŻ�ȡ
	Account* GetAccount(const char* name);
	// ��ȡ�ʺ�
	Account* GetAccount(int index);
	// ��ȡ�ʺ�[����״̬]
	Account* GetAccountByStatus(int status);
	// ��ȡ��������ʺ�
	Account* GetMaxXLAccount(Account** last=nullptr);
	// ��ȡ׼����¼���ʺ�[�Ƿ��ų����]
	Account* GetReadyAccount(bool nobig=true);
	// ��ȡ��һ��Ҫ��¼���ʺ�
	Account* GetNextLoginAccount();
	// ��ȡ�ʺ�
	Account* GetAccountBySocket(SOCKET s);
	// ��ȡ����ʺ�
	Account* GetBigAccount();
	// ��ȡ���SOCKET
	SOCKET   GetBigSocket();
	// �����Ѿ����
	void SetInTeam(int index);
	// �ر���Ϸ
	void CloseGame(int index);
	// ������׼�����ʺ�����
	void SetReadyCount(int v);
	// ������׼�����ʺ�����
	int AddReadyCount(int add = 1);
	// �Ա��ʺ�״̬
	bool CheckStatus(Account* p, int status);
	// �Ƿ��Զ���¼
	bool IsAutoLogin();
	// �Ƿ���ȫ��׼����
	bool IsAllReady();
	// �ʺ��Ƿ��ڵ�¼
	bool IsLogin(Account* p);
	// �ʺ��Ƿ�����
	bool IsOnline(Account* p);
	// ��ȡ״̬�ַ�
	char* GetStatusStr(Account* p);

	// ���Ϳ�ס����ʱ��
	int SendQiaZhuS(int second);
	// ���͸����
	int SendToBig(SOCKET_OPCODE opcode, bool clear=false);
	// ���͸����
	int SendToBig(SOCKET_OPCODE opcode, int v, bool clear = true);

	// ������ݿ�
	void CheckDB();
	// ������Ʒ��Ϣ
	void UpdateDBItem(const char* account, const char* item_name);
	// ����ˢ��������
	void UpdateDBFB(int count=-1);
	// ����ˢ����ʱ��
	void UpdateDBFBTimeLong(int time_long);

	// ����ˢ���������ı�
	void UpdateFBCountText(int lx, bool add=true);
	// ����ˢ����ʱ���ı�
	void UpdateFBTimeLongText(int time_long, int add_time_long);
	// ���µ��߸�������ı�
	void UpdateOffLineAllText(int offline, int reborn);

	// д���ռ�
	void WriteLog(const char* log);

	// ��ȡ�����ļ�
	DWORD ReadConf();
	// ��ȡ�ʺ�����
	bool  ReadAccountSetting(const char* data);
	// ��ȡ��������
	void  ReadSetting(const char* data);

	// �Զ��ػ�
	void AutoShutDown();
	// ��ʱ�ػ�
	bool ClockShutDown(int flag=0);
	// �ػ�
	void ShutDown();
	// ��ǰʱ���Ƿ��ڴ�ʱ��
	bool IsInTime(int s_hour, int s_minute, int e_hour, int e_minute);

	// ����
	void PutSetting(wchar_t* name, int v);
	// ����
	void PutSetting(wchar_t* name, wchar_t* v);
	// ����Ϸ
	void OpenGame(int index, bool close_all=true);
	// ����
	void CallTalk(wchar_t* text, int type);
	// ע��DLL
	void InstallDll();
	// �Զ��Ǻ�
	void AutoPlay(int index, bool stop);
	// �����ʺ�
	void AddAccount(Account* account);
	// ��֤����
	void VerifyCard(wchar_t* card);
	// ���³���汾
	void UpdateVer();

	// �����ʺ�״̬
	void UpdateAccountStatus(Account * account);
	// �����ʺ�����ʱ��
	void UpdateAccountPlayTime(Account * account);
	// �����ռǵ�UI����
	void AddUILog(char text[], char cla[], bool ui = false);
	// ����
	void UpdateText(int row, int col, char text[]);

	// ʱ��ת������
	void FormatTimeLong(char* text, int time_long);
public:
	// �۲��Ƿ������Ϸ
	static DWORD WINAPI WatchInGame(LPVOID);
public:
	// ����
	struct
	{
		char MnqPath[32];    // ģ����·��
		char GamePath[32];   // ��Ϸ�ͻ���·��
		char GameFile[32];   // ��Ϸ�ļ�
		char SerBig[32];     // ��Ϸ����
		char SerSmall[32];   // ��ϷС��
		int  CloseMnq;       // ����ǰ�Ƿ�ر�ģ����
		int  InitTimeOut;    // ������ʱʱ��
		int  LoginTimeOut;   // ��¼��ʱ����ʱ��
		int  TimeOut;        // ��Ϸ��ʱʱ��
		int  ReConnect;      // �Ƿ��������
		int  AutoLoginNext;  // �Ƿ��Զ���¼�ʺ�
		int  LogoutByGetXL;  // ���������Ƿ��Զ��˳�
		int  NoGetXL;        // ��������
		int  NoPlayFB;       // ��ˢ����
		int  ShutDownNoXL;   // ˢ��û�������Զ��ػ�
		int  FBMode;         // ����ģʽ1=�����ˢ,2=����ֻˢ,3=�����ˢ,4=ֻ�첻ˢ

		int  ShutDown_SH;    // ��ʱ�ػ�[��ʼСʱ]
		int  ShutDown_SM;    // ��ʱ�ػ�[��ʼ����]
		int  ShutDown_EH;    // ��ʱ�ػ�[����Сʱ]
		int  ShutDown_EM;    // ��ʱ�ػ�[��������]

		int  OffLine_SH;     // ��ʱ�ػ�[��ʼСʱ]
		int  OffLine_SM;     // ��ʱ�ػ�[��ʼ����]
		int  OffLine_EH;     // ��ʱ�ػ�[����Сʱ]
		int  OffLine_EM;     // ��ʱ�ػ�[��������]

		int  AutoLogin_SH;   // ��ʱ��¼[��ʼСʱ]
		int  AutoLogin_SM;   // ��ʱ��¼[��ʼ����]
		int  AutoLogin_EH;   // ��ʱ��¼[����Сʱ]
		int  AutoLogin_EM;   // ��ʱ��¼[��������]

		int  NoAutoSelect;   // ���Զ�ѡ����Ϸ��
		int  TalkOpen;       // �Ƿ��Զ�����    
		int  IsDebug;        // �Ƿ���ʾ������Ϣ
		
	} m_Setting;

public:
	// ����
	Game* m_pGame;
	// ģ����
	Emulator* m_pEmulator;
	// ����
	GameConf* m_pGameConf;
	// ��Ϸ������
	GameData* m_pGameData;
	// ��Ϸ������
	GameProc* m_pGameProc;
	// �ƶ�
	Move*     m_pMove;
	// ��Ʒ
	Item*     m_pItem;
	// ����
	Magic*    m_pMagic;
	// �Ի�
	Talk*     m_pTalk;
	// ����
	Pet*      m_pPet;
	// ��ͼ
	PrintScreen* m_pPrintScreen;
	// ���ݿ�
	Sqlite*   m_pSqlite;

	// JsCall
	JsCall*  m_pJsCall;
	// �ʺ��б��ؼ�
	WebList* m_pAccoutCtrl;
	// �ռ��б��ؼ�
	WebList* m_pLogCtrl;
	// Home
	Home* m_pHome;
	// Driver
	Driver* m_pDriver;

public:
	// UI����
	HWND m_hUIWnd;
	// ���°汾�߳�
	LPTHREAD_START_ROUTINE m_funcUpdateVer;
	// ��Ϣ
	my_msg2 m_Msg[10];

	// ��Ϸ��Ŵ���
	HWND m_hWndBig;
	// ��Ϸ���
	HANDLE m_hProcessBig;

	float m_fScale;

	// ������Ϸ��Ϣ
	GameServer* m_pServer;

	// �ʺ��б�
	std::vector<Account*> m_AccountList;

	// ���
	Account* m_pBig = nullptr;

	// �Ǻ����� -2ֹͣ�� -1ȫ�� �����˺��б�����
	int m_iLoginFlag = -2;

	// ��ǰ�Ǻ�����
	int m_iLoginIndex = 0;
	// ��Ҫ��¼�ʺ�����
	int m_iLoginCount = 0;
	// �Ѿ�׼�����ʺ�����
	int m_iReadyCount = 0;
	// �ϴο��������ʺ�����
	int m_iOpenFBIndex = -1;
	// �Ƿ��ʹ�������
	int m_iSendCreateTeam = 1;
	// �Ƿ�������¼
	bool m_bLockLogin = false;

	char m_chTitle[32];
	char m_chPath[255];
	ofstream m_LogFile;

	bool m_bLock = false;

	// ����ʱ��
	int m_nStartTime = 0;
	// ��������ʱ��
	int m_nUpdateTimeLongTime = 0;
};