//�n�[�h�E�F�A�ω��ʒm����
#include "config.h"
#include "bas_property.h"

static char* notify_queue[10];
static byte notify_idx = 0;

//�Ǎ��ݐ�p(�ʒm�Ȃ��j
static int pos;						//���݈ʒu�i�p���X�j
static int spd;						//���ݑ��x�i���x�j
static int enc;						//���݈ʒu�i�G���R�[�_�j
static ushort ad_row_val;			//AD�l�i���l�j
static ushort ad_kin1_val;			//AD1�l�i�t�B���^�P�j
static ushort ad_kin2_val;			//AD2�l�i�t�B���^�Q�j
static ushort ad_kin3_val;			//AD3�l�i�t�B���^�R�j

//�ǂݏ���OK�i�ʒm�Ȃ��j
static byte torque;					//�g���NON/OFF(0=OFF, 1=ON)
static byte power;					//�ő�o��(0-100, 0=0%, 100=100%)
static byte dir;					//����(0=���], 1=���])
static char ad_kin1_filter[16];		//AD�t�B���^1�p�����[�^
static char ad_kin2_filter[16];		//AD�t�B���^2�p�����[�^
static char ad_kin3_filter[16];		//AD�t�B���^3�p�����[�^

//�Ǎ��ݐ�p�i�ʒm����j
static byte moving;					//�ړ���(0=��~�A1=���쒆)
static byte refl;					//���_�Z���T(0=���_�X�C�b�`OFF�@�@�A1=���_�X�C�b�`ON�j
static byte refr;					//���_�Z���T(0=�X�C�b�`���~�b�gOFF�A1=�X�C�b�`���~�b�gON�j


//�ʒm����(�ω�������Βʒm)
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
