#include "config.h"
#include "iot_basic.h"

//���M�o�b�t�@
static char send_buf[PROGRAM_LINE_COUNT];

//BAS�|��@�̏��
BASIC_STATE state;

//24����x100�s�̃v���O�����G���A
char program_areas[PROGRAM_LINE_MAX][PROGRAM_LINE_COUNT] =
{
	//���b�X��
	"AXIS_Z1",			//  0: UNIQUE_NAME�i�������A�Ԏ���Ԃ��j
	"AXIS_Z",			//  1: LISTEN1(NOTIFY)
	"",				    //  2: LISTEN2(NOTIFY)
	"",				    //  3: LISTEN3(NOTIFY)
	"",					//  4: LISTEN4(NOTIFY)
	"",					//  5: LISTEN5(NOTIFY)
	"",					//  6: LISTEN6(NOTIFY)
	"",					//  7: LISTEN7(NOTIFY)
	"",					//  8: LISTEN8(NOTIFY)
	"",					//  9: LISTEN9(NOTIFY)
						// 10:�v���O�����̈�ȍ~���ׂ�
};

//�ԐM�d���쐬
void bas_send_message(char* from, char* to, char cmd, char* message)
{
	//�d����FSENDER,RECIEVER,1:PARAMETER...\n	
	char* msg = send_buf;
	while ('\0' != *from) *(msg++) = *(from++);
	*(msg++) = ',';
	while ('\0' != *to) *(msg++) = *(to++);
	*(msg++) = ',';
	*(msg++) = cmd;
	*(msg++) = ':';
	while ('\0' != *message) *(msg++) = *(message++);
	*(msg++) = '\n';
	*(msg++) = '\0';
	printf(send_buf);
}

//�p�����[�^���
bool bas_parse_parameter(BAS_PACKET* packet, char* msg, char separator)
{
	packet->opcode = msg;
	packet->prm1 = packet->prm2 = packet->prm3 = packet->prm4 = NULL;
	char** prms = &packet->prm1;
	for (;; msg++)
	{
		char ch = *msg;
		if (separator == ch)
		{
			*(msg++) = '\0';
			*(prms++) = msg;
			continue;
		}
		if (('\n' == ch) || ('\0' == ch))
		{
			*msg = '\0';
			return true;
		}
	}
}

//���x���ƍ�
bool bas_check_label(char* label, char* msg)
{
	while (true)
	{
		char a = *(label++);
		char b = *(msg++);
		if (a == '\0') return true;
		if (a != b) return false;
	}
}

//���x���ʒu�擾�i�s�ԍ����܂ށj
int bas_search_label(char* label, bool* is_label)
{
	int idx;
	char ch = label[0];

	if ('0' <= ch && ch <= '9')
	{
		//����
		*is_label = false;
		return strtol(label, NULL, 0);
	}
	if ('*' == ch)
	{
		//�W�����v��
		for (idx = 0; idx < PROGRAM_LINE_MAX; idx++)
		{
			if (bas_check_label(label, &program_areas[idx]))
			{
				*is_label = true;
				return idx;
			}
		}
	}
	//�Y���Ȃ�
	state.err_code = err_jump;
	return 0;
}

