#include "Game.h"
#include "Item.h"
#include "Talk.h"
#include "PrintScreen.h"
#include "GameProc.h"
#include <My/Common/C.h>

Item::Item(Game* p)
{
	m_pGame = p;
}

// 打开背包
bool Item::OpenBag()
{
	int x = MyRand(1120, 1130), y = MyRand(190, 200);
	printf("打开背包:(%d,%d)\n", x, y);
	m_pGame->m_pGameProc->Click(x, y);
	for (int i = 0; i < 2000; i += 150) {
		if (BagIsOpen())
			return true;

		Sleep(150);
	}

	m_pGame->m_pPrintScreen->GetPixelCount(0xFFB83700, 0x100, true);;
	printf("检测背包打开失败\n");
	Sleep(150);
	CloseBag();
	return false;
}

// 关闭背包
void Item::CloseBag()
{
	printf("关闭背包\n");
	m_pGame->m_pGameProc->Click(1236, 75, 1250, 80);
}

// 整理背包
void Item::SetBag()
{
	printf("整理背包\n");
	m_pGame->m_pGameProc->Click(1025, 666, 1125, 686);
}

// 滑动背包
bool Item::SlideBag(int page)
{
	if (page <= 0)
		return false;

	int slide_y = -400;
	int x = MyRand(700, 900), y = MyRand(350, 600);
	printf("滑动背包(%d):(%d,%d)\n", page, x, y);

#if 1
	m_pGame->m_pEmulator->Swipe(x, y, x, y + slide_y, m_pGame->m_pBig->Mnq->Index, 800);
#else
	int slide_y = -487;
	m_pGame->m_pGameProc->Click(x, y, 0x01);
	m_pGame->m_pGameProc->MouseMove(x, y, 0, slide_y);
	m_pGame->m_pGameProc->Click(x, y + slide_y, 0x02);
#endif
	return true;
}

// 打开仓库
void Item::OpenStorage()
{
	OpenBag();
	Sleep(1000);
	printf("打开仓库\n");
	m_pGame->m_pGameProc->Click(710, 670, 725, 686);
}

// 滑动仓库
bool Item::SlideStorge(int page)
{
	if (page <= 0)
		return false;

	int slide_y = -400;
	int x = MyRand(260, 360), y = MyRand(350, 600);

#if 1
	m_pGame->m_pEmulator->Swipe(x, y, x, y + slide_y, m_pGame->m_pBig->Mnq->Index, 800);
#else
	int slide_y = -487;
	m_pGame->m_pGameProc->Click(x, y, 0x01);
	m_pGame->m_pGameProc->MouseMove(x, y, 0, slide_y);
	m_pGame->m_pGameProc->Click(x, y + slide_y, 0x02);
#endif
	return true;
}

// 关闭仓库
void Item::CloseStorage()
{
	CloseBag();
}

// 加满血
void Item::AddFullLife()
{
	m_pGame->m_pGameData->ReadLife();
	if ((m_pGame->m_pGameData->m_dwLife+1000) >= m_pGame->m_pGameData->m_dwLifeMax)
		return;

	int num = (m_pGame->m_pGameData->m_dwLifeMax - m_pGame->m_pGameData->m_dwLife) / 2500 + 1;
	UseYao(num);
}

// 使用药
void Item::UseYao(int num, bool use_yaobao, int sleep_ms)
{
	//printf("使用药\n");
	for (int i = 0; i < num; i++) {
		m_pGame->m_pGameProc->Click(893, 590, 935, 627);
		Sleep(sleep_ms);
	}
	if (use_yaobao && IsNeedUseYaoBao())
		UseYaoBao();
}

// 使用药包包
void Item::UseYaoBao()
{
	//printf("使用药包\n");
	m_pGame->m_pGameProc->Click(910, 496, 955, 530);
	Sleep(300);
}

// 去副本门口
void Item::GoFBDoor()
{
	m_pGame->m_pGameProc->Click(893, 590, 935, 627); // 外围下面那格
}

// 去副本门口
void Item::GoShop()
{
	m_pGame->m_pGameProc->Click(910, 496, 955, 530); // 外围下面倒数第二
}

