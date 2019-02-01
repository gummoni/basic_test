#include "config.h"
#include "bas_protcol.h"
#include "bas_script.h"
#include "dic.h"
#include "rpn.h"
#include "heap.h"


//=============================================================================
//�X�N���v�g�Ǎ���
//=============================================================================
static bool bas_comm_read(BAS_PACKET* packet)
{
	char* addr = packet->prm1;
	int address = strtol(addr, NULL, 0);
	
	char* msg = make_message(SELF_NAME, packet->sender, packet->command, program_areas[address]);
	int len = strlen(msg);
	packet->response(msg, len);

	return true;
}

//=============================================================================
//�X�N���v�g������
//=============================================================================
static bool bas_comm_write(BAS_PACKET* packet)
{
	char* addr = packet->prm1;
	char* value = packet->prm2;
	int address = strtol(addr, NULL, 0);
	strcpy(program_areas[address], value);
	return true;
}

//=============================================================================
//�X�N���v�g���s
//=============================================================================
static bool bas_comm_start(BAS_PACKET* packet)
{
	char* addr = packet->prm1;
	int run_no = strtol(addr, NULL, 0);
	if (0 != state.err_no) return false;
	if (0 != state.run_no) return false;
	state.run_no = run_no;
	return true;
}

//=============================================================================
//���f
//=============================================================================
static bool bas_comm_abort(BAS_PACKET* packet)
{
	if (0 != state.err_no) return false;
	state.run_no = 0;
	return true;
}

//=============================================================================
//�G���[�N���A
//=============================================================================
static bool bas_comm_errclear(BAS_PACKET* packet)
{
	state.err_no = state.run_no = state.stp_no = 0;
	return true;
}

//=============================================================================
//�X�e�[�^�X�擾(�ԐM�����̒��ŕԐM�d�����쐬���Ă���)
//=============================================================================
static bool bas_comm_status(BAS_PACKET* packet)
{
	return true;
}

//=============================================================================
//�t�@�[���Ǎ���
//=============================================================================
static bool bas_comm_load(BAS_PACKET* packet)
{
	//TODO
	return true;
}

//=============================================================================
//�t�@�[��������
//=============================================================================
static bool bas_comm_save(BAS_PACKET* packet)
{
	//TODO
	return true;
}

//=============================================================================
//�ʐM�p�P�b�g��M���
//=============================================================================
#define PACKET_BAS_COMMAND_TABLE 9
static BAS_COMM_TABLE packet_command_table[PACKET_BAS_COMMAND_TABLE] =
{
	{ READ		, bas_comm_read			},		// �p�����[�^�Ǎ��݁i�߂�l�F�l�j
	{ WRITE		, bas_comm_write		},		// �p�����[�^�����݁i�߂�l�F�Ȃ��j
	{ START		, bas_comm_start		},		// �J�n�R�}���h�i�߂�l�F�Ȃ��j
	{ ABORT		, bas_comm_abort		},		// ���f�R�}���h�i�߂�l�F�Ȃ��j
	{ ERR_CLEAR	, bas_comm_errclear		},		// �G���[�����i�߂�l�F�Ȃ��j
	{ INVOKE	, rpn_execute			},		// �v�Z�R�}���h�i�߂�l�F�l�j
	{ LOAD		, bas_comm_load			},		// �t�@�[���Ǎ���
	{ SAVE		, bas_comm_save			},		// �t�@�[���ۑ�
};

