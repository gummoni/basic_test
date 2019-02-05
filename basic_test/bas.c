//=============================================================================
//�d�����
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
//��̓o�b�t�@
//=============================================================================
static BAS_PACKET packet_up;	//��ʑ�
static BAS_PACKET packet_dn;	//���ʑ�

//BAS�|��@�̏��
BASIC_STATE state;

//�ʐM��͗p�p�����[�^
BAS_PARSER bas_parser;

//24����x100�s�̃v���O�����G���A
char program_areas[PROGRAM_LINE_MAX][PROGRAM_LINE_COUNT];
/*
	//  0: UNIQUE_NAME�i�������A�Ԏ���Ԃ��j
	//  1: LISTEN1(NOTIFY)
	//  2: LISTEN2(NOTIFY)
	//  3: LISTEN3(NOTIFY)
	//  4: LISTEN4(NOTIFY)
	//  5: LISTEN5(NOTIFY)
	//  6: LISTEN6(NOTIFY)
	//  7: LISTEN7(NOTIFY)
	//  8: LISTEN8(NOTIFY)
	//  9: LISTEN9(NOTIFY)
	// 10:�v���O�����̈�ȍ~���ׂ�
*/

//=============================================================================
//������
//=============================================================================
void bas_init(void)
{
	//�p�����[�^������
	dic_clear();
	bas_script_init();

	int i;
	for (i = 0; i < PROGRAM_LINE_MAX; i++)
	{
		program_areas[i][0] = '\0';
	}
	strcpy(program_areas[0], "AXIS_Z1");
	strcpy(program_areas[1], "AXIS_Z");

	//��ʒʐM�p
	clear_packet(&packet_up);
	packet_up.recieve = serial_read;
	packet_up.response = serial_write;
	packet_up.broadcast = usb_write;
	
	//���ʒʐM�p
	clear_packet(&packet_dn);
	packet_dn.recieve = usb_read;
	packet_dn.response = usb_write;
	packet_dn.broadcast = serial_write;

	//�X�N���v�g���s�p
	script_packet.response = serial_write;
	script_packet.broadcast = usb_write;
}

//=============================================================================
//���b�Z�[�W����
//=============================================================================
//TODO:�f�[�^�擾�͕ʃh���C�o�ōs��
void bas_job(void)
{
	bas_property_job(&packet_up);										//�����I�ʒm
	bas_script_job();													//�X�N���v�g����
	bas_comm_job(&packet_up);											//��ʒʐM�|�[�g��M���
	bas_comm_job(&packet_dn);											//���ʒʐM�|�[�g��M���
}



