#ifndef __BAS_PROPERTY_H__
#define __BAS_PROPERTY_H__

//内部変数
extern bool TORQUE;		// false=OFF, true=ON
extern bool DIR;		// false=正転,true=逆転
extern byte POWER;		// 0=出力0%、100=出力100% --->将来的にはアンペア（A)表記にする
extern int POS;			// 現在位置
extern int SPD;			// 現在速度
extern int ENC;			// エンコーダ値
extern ushort AD0;		// AD値直値
extern ushort AD1;		// AD1フィルタ値
extern ushort AD2;		// AD2フィルタ値
extern ushort AD3;		// AD3フィルタ値
extern bool MOVING;		// 移動状態
extern bool REFL;		// 左原点
extern bool REFR;		// 右原点


//スクリプトから操作可能な関数一覧
extern void bas_property_init(void);
extern void bas_update_status(BAS_PACKET* packet);
extern void bas_set_speed(int l, int h, int up, int dn);
extern void bas_do_abs(int pls);
extern void bas_do_inc(int pls);
extern void bas_do_stop(void);
extern void bas_set_power(byte value);
extern void bas_set_torque(bool sw);
extern void bas_set_dir(bool value);


#endif//__BAS_PROPERTY_H__