//=============================================================================
//�p�P�b�g���s
//=============================================================================
bool bas_comm_execute(BAS_PACKET* packet)
{
	for (int i = 0; i < PACKET_BAS_COMMAND_TABLE; i++)
	{
		BAS_COMM_TABLE cmd = packet_command_table[i];
		if (packet->command == (char)cmd.name)
		{
			return cmd.execute(packet);
		}
	}
	return false;
}
//=============================================================================
//��M�f�[�^��荞��(TRUE=�p�P�b�g��荞�݊����j
//=============================================================================
static bool bas_parse(BAS_PACKET* packet, char ch)
{
	//�f�[�^���G���[
	if (bas_parser.parse_length >= BUFF_COUNT)
	{
		clear_packet(packet);
		return false;
	}

	//===���s���m===
	if ('\r' == ch || '\n' == ch || '\0' == ch)
	{
		if (RECV_STATE_CORON < bas_parser.state)
		{
			//����Ɏ�M
			bas_parser.parse_buff[bas_parser.parse_length++] = '\0';
			return true;
		}
		else
		{
			//�ُ��M���Z�b�g
			clear_packet(packet);
			return false;
		}
	}

	//===�w�b�_��؂茟�m===
	if (':' == ch)
	{
		if (RECV_STATE_CORON == bas_parser.state)
		{
			bas_parser.state = RECV_STATE_PRM1;
		}
		else
		{
			//�G���[�i���Z�b�g�҂��j
			bas_parser.state = RECV_STATE_ERROR;
		}
		return false;
	}

	//===�{�f�B��؂茟�m===
	if ((',' == ch) && !bas_parser.is_quot)
	{
		switch (bas_parser.state)
		{
		case RECV_STATE_FROM:
			bas_parser.parse_buff[bas_parser.parse_length++] = '\0';
			packet->reciever = &bas_parser.parse_buff[bas_parser.parse_length];
			bas_parser.state = RECV_STATE_TO;
			break;

		case RECV_STATE_TO:
			bas_parser.parse_buff[bas_parser.parse_length++] = '\0';
			packet->prm1 = &bas_parser.parse_buff[bas_parser.parse_length];
			bas_parser.state = RECV_STATE_COMMAND;
			break;

		case RECV_STATE_COMMAND:
		case RECV_STATE_CORON:
		case RECV_STATE_PRM6:
			bas_parser.state = RECV_STATE_ERROR;
			break;

		case RECV_STATE_PRM1:
			bas_parser.parse_buff[bas_parser.parse_length++] = '\0';
			packet->prm2 = &bas_parser.parse_buff[bas_parser.parse_length];
			bas_parser.state = RECV_STATE_PRM2;
			break;

		case RECV_STATE_PRM2:
			bas_parser.parse_buff[bas_parser.parse_length++] = '\0';
			packet->prm3 = &bas_parser.parse_buff[bas_parser.parse_length];
			bas_parser.state = RECV_STATE_PRM3;
			break;

		case RECV_STATE_PRM3:
			bas_parser.parse_buff[bas_parser.parse_length++] = '\0';
			packet->prm4 = &bas_parser.parse_buff[bas_parser.parse_length];
			bas_parser.state = RECV_STATE_PRM4;
			break;

		case RECV_STATE_PRM4:
			bas_parser.parse_buff[bas_parser.parse_length++] = '\0';
			packet->prm5 = &bas_parser.parse_buff[bas_parser.parse_length];
			bas_parser.state = RECV_STATE_PRM5;
			break;

		case RECV_STATE_PRM5:
			bas_parser.parse_buff[bas_parser.parse_length++] = '\0';
			packet->prm6 = &bas_parser.parse_buff[bas_parser.parse_length];
			bas_parser.state = RECV_STATE_PRM6;
			break;
		}
		return false;
	}

	//===�f�[�^���===
	switch (bas_parser.state)
	{
	case RECV_STATE_FROM:
		if (('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9') || ('_' == ch))
		{
			//�����i�[
			bas_parser.parse_buff[bas_parser.parse_length++] = ch;
		}
		else
		{
			//�G���[�i���Z�b�g�҂��j
			bas_parser.state = RECV_STATE_ERROR;
		}
		break;

	case RECV_STATE_TO:
		if ('a' <= ch && ch <= 'z') ch &= 0xbf;
		if (('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9') || ('_' == ch))
		{
			//�����i�[
			bas_parser.parse_buff[bas_parser.parse_length++] = ch;
		}
		else
		{
			//�G���[�i���Z�b�g�҂��j
			bas_parser.state = RECV_STATE_ERROR;
		}
		break;

	case RECV_STATE_COMMAND:
		switch (ch)
		{
		case READ:
		case WRITE:
		case START:
		case ABORT:
		case ERR_CLEAR:
		case INVOKE:
		case LOAD:
		case SAVE:
			packet->command = ch;
			bas_parser.state = RECV_STATE_CORON;
			break;
		default:
			bas_parser.state = RECV_STATE_ERROR;
			break;
		}
		break;

	case RECV_STATE_CORON:
		//�擪�s�Ń`�F�b�N�ς�
		break;

	case RECV_STATE_PRM1:
	case RECV_STATE_PRM2:
	case RECV_STATE_PRM3:
	case RECV_STATE_PRM4:
	case RECV_STATE_PRM5:
	case RECV_STATE_PRM6:
		if ('"' == ch)
		{
			//������L�����m
			bas_parser.is_quot ^= true;
			bas_parser.parse_buff[bas_parser.parse_length++] = '"';
		}
		else
		{
			if (!bas_parser.is_quot)
			{
				if ('a' <= ch && ch <= 'z') ch &= 0xbf;
			}
			if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9') || ('+' == ch) || ('-' == ch) || ('*' == ch) || ('/' == ch) || ('.' == ch) || ('<' == ch) || ('>' == ch) || ('=' == ch) || ('%' == ch) || ('$' == ch) || (' ' == ch) || (',' == ch) || ('_' == ch) || ('(' == ch) || (')' == ch))
			{
				bas_parser.parse_buff[bas_parser.parse_length++] = ch;
			}
			else
			{
				bas_parser.state = RECV_STATE_ERROR;
			}
		}
		break;
	default:
		break;
	}
	return false;
}

