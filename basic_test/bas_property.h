#ifndef __BAS_PROPERTY_H__
#define __BAS_PROPERTY_H__

//�����ϐ�
extern bool TORQUE;		// false=OFF, true=ON
extern bool DIR;		// false=���],true=�t�]
extern byte POWER;		// 0=�o��0%�A100=�o��100% --->�����I�ɂ̓A���y�A�iA)�\�L�ɂ���
extern int POS;			// ���݈ʒu
extern int SPD;			// ���ݑ��x
extern int ENC;			// �G���R�[�_�l
extern ushort AD0;		// AD�l���l
extern ushort AD1;		// AD1�t�B���^�l
extern ushort AD2;		// AD2�t�B���^�l
extern ushort AD3;		// AD3�t�B���^�l
extern bool MOVING;		// �ړ����
extern bool REFL;		// �����_
extern bool REFR;		// �E���_


//�X�N���v�g���瑀��\�Ȋ֐��ꗗ
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
