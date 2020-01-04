#include "Game.h"
#include "GameConf.h"
#include <ShlObj_core.h>
#include <stdio.h>
#include <memory.h>
#include <My/Common/mystring.h>
#include <My/Common/OpenTextFile.h>
#include <My/Common/Explode.h>

// ...
GameConf::GameConf(Game* p)
{
	m_pGame = p;

	memset(&m_stPetOut,  0, sizeof(m_stPetOut));
	memset(&m_stPickUp,  0, sizeof(m_stPickUp));
	memset(&m_stUse,     0, sizeof(m_stPickUp));
	memset(&m_stCheckIn, 0, sizeof(m_stCheckIn));
	memset(&m_stTrump,   0, sizeof(m_stTrump));
	memset(&m_Setting,   0, sizeof(m_Setting));
}

// 读取配置文件
bool GameConf::ReadConf(const char* path)
{
	char conf_file[255];
	strcpy(conf_file, path);
	strcat(conf_file, "\\配置.txt");
	printf("游戏配置文件:%s\n", conf_file);
	OpenTextFile file;
	if (!file.Open(conf_file)) {
		printf("找不到'%s'文件！！！\n", conf_file);
		return false;
	}

	bool pet = false, pickup = false, use = false, checkin = false, sell = false, trump = false;
	int length = 0;
	char data[128];
	while ((length = file.GetLine(data, 128)) > -1) {
		if (length == 0) {
			continue;
		}
		if (strstr(data, "=")) {
			trim(data);
			ReadSetting(data);
			continue;
		}
		//printf("%d %s", length, data);
		if (data[0] == '\n' || data[0] == '\r') {
			pet = false;
			pickup = false;
			use = false;
			checkin = false;
			sell = false;
			trump = false;
			continue;
		}
		if (strstr(data, "出征宠物")) {
			pet = true;
			continue;
		}
		if (strstr(data, "捡拾物品")) {
			pickup = true;
			continue;
		}
		if (strstr(data, "使用物品")) {
			use = true;
			continue;
		}
		if (strstr(data, "售卖物品") || strstr(data, "出售物品")) {
			sell = true;
			continue;
		}
		if (strstr(data, "存入物品")) {
		    checkin = true;
			continue;
		}
		if (strstr(data, "合法宝") || strstr(data, "合成法宝")) {
			trump = true;
			continue;
		}

		trim(data);
		if (pet) {
			ReadPetOut(data);
			continue;
		}
		if (pickup) {
			ReadPickUp(data);
			continue;
		}
		if (use) {
			ReadUse(data);
			continue;
		}
		if (sell) {
			ReadSell(data);
			continue;
		}
		if (checkin) {
			ReadCheckIn(data);
			continue;
		}
		if (trump) {
			ReadTrump(data);
			continue;
		}
	}

	file.Close();
	return true;
}

// 读取出征宠物
void GameConf::ReadPetOut(const char* data)
{
	if (m_stPetOut.Length == 3)
		return;

	DWORD length = m_stPetOut.Length;
	m_stPetOut.No[m_stPetOut.Length++] = atoi(data) - 1;
	printf("%d.出征宠物编号:%d\n", m_stPetOut.Length, m_stPetOut.No[length] + 1);
}

// 读取捡拾物品
void GameConf::ReadPickUp(const char * data)
{
	if (m_stPickUp.Length >= MAX_CONF_ITEMS)
		return;

	DWORD length = m_stPickUp.Length;
	int type = TransFormItemType(data);
	strcpy(m_stPickUp.PickUps[length].Name, data);
	m_stPickUp.PickUps[length].Type = type;
	m_stPickUp.Length++;

	printf("%d.自动捡拾物品:%s %08X\n", m_stPickUp.Length, data, type);
}

// 读取使用物品
void GameConf::ReadUse(const char * data)
{
	if (m_stUse.Length >= MAX_CONF_ITEMS)
		return;

	DWORD length = m_stUse.Length;
	int type = TransFormItemType(data);
	strcpy(m_stUse.Uses[length].Name, data);
	m_stUse.Uses[length].Type = type;
	m_stUse.Length++;

	printf("%d.自动使用物品:%s %08X\n", m_stUse.Length, data, type);
}

// 读取售卖物品
void GameConf::ReadSell(const char * data)
{
	if (m_stSell.Length >= MAX_CONF_ITEMS)
		return;

	DWORD length = m_stSell.Length;
	int type = TransFormItemType(data);
	strcpy(m_stSell.Sells[length].Name, data);
	m_stSell.Sells[length].Type = type;
	m_stSell.Length++;

	printf("%d.自动售卖物品:%s %08X\n", m_stSell.Length, data, type);
}

// 读取存入物品
void GameConf::ReadCheckIn(const char* data)
{
	if (m_stCheckIn.Length >= MAX_CONF_ITEMS)
		return;

	DWORD length = m_stCheckIn.Length;
	int type = TransFormItemType(data);
	strcpy(m_stCheckIn.CheckIns[length].Name, data);
	m_stCheckIn.CheckIns[length].Type = type;
	m_stCheckIn.Length++;

	printf("%d.自动存入物品:%s %08X\n", m_stCheckIn.Length, data, type);
}

// 读取合法宝
void GameConf::ReadTrump(const char* data)
{
	if (m_stTrump.Length >= MAX_CONF_ITEMS)
		return;

	Explode arr("-", data);
	DWORD length = m_stTrump.Length;
	strcpy(m_stTrump.Trumps[length].Name, arr[0]);
	m_stTrump.Trumps[length].Level = arr.GetValue2Int(1);
	m_stTrump.Length++;

	printf("%d.自动合法宝:%s %d\n", m_stCheckIn.Length, arr[0], arr.GetValue2Int(1));
}

// 读取其它设置
void GameConf::ReadSetting(const char * data)
{
	Explode explode("=", data);
	if (strcmp(explode[0], "流程文件") == 0) {
		if (m_Setting.FBFileCount == 16) {
			printf("提示:只支持16流程个文件\n");
			::MessageBoxA(NULL, "只支持16流程个文件", "提示", MB_OK);
			return;
		}
		strcpy(m_Setting.FBFile[m_Setting.FBFileCount], explode[1]);
		m_Setting.FBFileCount++;
	}
	printf("%s:%s\n", explode[0], explode[1]);
}

// 转成物品类型
int GameConf::TransFormItemType(const char * item_name)
{
	return 168;
}