// 关闭商店
void Item::CloseShop()
{
	printf("关闭商店");
	m_pGame->m_pGameProc->Click(553, 85, 558, 90);
}

// 使用复活宠物灵药
void Item::UseLingYao()
{
	printf("使用速效圣兽灵药\n");
	m_pGame->m_pGameProc->Click(975, 430, 980, 435);
	Sleep(300);
}

// 是否需要使用药包
bool Item::IsNeedUseYaoBao()
{
	//FFB83800 FFB83700 FFA43700 FFA03301 FFA43F09 FF963301 FF8E3606 FFA14D1E
	//FFB33200 FFB83700 FFA03501 FFA93E07 FF943102 FF9C3B0C FFBA6536 FFC97749

	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_hWndBig, 917, 608, 925, 610);
	bool result = m_pGame->m_pPrintScreen->GetPixelCount(0xFFB83700, 0x100) == 0; // 有药的颜色
	m_pGame->m_pPrintScreen->Release();
	return result;
}

// 背包是否打开
bool Item::BagIsOpen()
{
	// 截取的是背包关闭按钮前面一点
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 1150, 75, 1160, 85, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("背包", nullptr, 1) > 0; // 打开会遮住按钮
}

// 物品操作按钮是否打开
bool Item::ItemBtnIsOpen(int index)
{
	// 截取物品操作按钮 
	if (index == 0)      // 第一行
		m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 145, 85, 155, 95, 0, true);
	else if (index == 1) // 第二行
		m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 145, 145, 155, 175, 0, true);
	else                 // 第三行
		m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 145, 215, 155, 245, 0, true);

	return m_pGame->m_pPrintScreen->ComparePixel("物品按钮", nullptr, 1) > 0;
}

// 等待物品操作按钮出现
bool Item::WaitForItemBtnOpen()
{
	for (int i = 0; i < 1000; i += 100) {
		if (ItemBtnIsOpen())
			return true;

		Sleep(100);
	}
	return false;
	//375,260 820,460 第一个BOSS区域扫描是否有宝宝药
}

// 拆分操作按钮是否出现
bool Item::ChaiFeiBtnIsOpen()
{
	// 截取拆分框取消按钮
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 466, 505, 476, 515, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("拆分框", nullptr, 1) > 0;
}

// 关闭拆分框
void Item::CloseChaiFeiBox()
{
	m_pGame->m_pGameProc->Click(466, 505, 566, 515);
}

// 捡物
int Item::PickUpItem(const char* name, int x, int y, int x2, int y2, int pickup_num)
{
	int count = 0;
	for (int i = 1; i <= pickup_num; i++) {
		int pos_x = 0, pos_y = 0;
		if (!GetGroundItemPos(name, x, y, x2, y2, pos_x, pos_y))
			break;

		printf("%d.捡东西:%s(%d,%d)\n", i, name, pos_x, pos_y);
		m_pGame->m_pGameProc->Click(pos_x, pos_y);
		WaitForPickUpItem();
		count++;

		Sleep(260);
	}

	return count;
}

// 等待捡起物品
int Item::WaitForPickUpItem(DWORD wait_ms)
{
	DWORD x = 0, y = 0;
	m_pGame->m_pGameData->ReadCoor(&x, &y, m_pGame->m_pGameProc->m_pAccount);
	for (DWORD i = 0; i <= wait_ms; i += 500) {
		Sleep(500);

		DWORD now_x = 0, now_y = 0;
		m_pGame->m_pGameData->ReadCoor(&now_x, &now_y, m_pGame->m_pGameProc->m_pAccount);
		if (x == now_x && y == now_y)
			return 1;

		x = now_x;
		y = now_y;
	}
	return 0;
}

