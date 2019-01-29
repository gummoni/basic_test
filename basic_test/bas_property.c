//ハードウェア変化通知処理
#include "config.h"
#include "bas_property.h"

static char* notify_queue[10];
static byte notify_idx = 0;

//読込み専用(通知なし）
static int pos;						//現在位置（パルス）
static int spd;						//現在速度（速度）
static int enc;						//現在位置（エンコーダ）
static ushort ad_row_val;			//AD値（直値）
static ushort ad_kin1_val;			//AD1値（フィルタ１）
static ushort ad_kin2_val;			//AD2値（フィルタ２）
static ushort ad_kin3_val;			//AD3値（フィルタ３）

//読み書きOK（通知なし）
static byte torque;					//トルクON/OFF(0=OFF, 1=ON)
static byte power;					//最大出力(0-100, 0=0%, 100=100%)
static byte dir;					//方向(0=正転, 1=反転)
static char ad_kin1_filter[16];		//ADフィルタ1パラメータ
static char ad_kin2_filter[16];		//ADフィルタ2パラメータ
static char ad_kin3_filter[16];		//ADフィルタ3パラメータ

//読込み専用（通知あり）
static byte moving;					//移動中(0=停止、1=動作中)
static byte refl;					//原点センサ(0=原点スイッチOFF　　、1=原点スイッチON）
static byte refr;					//原点センサ(0=スイッチリミットOFF、1=スイッチリミットON）


//通知処理(変化があれば通知)
void bas_property_job(BAS_PACKET* packet)
{
	/*
	ushort _ad_val;
	ushort _status;
	ushort _pos;
	ushort _enc;

	byte _refl = (_status & 0x01) ? 1 : 0;
	byte _refr = (_status & 0x01) ? 1 : 0;
	byte _moving = (_status & 0x01) ? 1 : 0;

	bool change_moving = pos != _pos;
	bool change_refl = refl != _refl;
	bool change_refr = refr != _refr;

	pos = _pos;
	moving = _moving;
	refl = _refl;
	refr = _refr;
	ad_row_val = _ad_val;

	if (change_refl)
	{
		sprintf(hw_buff, "%s.REFL=%d", SELF_NAME, refl);
		make_message(SELF_NAME, SELF_NAME, NOTIFY, hw_buff);
	}
	if (change_refr)
	{
		sprintf(hw_buff, "%s.REFR=%d", SELF_NAME, refr);
		make_message(SELF_NAME, SELF_NAME, NOTIFY, hw_buff);
	}
	if (change_moving)
	{
		sprintf(hw_buff, "%s.MOVING=%d", SELF_NAME, moving);
		make_message(SELF_NAME, SELF_NAME, NOTIFY, hw_buff);
	}
	*/
}
