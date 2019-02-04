//=============================================================================
//�X�N���v�g���
//=============================================================================
#include "config.h"
#include "bas_script.h"
#include "dic.h"
#include "rpn.h"
#include "heap.h"


//��M�o�b�t�@
BAS_PACKET script_packet;

//���M�o�b�t�@
static char send_buf[PROGRAM_LINE_COUNT];


//���x���ƍ�
static bool label_compare(char* label, char* msg)
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
static int label_search(char* label, bool* is_label)
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
			if (label_compare(label, &program_areas[idx][0]))
			{
				*is_label = true;
				return idx;
			}
		}
	}
	//�Y���Ȃ�
	state.err_no = err_jump;
	return 0;
}

//�X�N���v�g�\�����
static bool parse_script(BAS_PACKET* packet, char* dst, char* msg)
{
	packet->prm1 = dst;
	packet->prm2 = packet->prm3 = packet->prm4 = packet->prm5 = packet->prm6 = packet->prm7 = NULL;
	char** prms = &packet->prm2;

	bool qout = false;
	for (;; msg++)
	{
		char ch = *msg;
		if ('"' == ch)
		{	//������͈�(��؂蕶���ŋ�؂�Ȃ��悤�ɂ���)
			qout ^= true;
		}
		else if ((' ' == ch) && !qout)
		{	//��؂�L�����m
			*(dst++) = '\0';
			*(prms++) = dst;
		}
		else if (('\n' == ch) || ('\0' == ch))
		{	//�I�[
			*dst = '\0';
			return true;
		}
		else
		{	//��L�����o�Ȃ��ꍇ�̓f�[�^�R�s�[
			*(dst++) = ch;
		}
	}
}

//=============================================================================
//�ԐM�d���쐬
//=============================================================================
char* make_message(char* from, char* to, char cmd, char* message)
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

	return send_buf;
}

//=============================================================================
//��{���߃Z�b�g
//=============================================================================

//������
void bas_script_init(void)
{
	heap_init();
	state.run_no = state.err_no = state.stp_no = state.timer_count = 0;
}

//IF��
static bool bas_script_if(BAS_PACKET* packet)
{
	bool is_label;
	if (rpn_judge(packet))
	{
		//TRUE
		state.run_no = label_search(packet->prm3, &is_label);
		if (is_label) state.run_no++;
	}
	else
	{
		//FALSE
		if (NULL != packet->prm4)
		{
			state.run_no = label_search(packet->prm4, &is_label);
			if (is_label) state.run_no++;
		}
	}
	return true;
}

//GOTO��
static bool bas_script_goto(BAS_PACKET* packet)
{
	bool is_label;
	state.run_no = label_search(packet->prm2, &is_label);

	if (is_label)
	{
		//�������
		BAS_PACKET parse;
		parse.reciever = parse.sender = SELF_NAME;
		if (parse_script(&parse, bas_parser.resp_buff, program_areas[state.run_no++]))
		{
			if (NULL == parse.prm2) return false;
			if (NULL == packet->prm3) return false;
			dic_set(parse.prm2, packet->prm3);

			if (NULL == parse.prm3) return false;
			if (NULL == packet->prm4) return false;
			dic_set(parse.prm3, packet->prm4);

			if (NULL == parse.prm4) return false;
			if (NULL == packet->prm5) return false;
			dic_set(parse.prm4, packet->prm5);

			if (NULL == parse.prm5) return false;
			if (NULL == packet->prm6) return false;
			dic_set(parse.prm5, packet->prm6);

			if (NULL == parse.prm6) return false;
			if (NULL == packet->prm7) return false;
			dic_set(parse.prm6, packet->prm7);
		}
	}
	return true;
}

//GOSUB��
static bool bas_script_gosub(BAS_PACKET* packet)
{
	if (heap_enqueue(state.run_no))
	{
		bas_script_goto(packet);
	}
	else
	{
		state.err_no = err_out_of_return;
	}
	return true;
}

