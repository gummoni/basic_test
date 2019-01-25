//=============================================================================
//�X�N���v�g���
//=============================================================================
#include "config.h"
#include "bas_script.h"
#include "dic.h"
#include "rpn.h"
#include "heap.h"

//��M�o�b�t�@
static char recv_buf[PROGRAM_LINE_COUNT];


void bas_script_init(void)
{
	dic_clear();
	state.run_no = state.err_no = 0;
}

//IF��
static void bas_script_if(BAS_PACKET* packet)
{
	bool is_label;
	if (rpn_judge((BAS_PACKET_BODY*)&packet->prm1))
	{
		//TRUE
		state.run_no = label_search(packet->prm2, &is_label);
		if (is_label) state.run_no++;
	}
	else
	{
		//FALSE
		if (NULL != packet->prm3)
		{
			state.run_no = label_search(packet->prm3, &is_label);
			if (is_label) state.run_no++;
		}
	}
}

//GOTO��
static void bas_script_goto(BAS_PACKET* packet)
{
	bool is_label;
	state.run_no = label_search(packet->prm1, &is_label);

	if (is_label)
	{
		//�������
		BAS_PACKET parse;
		parse.reciever = parse.sender = SELF_NAME;
		parse.response = NULL;
		strcpy(recv_buf, program_areas[state.run_no++]);
		if (parse_parameter(&parse, recv_buf, ' '))
		{
			if (NULL == parse.prm1) return;
			if (NULL == packet->prm2) return;
			dic_set(parse.prm1, packet->prm2);

			if (NULL == parse.prm2) return;
			if (NULL == packet->prm3) return;
			dic_set(parse.prm2, packet->prm3);

			if (NULL == parse.prm3) return;
			if (NULL == packet->prm4) return;
			dic_set(parse.prm3, packet->prm4);

			if (NULL == parse.prm4) return;
			if (NULL == packet->prm5) return;
			dic_set(parse.prm4, packet->prm5);
		}
	}
}

//GOSUB��
static void bas_script_gosub(BAS_PACKET* packet)
{
	if (heap_enqueue(state.run_no))
	{
		bas_script_goto(packet);
	}
	else
	{
		state.err_no = err_out_of_return;
	}
}

//RETURN��
static void bas_script_return(BAS_PACKET* packet)
{
	if (!heap_dequeue(&state.run_no))
	{
		//�߂�悪�Ȃ��̂�END����
		state.run_no = state.stp_no = 0;
	}
}

//���b�Z�[�W�ʒm
static void bas_script_invoke(BAS_PACKET* packet)
{
	char resp[32];
	char* to = packet->prm1;
	char* key = packet->prm2;
	char* val = rpn_get_value(key);
	sprintf(resp, "%s.%s=%s", SELF_NAME, key, val);
	send_message(SELF_NAME, to, INVOKE, resp);
}

//�x������
static void bas_script_delay(BAS_PACKET* packet)
{
	//TODO
}

//------------------------------�g����������------------------------------

//���_���A
static void bas_script_org(BAS_PACKET* packet)
{
	//TODO
}

//��Βl�ړ�
static void bas_script_abs(BAS_PACKET* packet)
{
	//TODO
}

//���Βl�ړ�
static void bas_script_inc(BAS_PACKET* packet)
{
	//TODO
}

//���[�^��~
static void bas_script_stop(BAS_PACKET* packet)
{
	//TODO
}

//�G���[�`�F�b�N����
static void bas_script_check(BAS_PACKET* packet)
{
	//TODO
}

//�G���[�ʒm����
static void bas_script_err(BAS_PACKET* packet)
{
	//TODO
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
	{ "INVOKE"	, bas_script_invoke		},	//4
	{ "DELAY"	, bas_script_delay		},	//5
	{ "CHK"		, bas_script_check		},	//6
	{ "ERR"		, bas_script_err		},	//7
	//----�g�����߁iPB210�j---
	{ "ORG"		, bas_script_org		},	//8
	{ "ABS"		, bas_script_abs		},	//9
	{ "INC"		, bas_script_inc		},	//10
	{ "STOP"	, bas_script_stop		},	//11
};

//�R�}���h���s
static void bas_script_execute(BAS_PACKET* packet)
{
	//�\�񖽗ߎ��s
	int i = 0;
	for (i = 0; i < SCRIPT_COMMAND_TABLE_LENGTH; i++)
	{
		BAS_SCRIPT_TABLE command = script_command_table[i];
		if (0 == strcmp(packet->opcode, command.name))
		{
			command.execute(packet);
			return;
		}
	}

	//RPN���s
	rpn_execute((BAS_PACKET_BODY*)&packet->opcode);
}

//�X�N���v�g���s����
void bas_script_job(void)
{
	BAS_PACKET packet;

	while (true)
	{
		if ((0 != state.err_no) || (0 == state.run_no)) return;
		//�v���O�����Ԓn�̃R�[�h��CSV�������Ď��s
		packet.reciever = packet.sender = SELF_NAME;
		packet.response = NULL;
		strcpy(recv_buf, program_areas[state.run_no++]);
		if (!parse_parameter(&packet, recv_buf, ' ')) return;
		if ('*' == packet.opcode[0]) continue;
		bas_script_execute(&packet);
		return;
	}
}
