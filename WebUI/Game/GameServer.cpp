#include <time.h>
#include "Home.h"
#include "GameServer.h"
#include "Game.h"
#include "GameProc.h"
#include "../Web/JsCall.h"
#include <My/Common/Explode.h>

#define RECV_NAME(var,data,len)  char var[32]; ZeroMemory(var,sizeof(var)); memcpy(var,data,len); 
#define RECV_ROLE(var,data,len)  char var[16]; ZeroMemory(var,sizeof(var)); memcpy(var,data,len); 

GameServer* GameServer::self = nullptr;
JsCall* GameServer::m_pJsCall = nullptr;
Game* GameServer::m_pGame = nullptr;

// ...
GameServer::GameServer(Game * p)
{
	m_pGame = p;
	m_pJsCall = p->m_pJsCall;
}

void GameServer::SetSelf(GameServer * p)
{
	self = p;
	m_iNoYaosi = 0;
}

// 监听
void GameServer::Listen(USHORT port)
{
	m_Server.onconect = OnConnect;
	m_Server.onread = OnRead;
	m_Server.onclose = OnClose;

	LOGVARN(64, "启动监听服务, 端口:%d", port);
	m_Server.Listen(port);
}

// 游戏窗口打开响应
void GameServer::Open(const char* data, int len)
{
	char log[64];
	LOGVARP(log, "游戏窗口已打开, 准备操作\n");
	Account* p = m_pGame->GetReadyAccount(true);
	if (!p) {
		LOGVARP(log, "没有要操作的帐号");
		return;
	}
	m_pGame->SetSocket(p, m_Socket);
	m_pGame->SetFlag(p, 1);

	// 发送是否显示调试
	m_Server.ClearSendString();
	m_Server.SetInt(m_pGame->m_Setting.IsDebug);
	Send(0, SCK_DEBUG, false);

	// 发送是否手动选区
	m_Server.ClearSendString();
	m_Server.SetInt(m_pGame->m_Setting.NoAutoSelect);
	Send(0, SCK_NOAUOTOSLT, false);

	// 发送是否开启喊话
	m_Server.ClearSendString();
	m_Server.SetInt(m_pGame->m_Setting.TalkOpen);
	Send(0, SCK_TALK_OPEN, false);

	// 发送选择哪个角色
	m_Server.ClearSendString();

	// 软件当前路径
	m_Server.SetInt(strlen(m_pGame->m_chPath));
	m_Server.SetContent((void*)m_pGame->m_chPath, strlen(m_pGame->m_chPath));

	// 哪个游戏大区
	char* ser_big = strlen(p->SerBig) > 0 ? p->SerBig : m_pGame->m_Setting.SerBig;
	m_Server.SetInt(strlen(ser_big));
	m_Server.SetContent((void*)ser_big, strlen(ser_big));

	// 哪个游戏小区
	char* ser_small = strlen(p->SerSmall) > 0 ? p->SerSmall : m_pGame->m_Setting.SerSmall;
	m_Server.SetInt(strlen(ser_small));
	m_Server.SetContent((void*)ser_small, strlen(ser_small));

	m_Server.SetInt(strlen(p->Name));
	m_Server.SetContent(p->Name, strlen(p->Name));

	m_Server.SetInt(strlen(p->Password));
	m_Server.SetContent(p->Password, strlen(p->Password));

	m_Server.SetInt(p->RoleNo);
	m_Server.SetInt(m_pGame->m_Setting.NoGetXL ? 1 : p->IsGetXL);
	m_Server.SetInt(p->IsBig);
	m_Server.SetInt(m_pGame->m_Setting.LogoutByGetXL);
	m_Server.SetInt(m_pGame->m_iSendCreateTeam);
	Send(0, SCK_ACCOUNT, false);

	memcpy(&m_pGame->m_fScale, data, 4);
	memcpy(&p->GameWnd, data + 4, 4);
	memcpy(&p->GameRect, data + 8, sizeof(RECT));

	if (m_pGame->m_iSendCreateTeam) // 第一次创建队伍
		m_pGame->m_iSendCreateTeam = 0;
}

// 登录
void GameServer::Login(_account_ * p)
{
	SetForegroundWindow(p->GameWnd);
	m_pGame->Login(p);
}

// 信息
void GameServer::Msg(_account_* p, const char* data, int len)
{
	if (len > 0) {
		if (strstr(data, "|") == nullptr) {
			LOGVARN2(128, "c0", "(MSG)%s: %s\n", p->Name, data);
		}
		else {
			Explode arr("|", data);
			LOGVARN2(128, arr[1], "(MSG)%s: %s\n", p->Name, arr[0]);
		}	
	}
		
}

