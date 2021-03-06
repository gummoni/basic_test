//=============================================================================
//電文解析
//=============================================================================
#include "config.h"
#include "bas.h"
#include "dic.h"
#include "usb.h"
#include "serial.h"
#include "bas_script.h"
#include "bas_protcol.h"
#include "bas_property.h"

//=============================================================================
//解析バッファ
//=============================================================================
static BAS_PACKET packet_up;	//上位側
static BAS_PACKET packet_dn;	//下位側

//BAS翻訳機の状態
BASIC_STATE state;

//通信解析用パラメータ
BAS_PARSER bas_parser;

//24文字x100行のプログラムエリア
char program_areas[PROGRAM_LINE_MAX][PROGRAM_LINE_COUNT];
/*
	//  0: UNIQUE_NAME（自分宛、返事を返す）
	//  1: LISTEN1(NOTIFY)
	//  2: LISTEN2(NOTIFY)
	//  3: LISTEN3(NOTIFY)
	//  4: LISTEN4(NOTIFY)
	//  5: LISTEN5(NOTIFY)
	//  6: LISTEN6(NOTIFY)
	//  7: LISTEN7(NOTIFY)
	//  8: LISTEN8(NOTIFY)
	//  9: LISTEN9(NOTIFY)
	// 10:プログラム領域以降すべて
*/

//=============================================================================
//初期化
//=============================================================================
void bas_init(void)
{
	//パラメータ初期化
	dic_clear();
	bas_script_init();

	int i;
	for (i = 0; i < PROGRAM_LINE_MAX; i++)
	{
		program_areas[i][0] = '\0';
	}
	//テスト用に設定>>>>>>>>>>>>>>>
	strcpy(program_areas[0], "AXIS_Z1");
	strcpy(program_areas[1], "AXIS_Z");
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	//辞書初期化
	dic_set("AD1PRM", "0 0 0 0");	// AD1フィルタ
	dic_set("AD2PRM", "0 0 0 0");	// AD2フィルタ
	dic_set("AD3PRM", "0 0 0 0");	// AD3フィルタ
	dic_set("LIMIT", "10000");		// 最大移動量
	dic_set("HAZUSI", "2000");		// 原点位置外しパルス
	dic_set("SPD_ORGL", "0 0 0 0");	// 原点復帰速度（高速）
	dic_set("SPD_ORGH", "0 0 0 0");	// 原点復帰速度（低速）
	dic_set("SPD_HIGH", "0 0 0 0");	// 速度（高速）
	dic_set("SPD_LOW", "0 0 0 0");	// 速度（低速）
	dic_set("SPD_SLOW", "0 0 0 0");	// 速度（最低速）
	dic_set("NOZLE", "S0.REFR");

	//上位通信用
	clear_packet(&packet_up);
	packet_up.recieve = serial_read;
	packet_up.response = serial_write;
	packet_up.broadcast = usb_write;
	
	//下位通信用
	clear_packet(&packet_dn);
	packet_dn.recieve = usb_read;
	packet_dn.response = usb_write;
	packet_dn.broadcast = serial_write;

	//スクリプト実行用
	script_packet.response = serial_write;
	script_packet.broadcast = usb_write;
}

//=============================================================================
//メッセージ処理
//=============================================================================
//TODO:データ取得は別ドライバで行う
void bas_job(void)
{
	bas_update_status(&packet_up);										//自発的通知
	bas_script_job();													//スクリプト処理
	bas_comm_job(&packet_up);											//上位通信ポート受信解析
	bas_comm_job(&packet_dn);											//下位通信ポート受信解析
}