// 获取地面物品坐标
int Item::GetGroundItemPos(const char* name, int x, int y, int x2, int y2, int& pos_x, int& pos_y)
{
	if (!x || !y || !x2 || !y2)
		return 0;

	//printf("捡物截图:%s(%d,%d %d,%d)\n", name, x, y, x2, y2);
	int tmp_x = 0, tmp_y = 0, tmp_x2 = 0, tmp_y2 = 0;
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, x, y, x2, y2, 0, true);
	if (strcmp("速效圣兽灵药", name) == 0) {
		if (m_pGame->m_pPrintScreen->GetPixelPos(0xFF72AA23, tmp_x, tmp_y, 0x00101010)) {
			m_pGame->m_pPrintScreen->GetPixelPos(0xFF528A03, tmp_x2, tmp_y2, 0x00101010);
		}
		//printf("%s:%d,%d %d,%d\n", name, tmp_x, tmp_y, tmp_x2, tmp_y2);
	}
	else if (strcmp("30星神兽碎片+3", name) == 0) {
		if (m_pGame->m_pPrintScreen->GetPixelPos(0xB885CA, tmp_x, tmp_y, 0x00080808)) {
			//m_pGame->m_pPrintScreen->GetPixelPos(0xF5C5FB, tmp_x2, tmp_y2, 0x00101010);
		}
		//printf("%s:%d,%d %d,%d\n", name, tmp_x, tmp_y, tmp_x2, tmp_y2);
	}
	if ((tmp_x && tmp_y) || (tmp_x2 && tmp_y2)) {
		//printf("发现物品:%s, 坐标:%d,%d 坐标2:%d,%d\n", name, tmp_x, tmp_y, tmp_x2, tmp_y2);

		if (tmp_x) {
			pos_x = tmp_x;
			pos_y = tmp_y;
		}
		else {
			pos_x = tmp_x2;
			pos_y = tmp_y2;
		}
			
		return 1;
	}

	return 0;
}

// 丢弃药包
int Item::DropItem(ComImgIndex index, int live_count, DWORD* ms)
{
	DWORD _tm = GetTickCount();

	if (!OpenBag())
		Sleep(500);

	Sleep(300);

	// 30星神兽碎片+2会使用不会丢弃 蓝色祝福碎片跟30星神兽碎片+2一个样 紫色祝福跟30星神兽碎片+3一个样
	char drops[][32] = {
		"四百点图鉴卡", "复仇骑士的挂坠", "钥匙",
		"卡利亚手记", "清凉的圣水", "巴力混沌发型包", "精英骑士纹章", "魔魂晶石",
		"30星神兽碎片+1", "30星神兽碎片+2", "紫色祝福碎片"
	};
	int drops_num = sizeof(drops)/ sizeof(drops[0]);
	int count = 0; // 数量
	for (int i = 0; i <= 5; i++) {
		if (!BagIsOpen() || m_pGame->m_pGameProc->m_bPause)
			break;

		if (SlideBag(i)) { // 滑动背包
			Sleep(200);
		}

		for (int idx = 0; idx < drops_num; idx++) {
			if (i > 0 && strcmp("紫色祝福碎片", drops[idx]) == 0)
				continue;

			ComPoint cp[32];
			PrintBagImg(true); // 截取背包图片
			int num = m_pGame->m_pPrintScreen->ComparePixel(drops[idx], cp, sizeof(cp) / sizeof(ComPoint));
			if (num == 0)
				continue;
			if (num == 1 && strcmp(drops[idx], "钥匙") == 0) // 钥匙只有一把不能丢
				continue;
			//m_pGame->m_pPrintScreen->SaveBitmapToFile(bitmap, L"C:\\Users\\fucan\\Desktop\\MNQ-9Star\\jt.bmp");
			//printf("%d.丢.%s.背包物品数量:%d\n", i + 1, drops[idx], num);

			int drop_num = 0;

			if (count >= live_count) { // 需要全部丢
				drop_num = num;
			}
			else {
				drop_num = count + num - live_count;
			}
			drop_num = strcmp(drops[idx], "钥匙") != 0 ? num : 1; // 钥匙只丢最前面那一把

			int btn_index = 1;
			if (strcmp("钥匙", drops[idx]) == 0 || strcmp("清凉的圣水", drops[idx]) == 0) // 只有一个操作按钮
				btn_index = 0;
			else if (strstr(drops[idx], "30星神兽碎片")) // 这个使用
				btn_index = 0;

			for (int j = 0; j < drop_num; j++) {
				if (j == 0) {
					if (strstr(drops[idx], "30星神兽碎片")) {
						UseItem(drops[idx], cp[0].x, cp[0].y);
					}
					else {
						DropItem(drops[idx], cp[0].x, cp[0].y, btn_index);
					}
				}
				else {
					ComPoint item[1];
					PrintBagImg(true); // 截取背包图片
					if (!m_pGame->m_pPrintScreen->ComparePixel(drops[idx], item, sizeof(item) / sizeof(ComPoint)))
						break;

					if (strstr(drops[idx], "30星神兽碎片")) {
						UseItem(drops[idx], item[0].x, item[0].y);
					}
					else {
						DropItem(drops[idx],item[0].x, item[0].y, btn_index);
					}
				}
				Sleep(600);

				if (strstr(drops[idx], "30星神兽碎片")) {
					m_pGame->m_pGameProc->CloseTipBox();
				}
				if (ChaiFeiBtnIsOpen()) {
					CloseChaiFeiBox();
					Sleep(300);
				}
			}
			count += num;

			m_pGame->m_pGameProc->CloseTipBox();
		}
		
		if (m_pGame->m_pPrintScreen->CompareImage(CIN_NoItem, nullptr, 1)) 
			break;
		if (m_pGame->m_pPrintScreen->CompareImage(CIN_LockItem, nullptr, 1)) 
			break;
	}

	if (ChaiFeiBtnIsOpen()) {
		CloseChaiFeiBox();
		Sleep(300);
	}

	Sleep(150);
	CloseBag();

	_tm = GetTickCount() - _tm;
	if (ms) {
		*ms = _tm;
	}
	
	printf("整理背包用时:%.2f秒, %d毫秒\n", (float)_tm/1000.0f, _tm);
	return count;
}