// 已进入游戏
void GameServer::InGame(_account_* p, const char* data, int len)
{
	char log[64];	

	char name[32], role[16];
	ZeroMemory(name, sizeof(name));
	ZeroMemory(role, sizeof(role));

	const char* tmp = data;
	int name_len = P2INT(tmp);
	memcpy(name, tmp + 4, name_len);
	tmp += 4 + name_len;

	int role_len = P2INT(tmp);
	memcpy(role, tmp + 4, role_len);

	if (!name_len || !role_len) {
		LOGVARP(log, "%s帐号不在列表", name);
		printf("%s 帐号不在列表\n", name);
		goto end;
	}

	p = m_pGame->GetAccount(name);
	if (!p) {
		LOGVARP(log, "%s帐号不在列表2", name);
		printf("%s 帐号不在列表\n", name);
		goto end;
	}

	m_pGame->SetStatus(p, ACCSTA_ONLINE, true);

	p->IsGetXL = 1;
	p->OfflineLogin = 1;
	p->Socket = m_Socket;
	p->LoginTime = 0;
	p->PlayTime = time(nullptr);
	memset(p->Role, 0, sizeof(p->Role));
	memcpy(p->Role, role, strlen(role));
	LOGVARP(log, "%s[%s] 已登入游戏", p->Name, p->Role);
end:
	if (!m_pGame->AutoLogin("GameServer::InGame")) {
		m_pGame->LoginCompleted();
		//m_pGame->m_pUIWnd->m_pLoginBtn->SetText("自动登号");
		//m_pGame->m_pUIWnd->m_pStartBtn->SetEnabled(true);
	}
}

// 可以组队
void GameServer::CanTeam(_account_* p, const char* data, int len)
{
	char log[64];
	if (0 && p->IsBig) {
		LOGVARP(log, "%s 已创建好队伍", p->Name);
		m_pGame->SetReady(p, 1);
		m_pGame->AddReadyCount(1);
		return;
	}
	
	printf("%s(%s)已准备好,可以组队\n", p->Name, p->Role);
	LOGVARP(log, "%s 已准备好,可以组队", p->Name);
	// 发送给其他玩家可以邀请此帐号组队了[其他玩家需要判断自己是否为队长再邀请]
	m_Server.ClearSendString();
	m_Server.SetInt(p->Index);
	m_Server.SetContent(&p->Role, sizeof(p->Role));
	SendToOther(p->Socket, SCK_CANTEAM, false);
}

// 入队
void GameServer::InTeam(_account_* p, const char* data, int len)
{
	char log[64];
	int index = P2INT(data);
	int result = P2INT(data + 4);
	Account* act = m_pGame->GetAccount(index);
	if (!act) {
		LOGVARP(log, "帐号不在此列表,无法入队[%d]", index);
		return;
	}

	m_iCreateTeam = 1;
	printf("%s 邀请 %s 入队%s\n", p->Name, act->Name, result ? "成功" : "失败");
	LOGVARP(log, "%s 邀请 %s 入队%s", p->Name, act->Name, result?"成功":"失败");
	if (!result)
		return;

	if (act->IsBig) { // 大号模拟器
		Sleep(1000);
		m_pGame->m_pGameProc->InTeam(m_pGame->m_pBig);
		CanInFB(act, nullptr, 0);
	}
	else {
		// 告诉此号可以入队
		Send(act->Socket, SCK_INTEAM, true);
	}
}

// 可以进副本了
void GameServer::CanInFB(_account_ * p, const char * data, int len)
{
	m_iCreateTeam = 1;
	char log[64];
	if (p->IsReady == 0) {
		m_pGame->SetReady(p, 1);
		m_pGame->AddReadyCount();
	}
	LOGVARP(log, "%s 已经入队了[%d/%d]", p->Name, m_pGame->m_iReadyCount, m_pGame->m_iLoginCount);

	if (m_pGame->IsAllReady()) {
		m_iCreateTeam = 0;
		printf("CanInFB->询问项链\n");
		AskXLCount();
	}
}

// 开启副本
void GameServer::OpenFB(_account_ * p, const char * data, int len)
{
}

