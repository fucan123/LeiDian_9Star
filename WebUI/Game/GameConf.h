#pragma once
#include "GameData.h"
#include <Windows.h>

#define MAX_CONF_ITEMS   16

class Game;
class GameConf
{
public:
	// ... 
	GameConf(Game* p);
	// 读取配置文件
	bool ReadConf(const char* path);
private:
	// 读取出征宠物
	void ReadPetOut(const char* data);
	// 读取捡拾物品
	void ReadPickUp(const char* data);
	// 读取使用物品
	void ReadUse(const char* data);
	// 读取售卖物品
	void ReadSell(const char* data);
	// 读取存入物品
	void ReadCheckIn(const char* data);
	// 读取合法宝
	void ReadTrump(const char* data);
	// 读取其它设置
	void ReadSetting(const char* data);
	// 转成物品类型
	int TransFormItemType(const char* item_name);
public:
	Game* m_pGame;

	// 出征宠物编号
	struct {
		DWORD No[3];
		DWORD Length;
	} m_stPetOut;

	// 捡拾物品列表
	struct {
		ConfItemInfo PickUps[MAX_CONF_ITEMS];
		DWORD        Length;
	} m_stPickUp;

	// 使用物品列表
	struct {
		ConfItemInfo Uses[MAX_CONF_ITEMS];
		DWORD        Length;
	} m_stUse;

	// 售卖物品列表
	struct {
		ConfItemInfo Sells[MAX_CONF_ITEMS];
		DWORD        Length;
	} m_stSell;

	// 存入物品列表
	struct {
		ConfItemInfo CheckIns[MAX_CONF_ITEMS];
		DWORD        Length;
	} m_stCheckIn;

	// 合成法宝列表
	struct {
		ConfItemInfo Trumps[MAX_CONF_ITEMS];
		DWORD        Length;
	} m_stTrump;

	// 其他设置
	struct {
		char FBFile[16][64]; // 副本流程文件
		int  FBFileCount;    // 共有多少文件
	} m_Setting;
};