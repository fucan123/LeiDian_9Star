#pragma once
#include "GameData.h"
#include <Windows.h>

enum ComImgIndex;
class Game;
class Item
{
public:
	Item(Game* p);

	// 打开背包
	bool OpenBag();
	// 关闭背包
	void CloseBag();
	// 整理背包
	void SetBag();
	// 滑动背包
	bool SlideBag(int page);
	// 打开仓库
	void OpenStorage();
	// 滑动仓库
	bool SlideStorge(int page);
	// 关闭仓库
	void CloseStorage();
	// 加满血
	void AddFullLife();
	// 使用药
	void UseYao(int num=1, bool use_yaobao=true, int sleep_ms=500);
	// 使用药包包
	void UseYaoBao();
	// 去副本门口
	void GoFBDoor();
	// 去商店
	void GoShop();
	// 关闭商店
	void CloseShop();
	// 使用复活宠物灵药
	void UseLingYao();
	// 是否需要使用药包
	bool IsNeedUseYaoBao();
	// 背包是否打开
	bool BagIsOpen();
	// 物品操作按钮是否打开
	bool ItemBtnIsOpen(int index=0);
	// 等待物品操作按钮出现
	bool WaitForItemBtnOpen();
	// 拆分操作按钮是否出现
	bool ChaiFeiBtnIsOpen();
	// 关闭拆分框
	void CloseChaiFeiBox();
	// 捡物
	int PickUpItem(const char* name, int x, int y, int x2, int y2, int pickup_num=10);
	// 等待捡起物品
	int WaitForPickUpItem(DWORD wait_ms=3500);
	// 获取地面物品坐标
	int GetGroundItemPos(const char* name, int x, int y, int x2, int y2, int& pos_x, int& pos_y);
	// 丢弃药包
	int  DropItem(ComImgIndex index, int live_count=6, DWORD* ms=nullptr);
	// 使用物品
	void UseItem(const char* name, int x, int y);
	// 丢弃物品
	void DropItem(const char* name, int x, int y, int index=-1);
	// 售卖物品
	int  SellItem(ConfItemInfo* items, DWORD length);
	// 售卖物品
	void SellItem(int x, int y);
	// 获取物品操作按钮位置
	void GetItemBtnPos(int& x, int& y, int index);
	// 存入仓库
	void CheckIn(ConfItemInfo* items, DWORD length);
	// 取出仓库
	int CheckOut(ConfItemInfo* items, DWORD length);
	// 取出一个仓库物品
	int CheckOutOne(const char* name, bool open=false, bool close=false);
	// 获取背包物品数量
	int GetBagCount(ComImgIndex index);
	// 获取快捷栏物品数量
	int GetQuickYaoOrBaoNum(int& yaobao, int& yao);
	// 切换到技能快捷栏
	void SwitchMagicQuickBar();
	// 快捷栏是否有星辰之眼
	bool QuickBarIsXingChen();
	// 切换快捷栏
	void SwitchQuickBar(int page);
	// 获取物品的ComImgIndex
	ComImgIndex GetItemComImgIndex(const char* name);
	// 截取背包图片
	HBITMAP PrintBagImg(bool del=false);
	// 删除背包图片
	void    DeleteBagImg();
	// 截取仓库图片
	HBITMAP PrintStorageImg(bool del=false);
	// 删除仓库图片
	void    DeleteStorageImg();
public:
	// Game类
	Game* m_pGame;
};