// 进入副本
void GameServer::InFB(_account_ * p, const char * data, int len)
{
	LOGVARN(64, "%s邀请进入副本[%d]", p->Name, p->Socket);
	printf("%s邀请进入副本 [%d]\n", p->Name, p->Socket);
	SendToOther(p->Socket, SCK_INFB, true);

	if (!p->IsBig) {
		Sleep(1000);
		m_pGame->m_pGameProc->InFB(m_pGame->m_pBig);
	}
}

// 出去副本
void GameServer::OutFB(_account_ * p, const char * data, int len)
{
	m_iOutFBCount++;
	m_pGame->SetStatus(p, ACCSTA_ONLINE);
	m_pGame->UpdateAccountStatus(p);
	m_pGame->UpdateFBCountText(m_iOutFBCount, true);

	char log[64];
	LOGVARP2(log, "blue", "%s 出副本", p->Name);

	if (!m_pGame->m_pHome->IsValid()) {
		SendToOther(0, SCK_EXPIRE_OUT, true);
		LOGVARP2(log, "red", "卡号已过期, 请激活.");
		::MessageBoxA(NULL, "卡号已过期, 请激活.", "提示", MB_OK);
	}
}

// 已在副本
void GameServer::AtFB(_account_* p, const char * data, int len)
{
	p->AtFBTime = time(nullptr);
	m_pGame->SetStatus(p, ACCSTA_ATFB);
	m_pGame->UpdateAccountStatus(p);

	LOGVARN2(64, "blue", "%s 开始刷副本", p->Name);
}

// 小号进入副本
void GameServer::SmallInFB(_account_ * p, const char * data, int len)
{
	if (p->IsBig) {
		printf("发送SCK_SMALLINFB\n");
		SendToOther(p->Socket, SCK_SMALLINFB, true);
	}
}

// 小号出去副本
void GameServer::SmallOutFB(_account_ * p, const char * data, int len)
{
	if (p->IsBig) {
		printf("发送SCK_SMALLOUTFB\n");
		SendToOther(p->Socket, SCK_SMALLOUTFB, true);
	}
}

// 获取进副本项链数量 
void GameServer::GetXL(_account_* p, const char * data, int len)
{
	if (m_pGame->ClockShutDown(0))
		return;

	char log[64];
	p->XL = data ? P2INT(data) : 0;
	printf("%s 拥有项链:%d (%d/%d)\n", p->Name, p->XL, m_iRecvXL+1, m_iSendXL);
	LOGVARP(log, "%s 拥有项链:%d", p->Name, p->XL);

	if (++m_iRecvXL >= m_iSendXL) {
		bool big_in = false; // 是否大号进
		SYSTEMTIME stLocal;
		::GetLocalTime(&stLocal);
		if (m_iNoYaosi == 0 && stLocal.wHour >= 20 && stLocal.wHour <= 23) { // 20点到24点可以免费进
			big_in = stLocal.wHour == 23 ? stLocal.wMinute < 59 : true; // 至少需要1分钟
		}
		if (big_in) {
			LOGVARP(log, "由 [大号] 开启副本(发送)[45秒后]\n");
			for (int xxx = 0; xxx < 45; xxx++) {
				//Sleep(1000);
			}
			
			m_pGame->m_pGameProc->OpenFB();
			InFB(m_pGame->m_pBig, nullptr, 0);
			return;
		}

		Account* last = nullptr;
		Account* act = m_pGame->GetMaxXLAccount(&last);
		if (!act) {
			if (last && !last->IsBig) { // 不是大号
				Account* next = m_pGame->GetNextLoginAccount();
				if (next) {
					LOGVARP(log, "%s 没有项链, %s准备登录", last->Name, next->Name);

					m_pGame->SetCompleted(last);         // 设置已完成
					m_Server.ClearSendString();
					m_Server.SetInt(1);
					Send(last->Socket, SCK_CLOSE, false); // 通知游戏关闭
				}
				else {
					LOGVARP(log, "已经没有其它号了,无法进入\n");
					m_pGame->AutoShutDown();
				}
			}
			else {
				LOGVARP(log, "没有人有项链,无法进入\n");
				m_pGame->AutoShutDown();
			}
		}
		else {
			m_pGame->SetStatus(act, ACCSTA_OPENFB, true);
			int ret = Send(act->Socket, SCK_OPENFB, true);

			printf("由%s[%s]开启副本(发送)(%d)\n", act->Name, act->Role, ret);
			LOGVARP(log, "由%s[%s]开启副本(发送)(%d)", act->Name, act->Role, ret);
		}
	}
}

