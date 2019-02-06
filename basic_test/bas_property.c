#include "config.h"
#include "bas_property.h"
#include "bas_script.h"

//�Ǎ��ݐ�p�i�ʒm�Ȃ��j
int POS;		// ���݈ʒu
int SPD;		// ���ݑ��x
int ENC;		// �G���R�[�_�l
ushort AD0;		// AD�l���l
ushort AD1;		// AD1�t�B���^�l
ushort AD2;		// AD2�t�B���^�l
ushort AD3;		// AD3�t�B���^�l

//�ǂݏ����\�F�t���O�i�ʒm�Ȃ��j
bool TORQUE;	// false=OFF, true=ON
bool DIR;		// false=���],true=�t�]
byte POWER;		// 0=�o��0%�A100=�o��100% --->�����I�ɂ̓A���y�A�iA)�\�L�ɂ���

//�Ǎ��ݐ�p�i�ʒm����j
bool MOVING;	// �ړ����
bool REFL;		// �����_
bool REFR;		// �E���_

//TORQUE=1
//POWER=100
//DIR=0
//AD1$="10,10,10"
//AD2$="10,10,10"
//AD3$="10,10,10"
//MOVING
//REFL
//REFR


//������
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

//�p�����[�^�X�V
void bas_update_status(BAS_PACKET* packet)
{
	//----TMC5130�p�����[�^----
	//���݈ʒu�擾
	//���݃G���R�[�_�l�擾
	//���ݑ��x�擾
	//AD�l�擾
	//AD1�t�B���^�l�Z�o
	//AD2�t�B���^�l�Z�o
	//AD3�t�B���^�l�Z�o
	//�ړ����
	//REFL
	//REFR
	//�G���[

	/*
	char resp[PROGRAM_LINE_COUNT];
	//�G���[���u���[�h�L���X�g
	sprintf(resp, "%s.REFL=%d", SELF_NAME, SELF_NAME, state.err_no);
	bas_script_broadcast(packet, SELF_NAME, resp);
	sprintf(resp, "%s.REFR=%d", SELF_NAME, SELF_NAME, state.err_no);
	bas_script_broadcast(packet, SELF_NAME, resp);
	sprintf(resp, "%s.MOVING=%d", SELF_NAME, SELF_NAME, state.err_no);
	bas_script_broadcast(packet, SELF_NAME, resp);
	*/
}

//���x�ݒ�
void bas_set_speed(int l, int h, int up, int dn)
{
}

//�ړ��R�}���h
void bas_do_abs(int pls)
{
	//TODO
}

//�ړ��R�}���h
void bas_do_inc(int pls)
{
	//TODO
}

//��~�R�}���h
void bas_do_stop(void)
{
}

//�o�͐ݒ�
void bas_set_power(byte value)
{
	POWER = value;
	//TODO
}

//�g���NOFF/ON
void bas_set_torque(bool sw)
{
	TORQUE = sw;
	//TODO
}

//�ɐ��ݒ�
void bas_set_dir(bool value)
{
	DIR = value;
	//TODO
}

