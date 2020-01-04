#pragma once
#include "GameData.h"
#include <Windows.h>

enum ComImgIndex;
class Game;
class Item
{
public:
	Item(Game* p);

	// �򿪱���
	bool OpenBag();
	// �رձ���
	void CloseBag();
	// ��������
	void SetBag();
	// ��������
	bool SlideBag(int page);
	// �򿪲ֿ�
	void OpenStorage();
	// �����ֿ�
	bool SlideStorge(int page);
	// �رղֿ�
	void CloseStorage();
	// ����Ѫ
	void AddFullLife();
	// ʹ��ҩ
	void UseYao(int num=1, bool use_yaobao=true, int sleep_ms=500);
	// ʹ��ҩ����
	void UseYaoBao();
	// ȥ�����ſ�
	void GoFBDoor();
	// ȥ�̵�
	void GoShop();
	// �ر��̵�
	void CloseShop();
	// ʹ�ø��������ҩ
	void UseLingYao();
	// �Ƿ���Ҫʹ��ҩ��
	bool IsNeedUseYaoBao();
	// �����Ƿ��
	bool BagIsOpen();
	// ��Ʒ������ť�Ƿ��
	bool ItemBtnIsOpen(int index=0);
	// �ȴ���Ʒ������ť����
	bool WaitForItemBtnOpen();
	// ��ֲ�����ť�Ƿ����
	bool ChaiFeiBtnIsOpen();
	// �رղ�ֿ�
	void CloseChaiFeiBox();
	// ����
	int PickUpItem(const char* name, int x, int y, int x2, int y2, int pickup_num=10);
	// �ȴ�������Ʒ
	int WaitForPickUpItem(DWORD wait_ms=3500);
	// ��ȡ������Ʒ����
	int GetGroundItemPos(const char* name, int x, int y, int x2, int y2, int& pos_x, int& pos_y);
	// ����ҩ��
	int  DropItem(ComImgIndex index, int live_count=6, DWORD* ms=nullptr);
	// ʹ����Ʒ
	void UseItem(const char* name, int x, int y);
	// ������Ʒ
	void DropItem(const char* name, int x, int y, int index=-1);
	// ������Ʒ
	int  SellItem(ConfItemInfo* items, DWORD length);
	// ������Ʒ
	void SellItem(int x, int y);
	// ��ȡ��Ʒ������ťλ��
	void GetItemBtnPos(int& x, int& y, int index);
	// ����ֿ�
	void CheckIn(ConfItemInfo* items, DWORD length);
	// ȡ���ֿ�
	int CheckOut(ConfItemInfo* items, DWORD length);
	// ȡ��һ���ֿ���Ʒ
	int CheckOutOne(const char* name, bool open=false, bool close=false);
	// ��ȡ������Ʒ����
	int GetBagCount(ComImgIndex index);
	// ��ȡ�������Ʒ����
	int GetQuickYaoOrBaoNum(int& yaobao, int& yao);
	// �л������ܿ����
	void SwitchMagicQuickBar();
	// ������Ƿ����ǳ�֮��
	bool QuickBarIsXingChen();
	// �л������
	void SwitchQuickBar(int page);
	// ��ȡ��Ʒ��ComImgIndex
	ComImgIndex GetItemComImgIndex(const char* name);
	// ��ȡ����ͼƬ
	HBITMAP PrintBagImg(bool del=false);
	// ɾ������ͼƬ
	void    DeleteBagImg();
	// ��ȡ�ֿ�ͼƬ
	HBITMAP PrintStorageImg(bool del=false);
	// ɾ���ֿ�ͼƬ
	void    DeleteStorageImg();
public:
	// Game��
	Game* m_pGame;
};