// 捡物
void GameServer::PickUpItem(_account_ * p, const char * data, int len)
{
	//LOGVARN(64, "%s 拾取.%s", p->Name, data);
	m_pGame->UpdateDBItem(p->Name, data);
}

// 复活
void GameServer::ReBorn(_account_ * p, const char * data, int lne)
{
	return;
	LOGVARN(64, "%s[%s] 复活", p->Name, p->Role);
	if (p->IsBig) {
		m_iReBorn++;
		m_pGame->UpdateOffLineAllText(m_iOffLine, m_iReBorn);
	}	
}

// 游戏已下线
void GameServer::OffLine(_account_ * p, const char * data, int len)
{
	return;
	m_pGame->SetStatus(p, ACCSTA_OFFLINE);
	m_pGame->UpdateAccountStatus(p);
	return;
	LOGVARN(64, "%s 掉线了", p->Name);
	
	if (m_pGame->m_Setting.ReConnect && m_pGame->IsAutoLogin()) { // 通知游戏关闭
		m_Server.ClearSendString();
		m_Server.SetInt(1);
		Send(p->Socket, SCK_CLOSE, false);
	}
}

// 询问项链数量
void GameServer::AskXLCount()
{
	char log[64];
	if (!m_pGame->m_pHome->IsValid()) {
		LOGVARP2(log, "red", "卡号已过期, 请激活");
		m_pJsCall->ShowMsg("卡号已过期, 请先激活.", "提示", 2);
		return;
	}
	m_iOutFBCount = 0;
	m_iRecvXL = 0;
	m_iSendXL = SendToOther(0, SCK_GETXL, true);
	if (m_iSendXL > 5)
		m_iSendXL = 5;

	if (m_pGame->GetAccountCount() == 1 && m_pGame->m_pBig) { // 只有配置大号
		GetXL(m_pGame->m_pBig, nullptr, 0);
	}

	printf("询问进副本项链数量, 询问数量:%d\n", m_iSendXL);
	LOGVARP(log, "询问进副本项链数量,询问数量:%d", m_iSendXL);
}

// 发送数据
int GameServer::Send(SOCKET s)
{
	int ret = send(s ? s : m_Socket, m_Server.GetSendString(), m_Server.GetSendLength(), 0);
	//printf("发送:%d %d\n", m_Server.GetSendLength(), ret);
	return ret;
}

// 发送数据
int GameServer::Send(SOCKET s, SOCKET_OPCODE opcode, bool clear)
{
	if (clear) {
		m_Server.ClearSendString();
	}
	m_Server.MakeSendString(opcode);
	//printf("发送:%d %08X\n", opcode, s);
	return Send(s);
}

// 发送给其他帐号
int GameServer::SendToOther(SOCKET s, SOCKET_OPCODE opcode, bool clear)
{
	if (clear) {
		m_Server.ClearSendString();
	}
	m_Server.MakeSendString(opcode);
	int ret = 0;
	int count = m_pGame->GetAccountCount();
	for (int i = 0; i < count; i++) {
		Account* p = m_pGame->GetAccount(i);
		if (p && m_pGame->IsOnline(p) && p->Socket != s) {
			Send(p->Socket);
			ret++;
		}
	}
	return ret;
}

void GameServer::OnConnect(SOCKET client, SOCKADDR_IN& info, int index)
{
	self->m_Socket = client;
	self->m_Server.ClearSendString();
	//self->m_Server.MakeSendString(SCK_FIRST);
	//self->Send(client);
}