//RETURN��
static bool bas_script_return(BAS_PACKET* packet)
{
	if (!heap_dequeue(&state.run_no))
	{
		//�߂�悪�Ȃ��̂�END����
		bas_script_init();
	}
	return true;
}

static bool bas_script_end(BAS_PACKET* packet)
{
	bas_script_init();
	return true;
}

//���b�Z�[�W�ʒm
static bool bas_script_invoke(BAS_PACKET* packet)
{
	char resp[32];
	char* to = packet->prm1;
	char* key = packet->prm3;
	char* val = rpn_get_value(key);
	sprintf(resp, "%s.%s=%s", SELF_NAME, key, val);
	char* response_message = make_message(SELF_NAME, to, INVOKE, resp);
	int length = strlen(response_message);
	packet->response(response_message, length);
	packet->broadcast(response_message, length);
	return true;
}

//�x������
static bool bas_script_delay(BAS_PACKET* packet)
{
	return true;														//TODO
}

//------------------------------�g����������------------------------------

//���_���A
static bool bas_script_org(BAS_PACKET* packet)
{
	return true;														//TODO
}

//��Βl�ړ�
static bool bas_script_abs(BAS_PACKET* packet)
{
	return true;														//TODO
}

//���Βl�ړ�
static bool bas_script_inc(BAS_PACKET* packet)
{
	return true;														//TODO
}

//���[�^��~
static bool bas_script_stop(BAS_PACKET* packet)
{
	return true;														//TODO
}

//�G���[�`�F�b�N����
static bool bas_script_check(BAS_PACKET* packet)
{
	return true;														//TODO
}

//�G���[�ʒm����
static bool bas_script_err(BAS_PACKET* packet)
{
	return true;														//TODO
}

//------------------------------�g�������܂�------------------------------

#define SCRIPT_COMMAND_TABLE_LENGTH	12
static BAS_SCRIPT_TABLE script_command_table[SCRIPT_COMMAND_TABLE_LENGTH] =
{
	//---��{����---
	{ "IF"		, bas_script_if			},	//0
	{ "GOTO"	, bas_script_goto		},	//1
	{ "GOSUB"	, bas_script_gosub		},	//2
	{ "RETURN"	, bas_script_return		},	//3
	{ "END"		, bas_script_end		},	//4
	{ "INVOKE"	, bas_script_invoke		},	//5
	{ "DELAY"	, bas_script_delay		},	//6
	{ "CHK"		, bas_script_check		},	//7
	{ "ERR"		, bas_script_err		},	//8
	//----�g�����߁iPB210�j---
	{ "ORG"		, bas_script_org		},	//9
	{ "ABS"		, bas_script_abs		},	//10
	{ "INC"		, bas_script_inc		},	//11
	{ "STOP"	, bas_script_stop		},	//12
};

//�R�}���h���s
static bool bas_script_execute(BAS_PACKET* packet)
{
	//�\�񖽗ߎ��s
	int i = 0;
	for (i = 0; i < SCRIPT_COMMAND_TABLE_LENGTH; i++)
	{
		BAS_SCRIPT_TABLE command = script_command_table[i];
		if (0 == strcmp(packet->prm1, command.name))
		{
			return command.execute(packet);
		}
	}

	//RPN���s
	return rpn_execute(packet);
}

//�X�N���v�g���s����
void bas_script_job(void)
{
	while (true)
	{
		if (state.run_no == 0)
		{
			printf("%d\n", state.run_no);
		}
		else
		{
			printf("%d\n", state.run_no);
		}
		if ((0 != state.err_no) || (10 > state.run_no)) return;					//�v���O�������s�����ǂ�������
		char* msg = program_areas[state.run_no++];								//�v���O�����R�[�h�擾
		if ('*' == msg[0]) continue;											//���x���Ȃ�X�L�b�v
		if (!parse_script(&script_packet, bas_parser.parse_buff, msg)) return;	//�v���O�����Ԓn�̃R�[�h�����
		if (!bas_script_execute(&script_packet)) return;						//���s
	}
}
