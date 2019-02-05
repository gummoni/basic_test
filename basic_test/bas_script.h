#ifndef __BAS_SCRIPT_H__
#define __BAS_SCRIPT_H__

//�R�}���h�ꗗ
typedef enum
{
	//A=0x41, a=0x61
	//�f�[�^�Ǎ��݁i�A�h���X����ǂݎ��j
	//-->SEND,RECV,R:10
	READ = 'R',
	READ_ACK = 'r',

	//�f�[�^�����݁i�A�h���X�ɏ������ށj
	WRITE = 'W',
	WRITE_ACK = 'w',

	//����J�n(�ԍ����w�肵�Ď��s)
	START = 'S',
	START_ACK = 's',

	//���쒆�f
	ABORT = 'A',
	ABORT_ACK = 'a',

	//�G���[����
	ERR_CLEAR = 'C',
	ERR_CLEAR_ACK = 'c',

	//�R�}���h���M	C$="100" �Ƃ����p�����[�^�𑗂�Ί�̒��Ŏ��s����B�A�ω��ʒm�����̃R�}���h
	INVOKE = 'I',
	INVOKE_ACK = 'i',

	//�t�@�[���Ǎ���
	LOAD = 'X',
	LOAD_ACK = 'x',

	//�t�@�[���ۑ�
	SAVE = 'O',
	SAVE_ACK = 'o',

} COMMAND;


//�R�}���h�e�[�u��
typedef struct
{
	char name;
	bool(*execute)(BAS_PACKET* context);
} BAS_COMM_TABLE;

//�R�}���h�e�[�u��
typedef struct
{
	char* name;
	bool(*execute)(BAS_PACKET*);
} BAS_SCRIPT_TABLE;


//BAS�|��@�̏��
extern BAS_PACKET script_packet;

//�֐��ꗗ
extern char* make_message(char* from, char* to, char cmd, char* message);
extern void bas_script_init(void);
extern bool bas_comm_execute(BAS_PACKET* packet);
extern void bas_script_job(void);

#endif//__BAS_SCRIPT_H__