void GameServer::OnRead(SOCKET client, int index, int opcode, const char* data, int len)
{
	self->m_Socket = client;

	Account* p = self->m_pGame->GetAccountBySocket(client);
	if (!p && opcode != SCK_OPEN && opcode != SCK_INGAME) {
		printf("找不到对应帐号\n");
		return;
	}
	//printf("%s OpCode:%d %X\n", p ? p->Name : "null", opcode, client);
	if (p) {
		p->LastTime = time(nullptr);
		self->m_pGame->UpdateAccountStatus(p);
	}
	switch (opcode)
	{
	case SCK_OPEN:   // 游戏窗口已打开
		self->Open(data, len);
		break;
	case SCK_LOGIN:  // 可以登录了
		self->Login(p);
		break;
	case SCK_MSG:
		self->Msg(p, data, len);
		break;
	case SCK_MSG2:
		self->m_pGame->WriteLog(data);
		break;
	case SCK_INGAME: // 已进入游戏
		self->InGame(p, data, len);
		break;
	case SCK_CANTEAM: // 可以组队
		self->CanTeam(p, data, len);
		break;
	case SCK_INTEAM:  // 可以进队伍
		self->InTeam(p, data, len);
		break;
	case SCK_CANINFB:  // 可以进副本
		self->CanInFB(p, data, len);
		break;
	case SCK_OPENFB: // 开启副本
		self->OpenFB(p, data, len);
		break;
	case SCK_INFB:   // 进入副本
		self->InFB(p, data, len);
		break;
	case SCK_OUTFB:  // 出去副本
		self->OutFB(p, data, len);
		break;
	case SCK_ATFB:  // 正在副本
		self->AtFB(p, data, len);
		break;
	case SCK_SMALLINFB:   // 小号进入副本
		self->SmallInFB(p, data, len);
		break;
	case SCK_SMALLOUTFB:  // 小号出去副本
		self->SmallOutFB(p, data, len);
		break;
	case SCK_GETXL:  // 获取项链
		self->GetXL(p, data, len);
		break;
	case SCK_NOYAOSI: // 大号没有钥匙
		self->m_iNoYaosi = 1;
		self->AskXLCount();
		break;
	case SCK_PICKUPITEM: // 捡物品
		self->PickUpItem(p, data, len);
		break;
	case SCK_REBORN: // 复活
		self->ReBorn(p, data, len);
		break;
	case SCK_OFFLINE: // 游戏已掉线
		self->OffLine(p, data, len);
		break;
	case SCK_CLOSE:
		self->OnClose(client, index);
		break;
	default:
		break;
	}
}

void GameServer::OnClose(SOCKET client, int index)
{
	char log[64];
	Account* p = self->m_pGame->GetAccountBySocket(client);
	printf("游戏关闭:%s %d\n", p ? p->Name : nullptr, client);
	LOGVARP(log, "游戏关闭:%s %d\n", p ? p->Name : nullptr, client);
	if (!p)
		p = self->m_pGame->GetReadyAccount();

	if (p && p->Flag == 1) {
		if (p->IsBig && p->OfflineLogin) {
			self->m_iOffLine++;
			self->m_pGame->UpdateOffLineAllText(self->m_iOffLine, self->m_iReBorn);
			self->m_pGame->WriteLog("已退出游戏\n-----------------------------------\n\n\n");
#if 0
			p->PlayTime = 0;
			self->m_pGame->SetStatus(p, ACCSTA_OFFLINE);
			self->m_pGame->m_pUIWnd->UpdateAccountStatus(p);
			LOGVARN(64, "%s[%s] 已退出游戏", p->Name, p->Role);
			return;
#endif
		}
		p->XL = 0;
		self->m_pGame->SetReady(p, 0);
		self->m_pGame->AddReadyCount(-1); // 已准备好帐号
		self->m_pGame->SetFlag(p, 0); // 标志已处理退出 

		if (self->m_pGame->m_Setting.LogoutByGetXL) { // 领完项链后退出
			self->m_pGame->SetCompleted(p);
		}

		if (self->m_pGame->CheckStatus(p, ACCSTA_COMPLETED)) {  // 已完成
			//printf("self->m_pGame->CheckStatus(p, ACCSTA_COMPLETED)\n");
			Account* next = self->m_pGame->GetNextLoginAccount();
			if (next) { // 自动登录下一个号
				printf("下一个要登录的帐号:%s:%d %08X\n", next->Name, next->Index, next->Status);
				self->m_pGame->SetLoginFlag(next->Index);
				self->m_pGame->AutoLogin("GameServer::OnClose:下一个");
			}
			else if (self->m_pGame->m_Setting.LogoutByGetXL && !self->m_pGame->m_Setting.NoPlayFB) { // 已领完
				self->m_pGame->m_Setting.LogoutByGetXL = 0;
				self->m_pGame->SetAllStatus(ACCSTA_INIT);
				self->m_pGame->SetLoginFlag(-1);
				self->m_pGame->AutoLogin("GameServer::OnClose::领项链");
			}
			return;
		}
		
		p->PlayTime = 0;
		self->m_pGame->SetStatus(p, ACCSTA_OFFLINE);
		self->m_pGame->UpdateAccountStatus(p);
		LOGVARP(log, "%s[%s] 已退出游戏", p->Name, p->Role);

		if (self->m_pGame->IsAutoLogin() && p->OfflineLogin && self->m_pGame->m_Setting.ReConnect) {
			self->m_pGame->SetLoginFlag(p->Index);
			self->m_pGame->AutoLogin("GameServer::OnClose::关闭后重新登录");
		}
	}
}