//=============================================================================
//�p�P�b�g���s
//=============================================================================
static void bas_response(BAS_PACKET* packet, bool ack)
{
	//�Q�ƑΏۂ������̂ŃR�}���h���s
	byte result[16];

	if (ack)
	{
		if ('\0' == bas_parser.resp_buff[0])
		{
			//���X�|���X����
			strcpy(result, "OK");
		}
		else
		{
			//���X�|���X�L��
			sprintf(result, "OK,%s", bas_parser.resp_buff);
		}
	}
	else
	{
		//�G���[����
		sprintf(result, "ER,%d,%d", state.run_no, state.err_no);
	}
	//�ԐM
	char* response_message = make_message(SELF_NAME, packet->sender, packet->command | 0x20, result);
	int length = strlen(response_message);
	packet->response(response_message, length);
}

//=============================================================================
//����m�F�i�g�s�b�NID�擾�j
//=============================================================================
static bool get_listen_id(char* topic, int* result)
{
	int i;
	for (i = 0; i < 10; i++)
	{
		int length = strlen(program_areas[i]);
		if ('\0' == topic[length])
		{	//�����`�F�b�N���Ă���Ӓ�
			if (0 == strcmp(topic, program_areas[i]))
			{
				*result = i;
				return true;
			}
		}
	}

	*result = -1;
	return false;
}

//=============================================================================
//��M����
//=============================================================================
void bas_comm_job(BAS_PACKET* packet)
{
	int length;
	char* msg;

	if (packet->recieve(&msg, &length))
	{
		printf(">>>%s", msg);
		//��M���
		while (0 < length--)
		{
			char ch = *(msg++);

			if (bas_parse(packet, ch))
			{
				//1�p�P�b�g��M����
				if (get_listen_id(packet->reciever, &bas_parser.listen_id))
				{
					bool ack = bas_comm_execute(packet);
					if (0 == bas_parser.listen_id)
					{
						bas_response(packet, ack);
					}
				}
				//���̃p�P�b�g��M��
				clear_packet(packet);
				return;
			}
		}
	}
}

//=============================================================================
//�p�P�b�g�N���A
//=============================================================================
void clear_packet(BAS_PACKET* packet)
{
	bas_parser.parse_length = 0;
	bas_parser.state = 0;
	bas_parser.is_quot = false;
	packet->sender = bas_parser.parse_buff;
	packet->reciever = NULL;
	packet->command = '\0';
	packet->prm1 = NULL;
	packet->prm2 = NULL;
	packet->prm3 = NULL;
	packet->prm4 = NULL;
	packet->prm5 = NULL;
	packet->prm6 = NULL;
}