// 使用物品
void Item::UseItem(const char* name, int x, int y)
{
	printf("使用物品:%s(%d,%d)\n", name, x, y);
	m_pGame->m_pGameProc->Click(x, y);
	if (WaitForItemBtnOpen()) {
		Sleep(100);
		GetItemBtnPos(x, y, 0);
		m_pGame->m_pGameProc->Click(x, y); // 点击使用按钮
	}
}

// 丢弃物品
void Item::DropItem(const char* name, int x, int y, int index)
{
	printf("丢物品:%s(%d,%d)\n", name, x, y);
	m_pGame->m_pGameProc->Click(x, y);
	if (WaitForItemBtnOpen()) {
		Sleep(150);
		if (strcmp("紫色祝福碎片", name) == 0 && ItemBtnIsOpen(2)) // 有第三个按钮, 那么是30星神兽碎片+3
			return;
		if (strcmp("钥匙", name) != 0) {
			Sleep(100);
			// 点击物品出来操作按钮旁边的图标
			m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 320, 82, 375, 145, 0, true); 
			if (m_pGame->m_pPrintScreen->ComparePixel("钥匙", nullptr, 1) > 0) // 丢到了钥匙
				return;
		}

		GetItemBtnPos(x, y, index > -1 ? index : 1);
		m_pGame->m_pGameProc->Click(x, y); // 点击丢弃按钮
		if (m_pGame->m_pTalk->WaitForConfirmBtnOpen()) {
			Sleep(100);
			m_pGame->m_pTalk->ClickConfirmBtn(1); // 确定按钮
		}
	}	
}

// 售卖物品
int Item::SellItem(ConfItemInfo* items, DWORD length)
{
	for (int i = 0; i <= 5; i++) {
		if (!BagIsOpen() || m_pGame->m_pGameProc->m_bPause)
			break;

		if (SlideBag(i)) { // 滑动背包
			Sleep(100);
		}

		for (DWORD iti = 0; iti < length; iti++) {
			for (int sell_i = 0; sell_i < 10; sell_i++) {
				ComPoint cp[32];
				PrintBagImg(true); // 截取背包图片
				int num = m_pGame->m_pPrintScreen->ComparePixel(items[iti].Name, cp, sizeof(cp) / sizeof(ComPoint));
				//printf("%d.售卖.背包物品数量:%d\n", i + 1, num);
				if (num == 0)
					break;

				for (int j = 0; j < num; j++) {
					if (j == 0) {
						SellItem(cp[0].x, cp[0].y);
						printf("%d.售卖.物品:%s(%d,%d)\n", j + 1, items[iti].Name, cp[j].x, cp[j].y);
					}
					else {
						ComPoint item[1];
						PrintBagImg(true); // 截取背包图片
						if (!m_pGame->m_pPrintScreen->ComparePixel(items[iti].Name, item, sizeof(item) / sizeof(ComPoint)))
							break;

						SellItem(item[0].x, item[0].y);
						printf("%d.售卖.物品:%s(%d,%d)\n", j + 1, items[iti].Name, item[0].x, item[0].y);
					}
					Sleep(600);
				}
			}
		}

		if (m_pGame->m_pPrintScreen->CompareImage(CIN_NoItem, nullptr, 1))
			break;
		if (m_pGame->m_pPrintScreen->CompareImage(CIN_LockItem, nullptr, 1))
			break;
	}

	return 0;
}

