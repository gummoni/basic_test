#ifndef __UTIL_H__
#define __UTIL_H__

//	�R�}���h�t�H�[�}�b�g
//	[����],[����],[CMD]:[ADDR],([PRM1],[PRM2])[\n]

//�v���O�����̈�
#define PROGRAM_LINE_COUNT	32
#define PROGRAM_LINE_MAX	200
extern char program_areas[PROGRAM_LINE_MAX][PROGRAM_LINE_COUNT];
#define SELF_NAME	program_areas[0]
#define BUFF_COUNT	(PROGRAM_LINE_COUNT * 2)

//�ʐM��͗p�p�����[�^�i���ʁj
typedef struct
{
	char resp_buff[BUFF_COUNT];			//�ԐM�p�o�b�t�@
	char parse_buff[BUFF_COUNT];		//��͗p�o�b�t�@
	char* result_buff;					//���ʊi�[�|�C���^
	char parse_length;					//��M�f�[�^��
	bool is_quot;						//�������͒�(�_�u���N�E�H�[�e�[�V����)
	int state;							//�p�P�b�g��̓X�e�[�g	0=���M��, 1=��M��, 2=�R�}���h, 3=�R�}���h��؂�, 4=opcode, 5=prm1, 6=prm2, 7=prm3, 8=prm4, 9=prm5
	int listen_id;						//���b�X��ID�i��M�Җ����璲�ׁA�Y������ID��o�^�j
} BAS_PARSER;

//�ʐM�p�P�b�g�\����
typedef struct
{
	//-----���J�v���p�e�B-----
	char* sender;			//���M�Җ�
	char* reciever;			//��M�Җ�
	char command;			//�R�}���h�i�P�����j
	char* prm1;				//���[�h�|�C���^1
	char* prm2;				//���[�h�|�C���^2
	char* prm3;				//���[�h�|�C���^3
	char* prm4;				//���[�h�|�C���^4
	char* prm5;				//���[�h�|�C���^5
	char* prm6;				//���[�h�|�C���^6
	char* prm7;				//���[�h�|�C���^7
	//-----���\�b�h-----
	bool(*recieve)(char** msg, int* length);
	void(*response)(char* msg, int length);
	void(*broadcast)(char* msg, int length);
} BAS_PACKET;

//�G���[�R�[�h(�{�X�e�[�^�X�R�[�h)
typedef enum
{
	err_none = 0,			//�G���[����
	err_busy = 1,			//���쒆
	err_under_of_return,	//���^�[���̉񐔂�����
	err_out_of_return,		//�q�[�v�̈�I�[�o�[
	err_jump,				//�W�����v���s
	err_null = 2,			//NULL�G���[
	err_too_long = 3,		//�����񒷂��I�[�o�[
	err_parse,				//�p�[�X�G���[
	err_var_full,			//�����I�[�o�[

	err_butukari,			//�Ԃ���G���[
	err_org,				//���_�����m
	err_tumari,				//�l�܂茟�m�G���[
} error_code;

//������
typedef struct
{
	uint8_t run_no;			//���s�s��
	error_code err_no;		//�G���[�ԍ�
	int stp_no;				//�����X�e�b�v�ԍ�
	int timer_count;		//�^�C�}�[�J�E���g
} BASIC_STATE;

extern BASIC_STATE state;

extern BAS_PARSER bas_parser;
extern char* make_message(char* from, char* to, char cmd, char* message);

#endif//__UTIL_H__
