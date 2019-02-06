#include "config.h"
#include "bas_property.h"
#include "bas_script.h"

//読込み専用（通知なし）
int POS;		// 現在位置
int SPD;		// 現在速度
int ENC;		// エンコーダ値
ushort AD0;		// AD値直値
ushort AD1;		// AD1フィルタ値
ushort AD2;		// AD2フィルタ値
ushort AD3;		// AD3フィルタ値

//読み書き可能：フラグ（通知なし）
bool TORQUE;	// false=OFF, true=ON
bool DIR;		// false=正転,true=逆転
byte POWER;		// 0=出力0%、100=出力100% --->将来的にはアンペア（A)表記にする
int LIMIT;		// 最大移動量
int HAZUSI;		// 原点外しパルス

//読込み専用（通知あり）
bool MOVING;	// 移動状態
bool REFL;		// 左原点
bool REFR;		// 右原点

//TORQUE=1
//POWER=100
//DIR=0
//AD1$="10,10,10"
//AD2$="10,10,10"
//AD3$="10,10,10"
//MOVING
//REFL
//REFR


//初期化
void bas_property_init(void)
{
	TORQUE = false;
	DIR = false;
	POWER = 0;
	POS = 0;
	SPD = 0;
	ENC = 0;
	AD0 = 0;
	AD1 = 0;
	AD2 = 0;
	AD3 = 0;
}

//パラメータ更新
void bas_update_status(BAS_PACKET* packet)
{
	//----TMC5130パラメータ----
	//現在位置取得
	//現在エンコーダ値取得
	//現在速度取得
	//AD値取得
	//AD1フィルタ値算出
	//AD2フィルタ値算出
	//AD3フィルタ値算出
	//移動状態
	//REFL
	//REFR
	//エラー

	/*
	char resp[PROGRAM_LINE_COUNT];
	//エラーをブロードキャスト
	sprintf(resp, "%s.REFL=%d", SELF_NAME, SELF_NAME, state.err_no);
	bas_script_broadcast(packet, SELF_NAME, resp);
	sprintf(resp, "%s.REFR=%d", SELF_NAME, SELF_NAME, state.err_no);
	bas_script_broadcast(packet, SELF_NAME, resp);
	sprintf(resp, "%s.MOVING=%d", SELF_NAME, SELF_NAME, state.err_no);
	bas_script_broadcast(packet, SELF_NAME, resp);
	*/
}

//速度設定
static void bas_set_speed(char* speed)
{
	BAS_PACKET packet;
	if (!csv_split(&packet, bas_parser.resp_buff, speed)) return;
	//TODO:速度書込み
}

static void bas_set_position(int pls)
{
	//ポジション書込み
}

//移動コマンド(絶対値)
void bas_do_abs(char* speed, int pls)
{
	bas_set_speed(speed);
	bas_set_position(pls);
}

//移動コマンド（相対値）
void bas_do_inc(char* speed, int pls)
{
	bas_set_speed(speed);
	bas_set_position(POS + pls);
}

//停止コマンド
void bas_do_stop(void)
{
	//TODO
}

//出力設定
void bas_set_power(byte value)
{
	POWER = value;
	//TODO
}

//トルクOFF/ON
void bas_set_torque(bool sw)
{
	TORQUE = sw;
	//TODO
}

//極性設定
void bas_set_dir(bool value)
{
	DIR = value;
	//TODO
}