// 售卖物品
void Item::SellItem(int x, int y)
{
	//printf("卖物品:%d,%d\n", x, y);
	m_pGame->m_pGameProc->Click(x, y);
	if (WaitForItemBtnOpen()) {
		Sleep(100);
		GetItemBtnPos(x, y, 0);
		m_pGame->m_pGameProc->Click(x, y); // 点击丢弃按钮
		if (m_pGame->m_pTalk->WaitForConfirmBtnOpen()) {
			Sleep(100);
			m_pGame->m_pTalk->ClickConfirmBtn(1); // 确定按钮
		}
	}
}

// 获取物品操作按钮位置
void Item::GetItemBtnPos(int& x, int& y, int index)
{
	if (index == 0) {
		x = MyRand(180, 200);
		y = MyRand(86, 100);
		return;
	}
	if (index == 1) {
		x = MyRand(180, 200);
		y = MyRand(155, 168);
		return;
	}
}

// 存入仓库
void Item::CheckIn(ConfItemInfo* items, DWORD length)
{
	printf("存物\n");
	DWORD _tm = GetTickCount();

	OpenStorage();
	Sleep(1000);

	int pet_yao_1 = 0, pet_yao_2 = 1;
	for (int i = 0; i <= 5; i++) {
		if (!BagIsOpen() || m_pGame->m_pGameProc->m_bPause)
			break;

		m_pGame->m_pGameProc->CloseTipBox();
		if (SlideBag(i)) { // 滑动背包
			Sleep(300);
		}

		if (!pet_yao_1) {
			PrintBagImg(true); // 截取背包图片
			pet_yao_1 = m_pGame->m_pPrintScreen->ComparePixel("速效圣兽灵药", nullptr, 1) > 0;
		}
		
		bool is_save = false; // 是否存了东西
		for (DWORD iti = 0; iti < length; iti++) {
			ComPoint cp[32];
			PrintBagImg(true); // 截取背包图片
			int num = m_pGame->m_pPrintScreen->ComparePixel(items[iti].Name, cp, sizeof(cp) / sizeof(ComPoint));
			//printf("%d.存物.背包物品数量:%d\n", i + 1, num);
			if (num > 0) {
				for (int j = 0; j < num; j++) {
					if (j == 0) {
						m_pGame->m_pGameProc->Click(cp[0].x, cp[0].y);
						printf("%d.存物.%s:(%d,%d)\n", j + 1, items[iti].Name, cp[j].x, cp[j].y);
					}
					else {
						ComPoint item[1];
						PrintBagImg(true); // 截取背包图片
						if (!m_pGame->m_pPrintScreen->ComparePixel(items[iti].Name, item, sizeof(item) / sizeof(ComPoint)))
							break;

						m_pGame->m_pGameProc->Click(item[0].x, item[0].y);
						printf("%d.存物.%s:(%d,%d)\n", j + 1, items[iti].Name, item[0].x, item[0].y);
					}
					Sleep(800);
				}
				is_save = true;
			}
		}

		if (is_save && pet_yao_1) {
			PrintBagImg(true); // 截取背包图片
			pet_yao_2 = m_pGame->m_pPrintScreen->ComparePixel("速效圣兽灵药", nullptr, 1);
		}

		if (m_pGame->m_pPrintScreen->CompareImage(CIN_NoItem, nullptr, 1))
			break;
		if (m_pGame->m_pPrintScreen->CompareImage(CIN_LockItem, nullptr, 1))
			break;
	}
	if (pet_yao_1 && !pet_yao_2) {
		printf("\n======速效圣兽灵药不见了, 需要取出来======\n\n");
		CheckOutOne("速效圣兽灵药", false, false);
	}

	Sleep(150);
	CloseStorage();

	_tm = GetTickCount() - _tm;
	printf("存物用时:%.2f秒, %d毫秒\n", (float)_tm / 1000.0f, _tm);
}

