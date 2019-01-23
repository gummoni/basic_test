#ifndef __BAS_PACKET_H__
#define __BAS_PACKET_H__

//	�R�}���h�t�H�[�}�b�g
//	[����],[����],[CMD]:[ADDR],([PRM1],[PRM2])[\n]

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
	NOTIFY = 'N',
	NOTIFY_ACK = 'n',

	//�X�e�[�^�X�擾
	STATUS = 'Z',
	STATUS_ACK = 'z',

	//�t�@�[���Ǎ���
	LOAD = 'X',
	LOAD_ACK = 'x',

	//�t�@�[���ۑ�
	SAVE = 'O',
	SAVE_ACK = 'o',

} COMMAND;

//�G���[�R�[�h
typedef enum
{
	err_none,				//�G���[����
	err_invalid_return,		//���^�[���悪���݂��Ȃ�
	err_out_of_return,		//�q�[�v�̈�I�[�o�[
	err_jump,				//�W�����v���s
} error_code;

//������
typedef struct
{
	int run_no;
	error_code err_code;
} BASIC_STATE;

//�p�P�b�g�\����
typedef struct
{
	int listen_id;
	char* sender;
	char* reciever;
	char command;
	char* opcode;	//���[�h�|�C���^(�R�}���h)
	char* prm1;		//���[�h�|�C���^1
	char* prm2;		//���[�h�|�C���^2
	char* prm3;		//���[�h�|�C���^3
	char* prm4;		//���[�h�|�C���^4
	char* response;	//�ԐM�p���b�Z�[�W�|�C���^
} BAS_PACKET;

typedef struct
{
	char* opcode;	//���[�h�|�C���^(�R�}���h)
	char* prm1;		//���[�h�|�C���^1
	char* prm2;		//���[�h�|�C���^2
	char* prm3;		//���[�h�|�C���^3
	char* prm4;		//���[�h�|�C���^4
	char* response;	//�ԐM�p���b�Z�[�W�|�C���^
} BAS_PACKET_BODY;

//�R�}���h�e�[�u��
typedef struct
{
	char name;
	bool(*execute)(BAS_PACKET_BODY* context);
} BAS_PACKET_TABLE;

typedef struct
{
	char* name;
	void(*execute)(BAS_PACKET*);
} BAS_SCRIPT_TABLE;

//�v���O�����̈�
#define PROGRAM_LINE_COUNT	32
#define PROGRAM_LINE_MAX	200
extern char program_areas[PROGRAM_LINE_MAX][PROGRAM_LINE_COUNT];
#define SELF_NAME	program_areas[0]

//BAS�|��@�̏��
extern BASIC_STATE state;

extern void bas_send_message(char* from, char* to, char cmd, char* message);
extern bool bas_parse_parameter(BAS_PACKET* packet, char* msg, char separator);
extern bool bas_comm_parse(BAS_PACKET* packet, char* msg);

#endif//__BAS_PACKET_H__

