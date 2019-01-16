#ifndef __CSV_BASIC_H__
#define __CSV_BASIC_H__

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
	
	//�R�}���h���M	C$="100" �Ƃ����p�����[�^�𑗂�Ί�̒��Ŏ��s����B
	EVAL = 'E',
	EVAL_ACK = 'e',
	
	//�p�����[�^�l�擾
	GET_PARAM = 'P',
	GET_PARAM_ACK = 'p',
	
	//�ω��ʒm(�����I�ɑ��M)
	NOTIFY = 'N',

} COMMAND;

//�G���[�R�[�h
typedef enum
{
	err_invalid_return,		//���^�[���悪���݂��Ȃ�
	err_out_of_return,		//�q�[�v�̈�I�[�o�[
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
} BAS_PACKET;

//�R�}���h�e�[�u��
typedef struct
{
	char* name;
	bool(*execute)(BAS_PACKET*);
} BAS_PACKET_TABLE;

typedef struct
{
	char* name;
	void(*execute)(BAS_PACKET*);
} BAS_SCRIPT_TABLE;

typedef struct
{
	char* cmd;
	char* judge;
	char* jump_true;
	char* jump_false;
} SYNTAX_IF;

typedef struct
{
	char* cmd;
} SYNTAX_QUIT;

typedef struct
{
	char* eval;
} SYNTAX_EVAL;

typedef struct
{
	char* addr;
} SYNTAX_READ;

typedef struct
{
	char* addr;
	char* value;
} SYNTAX_WRITE;

typedef struct
{
	char* addr;
} SYNTAX_START;


typedef struct
{
	char* addr;
} SYNTAX_GET_PARAM;


extern void bas_main(char* data);


#endif//__CSV_BASIC_H__