// 取出仓库
int Item::CheckOut(ConfItemInfo* items, DWORD length)
{
	printf("取物\n");
	DWORD _tm = GetTickCount();

	OpenStorage();
	Sleep(1000);

	int count = 0;

	for (int i = 0; i <= 5; i++) {
		if (SlideStorge(i)) { // 滑动仓库
			Sleep(300);
		}

		for (DWORD iti = 0; iti < length; iti++) {
			ComPoint cp[32];
			PrintStorageImg(true); // 截取仓库图片
			int num = m_pGame->m_pPrintScreen->ComparePixel(items[iti].Name, cp, sizeof(cp) / sizeof(ComPoint));
			count += num;
			//printf("%d.取物.仓库物品数量:%d\n", i + 1, num);
			if (num > 0) {
				for (int j = 0; j < num; j++) {
					if (j == 0) {
						m_pGame->m_pGameProc->Click(cp[0].x, cp[0].y);
						printf("%d.取物.(%s:%d,%d)\n", j + 1, items[iti].Name, cp[j].x, cp[j].y);
					}
					else {
						ComPoint item[1];
						PrintStorageImg(true); // 截取仓库图片
						if (!m_pGame->m_pPrintScreen->ComparePixel(items[iti].Name, item, sizeof(item) / sizeof(ComPoint)))
							break;

						m_pGame->m_pGameProc->Click(item[0].x, item[0].y);
						printf("%d.取物.%s:(%d,%d)\n", j + 1, items[iti].Name, item[0].x, item[0].y);
					}
					Sleep(800);
				}
			}
		}

		if (m_pGame->m_pPrintScreen->CompareImage(CIN_NoItem, nullptr, 1))
			break;
		if (m_pGame->m_pPrintScreen->CompareImage(CIN_LockItem, nullptr, 1))
			break;
	}

	Sleep(150);
	CloseStorage();

	_tm = GetTickCount() - _tm;
	printf("取物用时:%.2f秒, %d毫秒\n", (float)_tm / 1000.0f, _tm);

	return count;
}

// 取出一个仓库物品
int Item::CheckOutOne(const char * name, bool open, bool close)
{
	if (open) {
		OpenStorage();
		Sleep(1000);
	}
	
	int count = 0;
	for (int i = 0; i <= 5; i++) {
		if (SlideStorge(i)) { // 滑动仓库
			Sleep(300);
		}

		ComPoint cp[1];
		PrintStorageImg(true); // 截取仓库图片
		if (m_pGame->m_pPrintScreen->ComparePixel(name, cp, 1)) {
			printf("取物.%s:(%d,%d)\n", name, cp[0].x, cp[0].y);
			m_pGame->m_pGameProc->Click(cp[0].x, cp[0].y);
			count = 1;
		}

		if (m_pGame->m_pPrintScreen->CompareImage(CIN_NoItem, nullptr, 1))
			break;
		if (m_pGame->m_pPrintScreen->CompareImage(CIN_LockItem, nullptr, 1))
			break;
	}

	if (close) {
		Sleep(150);
		CloseStorage();
	}

	return count;
}

