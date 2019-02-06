//=============================================================================
//�X�N���v�g���
//=============================================================================
#include "config.h"
#include "bas_script.h"
#include "dic.h"
#include "rpn.h"
#include "heap.h"
#include "bas_property.h"

//��M�o�b�t�@
BAS_PACKET script_packet;

//���M�o�b�t�@
static char send_buf[PROGRAM_LINE_COUNT];


//���x���ƍ�
static bool label_compare(char* label, char* msg)
{
	int i;
	for (i = 0; i < PROGRAM_LINE_MAX; i++)
	{
		char a = *(label++);
		char b = *(msg++);
		if (a == '\0') return true;
		if (a != b) return false;
	}

	state.err_no = err_too_long;
	return false;
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

//�X�N���v�g�\�����(CSV��؂�𕪉�)
bool csv_split(BAS_PACKET* packet, char* dst, char* msg)
{
	packet->prm1 = dst;
	packet->prm2 = packet->prm3 = packet->prm4 = packet->prm5 = packet->prm6 = packet->prm7 = NULL;
	char** prms = &packet->prm2;

	bool qout = false;
	int i;
	for (i = 0; i < BUFF_COUNT; i++, msg++)
	{
		char ch = *msg;
		if ('"' == ch)
		{	//������͈�(��؂蕶���ŋ�؂�Ȃ��悤�ɂ���)
			qout ^= true;
			*(dst++) = '"';
		}
		else if ((' ' == ch) && !qout)
		{	//��؂�L�����m
			*(dst++) = '\0';
			*(prms++) = dst;
		}
		else if ('\r' == ch)
		{
			//�X�L�b�v
		}
		else if (('\n' == ch) || ('\0' == ch))
		{	//�I�[
			*dst = '\0';
			return true;
		}
		else
		{	//��L�����ȏ�̓f�[�^�R�s�[
			*(dst++) = ch;
		}
	}
	state.err_no = err_parse;
	return false;
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
static void bas_script_if(BAS_PACKET* packet)
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
}

//GOTO��
static void bas_script_goto(BAS_PACKET* packet)
{
	bool is_label;
	state.run_no = label_search(packet->prm2, &is_label);

	if (is_label)
	{
		//�������
		BAS_PACKET parse;
		parse.reciever = parse.sender = SELF_NAME;
		if (csv_split(&parse, bas_parser.resp_buff, program_areas[state.run_no++]))
		{
			if (NULL == parse.prm2) return;
			if (NULL == packet->prm3) return;
			dic_set(parse.prm2, packet->prm3);

			if (NULL == parse.prm3) return;
			if (NULL == packet->prm4) return;
			dic_set(parse.prm3, packet->prm4);

			if (NULL == parse.prm4) return;
			if (NULL == packet->prm5) return;
			dic_set(parse.prm4, packet->prm5);

			if (NULL == parse.prm5) return;
			if (NULL == packet->prm6) return;
			dic_set(parse.prm5, packet->prm6);

			if (NULL == parse.prm6) return;
			if (NULL == packet->prm7) return;
			dic_set(parse.prm6, packet->prm7);
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
		bas_script_init();
	}
}

static void bas_script_end(BAS_PACKET* packet)
{
	bas_script_init();
}

//�u���[�h�L���X�g
void bas_script_broadcast(BAS_PACKET* packet, char* to, char* resp)
{
	char* response_message = make_message(SELF_NAME, to, INVOKE, resp);
	int length = strlen(response_message);
	packet->response(response_message, length);
	packet->broadcast(response_message, length);
}

//���b�Z�[�W�ʒm
static void bas_script_invoke(BAS_PACKET* packet)
{
	char resp[PROGRAM_LINE_COUNT];
	char* to = packet->prm1;
	char* key = packet->prm3;
	if (key == NULL)
	{
		state.err_no = err_null;
		return;
	}
	char* val = rpn_get_value(key);
	int len = strlen(key);
	if ('$' == key[len - 1])
	{
		sprintf(resp, "%s.%s=\"%s\"", SELF_NAME, key, val);
	}
	else
	{
		sprintf(resp, "%s.%s=%s", SELF_NAME, key, val);
	}
	bas_script_broadcast(packet, to, resp);
}

static void bas_set_error(BAS_PACKET* packet, error_code err_no)
{
	char resp[PROGRAM_LINE_COUNT];
	state.err_no = err_no;
	//�G���[���u���[�h�L���X�g
	sprintf(resp, "%s.ERR_NO=%d", SELF_NAME, state.err_no);
	bas_script_broadcast(packet, SELF_NAME, resp);
}

//�x������
static void bas_script_delay(BAS_PACKET* packet)
{
	state.stp_no = (0 < state.timer_count) ? 1 : 0;
}

//------------------------------�g����������------------------------------

//���_���A
static void bas_script_org(BAS_PACKET* packet)
{
	switch (state.stp_no)
	{
	case 0:
		//���_���A�R�}���h���s
		state.stp_no = 2;
		bas_do_abs(rpn_get_value("SPD_ORGH"), HAZUSI);
		break;

	case 2:
		//��~�҂�
		if (1 == REFR)
		{
			//�Ԃ����~
			bas_do_stop();
			state.err_no = err_butukari;
		}
		else if (0 == MOVING)
		{
			//��~�����猴�_���A�R�}���h���s
			state.stp_no = 3;
		}
		break;

	case 3:
		//��~�҂�
		if (1 == REFL)
		{
			//�Ԃ����~
			bas_do_stop();
			state.stp_no = 0;
		}
		else if (0 == MOVING)
		{
			//��~�����猴�_���A�R�}���h���s
			state.stp_no = 3;
		}
		break;
	}
}

//��Βl�ړ�
static void bas_script_abs(BAS_PACKET* packet)
{
	switch (state.stp_no)
	{
	case 0:
		//�����ړ��J�n
		state.stp_no = 1;
		bas_do_abs(packet->prm2, rpn_get_value(packet->prm3));
		break;

	case 1:
		//��~�҂�
		if (0 == MOVING)
		{
			//��~������I��
			state.stp_no = 0;
		}
		break;
	}
}

//���Βl�ړ�
static void bas_script_inc(BAS_PACKET* packet)
{
	switch (state.stp_no)
	{
	case 0:
		//�����ړ��J�n
		state.stp_no = 1;
		rpn_get_value(packet->prm2, dic_get(packet->prm3));
		break;

	case 1:
		//��~�҂�
		if (0 == MOVING)
		{
			//��~������I��
			state.stp_no = 0;
		}
		break;
	}
}

//���[�^��~
static void bas_script_stop(BAS_PACKET* packet)
{
	bas_do_stop();
}

//�G���[�ʒm����
static void bas_script_err(BAS_PACKET* packet)
{
	if (rpn_judge(packet))
	{
		//��������v������G���[�m��
		bas_set_error(packet, strtol(packet->prm3, NULL, 0));
	}
}

//�p�����[�^:Z1, Z2, ����PLS, WAIT
static void bas_script_tipon(BAS_PACKET* packet)
{
	switch (state.stp_no)
	{
	case 0:
		//������
		state.timer_count = -1;
		//Z1�ړ��R�}���h���s
		bas_do_abs(dic_get("SPD_HIGH"), strtol(packet->prm2, NULL, 0));	//Z1�ړ�
		if ("1" == rpn_get_value("NOZLE"))
		{
			//�m�Y�����m
			bas_do_stop();
			bas_set_error(packet, err_butukari);
		}
		else if (MOVING == 0)
		{
			//���슮������
			state.run_no = 1;
			bas_do_inc(dic_get("SPD_HIGH"), strtol(packet->prm3, NULL, 0));	//Z2�ړ�
		}

		bas_do_inc(dic_get("SPD_SLOW"), strtol(packet->prm4, NULL, 0));	//����
		//WAIT
		bas_do_org(dic_get("SPD_ORGL"), dic_get("LIMIT"));
		state.stp_no = 2;
		break;

	case 2:	
		//Z1�ړ������҂�
		if (dic_get("MOVING") == "0")
		{
			//Z1�ړ�����
			if (0 > state.timer_count)
			{
				//�^�C�}�[�ݒ�
				state.timer_count = 100;
			}
			if (0 == state.timer_count)
			{
				//�^�C�}�[�J�E���g�A�b�v
				state.stp_no = 3;
				//Z2�ړ��R�}���h���s
			}
		}
		break;

	case 3:	
		//Z2�ړ�
		if (dic_get("MOVING") == "0")
		{
			//Z2�ړ�����
			if (0 > state.timer_count)
			{
				//�^�C�}�[�ݒ�
				state.timer_count = 100;
			}
			if (0 == state.timer_count)
			{
				//�^�C�}�[�J�E���g�A�b�v
				state.stp_no = 3;
				//Z2�ړ��R�}���h���s
			}
		}
		break;

	case 4:	//Z����
	case 5:	//���_���A


		//���슮��
		if (dic_get("REFR") == "1")
		{
			//�Ԃ����~
			state.err_no = err_butukari;
		}
		else if (dic_get("MOVING") == "0")
		{
			//��~�����猴�_���A�R�}���h���s
			state.stp_no = 3;
		}
		break;
	}
}

static void bas_script_kin1(BAS_PACKET* packet)
{
	switch (state.stp_no)
	{
	case 0:
		//����J�n�R�}���h���M
		state.stp_no = 1;
		break;
	case 1:
		//���슮��
		state.stp_no = 0;
		break;
	}
}

static void bas_script_kin2(BAS_PACKET* packet)
{
	switch (state.stp_no)
	{
	case 0:
		//����J�n�R�}���h���M
		state.stp_no = 1;
		break;
	case 1:
		//���슮��
		state.stp_no = 0;
		break;
	}
}

static void bas_script_kout(BAS_PACKET* packet)
{
	switch (state.stp_no)
	{
	case 0:
		//����J�n�R�}���h���M
		state.stp_no = 1;
		break;
	case 1:
		//���슮��
		state.stp_no = 0;
		break;
	}
}

static void bas_script_tipoff(BAS_PACKET* packet)
{
	switch (state.stp_no)
	{
	case 0:
		//����J�n�R�}���h���M
		state.stp_no = 1;
		break;
	case 1:
		//���슮��
		state.stp_no = 0;
		break;
	}
}

//------------------------------�g�������܂�------------------------------

#define SCRIPT_COMMAND_TABLE_LENGTH	13
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
			command.execute(packet);
			if (0 == state.stp_no)
			{
				//���슮��
				state.run_no++;
				return true;
			}
			else
			{
				//���s��
				return false;
			}
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
		printf("%d\n", state.run_no);
		if ((0 != state.err_no) || (10 > state.run_no)) return;					//�v���O�������s�����ǂ�������
		char* msg = program_areas[state.run_no];								//�v���O�����R�[�h�擾
		if ('*' == msg[0]) continue;											//���x���Ȃ�X�L�b�v
		if (!csv_split(&script_packet, bas_parser.parse_buff, msg)) return;		//�v���O�����Ԓn�̃R�[�h�����
		if (!bas_script_execute(&script_packet)) return;						//���s
	}
}
