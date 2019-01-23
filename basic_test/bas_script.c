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
	state.run_no = state.err_code = 0;
}

//IF��
static void bas_script_if(BAS_PACKET* packet)
{
	bool is_label;
	if (rpn_judge(&packet->prm1))
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

			if (NULL == parse.prm2) return;
			if (NULL == packet->prm3) return;
			dic_set(parse.prm2, packet->prm3);
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
		state.err_code = err_out_of_return;
	}
}

//RETURN��
static void bas_script_return(BAS_PACKET* packet)
{
	if (!heap_dequeue(&state.run_no))
	{
		state.err_code = err_invalid_return;
	}
}

//END��
static void bas_script_end(BAS_PACKET* packet)
{
	state.run_no = 0;
}

//���b�Z�[�W�ʒm
static void bas_script_notify(BAS_PACKET* packet)
{
	char resp[32];
	char* to = packet->prm1;
	char* key = packet->prm2;
	char* val = rpn_get_value(key);
	sprintf(resp, "%s.%s=%s", SELF_NAME, key, val);
	send_message(SELF_NAME, to, NOTIFY, resp);
}

#define SCRIPT_COMMAND_TABLE_LENGTH	6
static BAS_SCRIPT_TABLE script_command_table[SCRIPT_COMMAND_TABLE_LENGTH] =
{
	{ "IF"		, bas_script_if			},
	{ "GOTO"	, bas_script_goto		},
	{ "GOSUB"	, bas_script_gosub		},
	{ "RETURN"	, bas_script_return		},
	{ "END"		, bas_script_end		},
	{ "NOTIFY"	, bas_script_notify		},
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
		if ((0 != state.err_code) || (0 == state.run_no)) return;
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