// 获取背包物品数量
int Item::GetBagCount(ComImgIndex index)
{
	DWORD _tm = GetTickCount();

	OpenBag();
	Sleep(1000);

	int count = 0; // 数量
	for (int i = 0; i <= 3; i++) {
		if (SlideBag(i)) { // 滑动背包
			Sleep(300);
		}

		PrintBagImg(true); // 截取背包图片
		int num = m_pGame->m_pPrintScreen->CompareImage(index);
		printf("%d.背包物品数量:%d\n", i + 1, num);

		count += num;
		if (count > 0 && num == 0) // 后面已没物品
			break;

		if (m_pGame->m_pPrintScreen->CompareImage(CIN_NoItem, nullptr, 1))
			break;
		if (m_pGame->m_pPrintScreen->CompareImage(CIN_LockItem, nullptr, 1))
			break;
	}

	Sleep(500);
	CloseBag();

	_tm = GetTickCount() - _tm;
	printf("获取物品用时:%.2f秒, %d毫秒\n", (float)_tm / 1000.0f, _tm);
	return count;
}

// 获取快捷栏物品数量
int Item::GetQuickYaoOrBaoNum(int& yaobao, int& yao)
{
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_hWndBig, 915, 520, 970, 640, 0, true);
	yaobao = m_pGame->m_pPrintScreen->LookNum(25, 0, 0, 25, 0xffffffff, 0x003A3A3A);
	yao = m_pGame->m_pPrintScreen->LookNum(0, 0, 95, 0, 0xffffffff, 0x003A3A3A);
	//printf("药包数量:%d 药数量:%d\n", yaobao, yao);

	return 0;
}

// 切换到技能快捷栏
void Item::SwitchMagicQuickBar()
{
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_hWndBig, 1235, 350, 1245, 360, 0, true);
	if (m_pGame->m_pPrintScreen->ComparePixel("快捷栏切换按钮", nullptr, 1) == 0) { // 可能是XP技能栏
		SwitchQuickBar(1);
		Sleep(500);
	}

	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_hWndBig, 926, 510, 936, 520, 0, true);
	if (m_pGame->m_pPrintScreen->ComparePixel("星辰之眼快捷栏", nullptr, 1)) { // 切换
		SwitchQuickBar(1);
		Sleep(500);
	}
}

// 快捷栏是否有星辰之眼
bool Item::QuickBarIsXingChen()
{
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_hWndBig, 908, 602, 923, 617, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("星辰之眼") > 0;
}

// 切换快捷栏
void Item::SwitchQuickBar(int page)
{
	printf("切换到快捷栏:%d\n", page);
	m_pGame->m_pGameProc->Click(1235, 360, 1250, 370);
}

// 获取物品的ComImgIndex
ComImgIndex Item::GetItemComImgIndex(const char * name)
{
	if (strcmp(name, "双子星青螭礼包") == 0)
		return CIN_QingChi;
	if (strcmp(name, "双子星紫冥礼包") == 0)
		return CIN_ZiMing;
	if (strcmp(name, "白羊星卡迪礼包") == 0)
		return CIN_KaDi;
	if (strcmp(name, "白羊星穆巴礼包") == 0)
		return CIN_MuBa;
	if (strcmp(name, "金牛星亚尔礼包") == 0)
		return CIN_YaEr;
	if (strcmp(name, "金牛星鲁迪礼包") == 0)
		return CIN_LuDi;
	if (strcmp(name, "幻魔晶石") == 0)
		return CIN_HuanMo;
	if (strcmp(name, "魔魂晶石") == 0)
		return CIN_MoHun;
	if (strcmp(name, "灵魂晶石") == 0)
		return CIN_LingHun;
	if (strcmp(name, "爱娜的项链") == 0)
		return CIN_XiangLian;

	return CIN_NoItem;
}

// 截取背包图片
HBITMAP Item::PrintBagImg(bool del)
{
	HBITMAP bitmap = m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 672, 158, 1170, 620);
	if (del) {
		m_pGame->m_pPrintScreen->GetPixel(0, 0);
		DeleteBagImg();
	}
	return bitmap;
}

// 删除背包图片
void Item::DeleteBagImg()
{
	m_pGame->m_pPrintScreen->Release();
}

// 截取仓库图片
HBITMAP Item::PrintStorageImg(bool del)
{
	HBITMAP bitmap = m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_hWndBig, 113, 158, 620, 620);
	if (del) {
		m_pGame->m_pPrintScreen->GetPixel(0, 0);
		DeleteBagImg();
	}
	return bitmap;
}

// 删除仓库图片
void Item::DeleteStorageImg()
{
	m_pGame->m_pPrintScreen->Release();
}
