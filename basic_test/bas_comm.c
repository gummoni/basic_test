//=============================================================================
//�p�P�b�g���
//=============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "csv_basic.h"
#include "bas_comm.h"
#include "bas_packet.h"
#include "script_reader.h"

//�X�N���v�g�Ǎ���
static bool bas_comm_read(BAS_PACKET_BODY* context)
{
	char* addr = context->opcode;
	int address = strtol(addr, NULL, 0);
	context->response = program_areas[address];

	return true;
}

//�X�N���v�g������
static bool bas_comm_write(BAS_PACKET_BODY* context)
{
	char* addr = context->opcode;
	char* value = context->prm1;
	int address = strtol(addr, NULL, 0);
	strcpy(program_areas[address], value);
	return true;
}

//�X�N���v�g���s
static bool bas_comm_start(BAS_PACKET_BODY* context)
{
	char* addr = context->opcode;
	int run_no = strtol(addr, NULL, 0);
	if (0 != state.err_code) return false;
	if (0 != state.run_no) return false;
	state.run_no = run_no;
	return true;
}

//���f
static bool bas_comm_abort(BAS_PACKET_BODY* context)
{
	if (0 != state.err_code) return false;
	state.run_no = 0;
	return true;
}

//�G���[�N���A
static bool bas_comm_errclear(BAS_PACKET_BODY* context)
{
	state.err_code = state.run_no = 0;
	return true;
}

//�X�e�[�^�X�擾(�ԐM�����̒��ŕԐM�d�����쐬���Ă���)
static bool bas_comm_status(BAS_PACKET_BODY* context)
{
	return true;
}

//�t�@�[���Ǎ���
static bool bas_comm_load(BAS_PACKET_BODY* context)
{
	//TODO
	return true;
}

//�t�@�[��������
static bool bas_comm_save(BAS_PACKET_BODY* context)
{
	//TODO
	return true;
}

//�ʐM�p�P�b�g��M���
#define PACKET_BAS_COMMAND_TABLE 9
static BAS_PACKET_TABLE packet_command_table[PACKET_BAS_COMMAND_TABLE] =
{
	{ READ		, bas_comm_read			},		// �p�����[�^�Ǎ��݁i�߂�l�F�l�j
	{ WRITE		, bas_comm_write		},		// �p�����[�^�����݁i�߂�l�F�Ȃ��j
	{ START		, bas_comm_start		},		// �J�n�R�}���h�i�߂�l�F�Ȃ��j
	{ ABORT		, bas_comm_abort		},		// ���f�R�}���h�i�߂�l�F�Ȃ��j
	{ ERR_CLEAR	, bas_comm_errclear		},		// �G���[�����i�߂�l�F�Ȃ��j
	{ NOTIFY	, rpn_execute			},		// �v�Z�R�}���h�i�߂�l�F�l�j
	{ STATUS	, bas_comm_status       },		// �X�e�[�^�X�擾
	{ LOAD		, bas_comm_load			},		// �t�@�[���Ǎ���
	{ SAVE		, bas_comm_save			},		// �t�@�[���ۑ�
};

//�p�P�b�g���s
static bool bas_comm_execute(BAS_PACKET* packet)
{
	for (int i = 0; i < PACKET_BAS_COMMAND_TABLE; i++)
	{
		BAS_PACKET_TABLE cmd = packet_command_table[i];
		if (packet->command == (char)cmd.name)
		{
 			return cmd.execute((BAS_PACKET_BODY*)&packet->opcode);
		}
	}
	return false;
}

//����m�F
static bool bas_comm_check_from(BAS_PACKET* self)
{
	int i;
	for (i = 0; i < 10; i++)
	{
		if (0 == strcmp(program_areas[i], self->reciever)) return true;
	}
	return false;
}

//�P�����擾
static bool bas_check_message(char* msg)
{
	int i;
	for (i = 0; i < 64; i++)
	{
		char ch = msg[i];
		if ('a' <= ch && ch <= 'z') continue;
		if ('A' <= ch && ch <= 'Z') continue;
		if ('0' <= ch && ch <= '9') continue;
		if ('(' == ch) continue;
		if (')' == ch) continue;
		if ('+' == ch) continue;
		if ('-' == ch) continue;
		if ('*' == ch) continue;
		if ('/' == ch) continue;
		if ('.' == ch) continue;
		if (',' == ch) continue;
		if (' ' == ch) continue;
		if ('!' == ch) continue;
		if ('=' == ch) continue;
		if ('<' == ch) continue;
		if ('>' == ch) continue;
		if ('$' == ch) continue;
		if ('%' == ch) continue;
		if ('_' == ch) continue;
		if (':' == ch) continue;
		if ('\"' == ch) continue;
		if ('\n' == ch) continue;
		if ('\0' == ch) return true;
		return false;
	}
	return false;
}

//�g�s�b�NID�擾(0-9=�Y������,-1=�Y���Ȃ�
static int bas_comm_get_topic_id(char* topic)
{
	int i;
	for (i = 0; i < 10; i++)
	{
		if (0 == strcmp(topic, program_areas[i])) return i;
	}
	return -1;
}

//--------------------------------------------
//��M���b�Z�[�W����͂���
//--------------------------------------------
//�d����FSENDER,RECIEVER,1:PARAMETER...\n
bool bas_comm_parse(BAS_PACKET* packet, char* msg)
{
	packet->response = NULL;
	if (!bas_check_message(msg)) return false;

	//----------------------------------------------------------------------------
	//�p�P�b�g���
	//----------------------------------------------------------------------------
	//[SENDER]
	packet->sender = msg;	
	for (msg++; ',' != *msg; msg++) if (('\0' == *msg) || ('\n' == *msg)) return false;
	*(msg++) = '\0';

	//[RECIEVER]
	packet->reciever = msg;
	for (msg++; ',' != *msg; msg++) if (('\0' == *msg) || ('\n' == *msg)) return false;
	*(msg++) = '\0';
	packet->listen_id = bas_comm_get_topic_id(packet->reciever);
	if (0 > packet->listen_id) return false;

	//[COMMAND]
	packet->command = *(msg++);
	if (':' != *(msg++)) return false;

	//[PARAMETER]
	return bas_parse_parameter(packet, msg, ',');
}

//�ʐM�d����͏���
void bas_comm_job(char* recv_message)
{
	BAS_PACKET packet;

	//�ʐM�p�P�b�g���
	if (!bas_comm_parse(&packet, recv_message)) return;
	//���b�X���Ώۂ��`�F�b�N
	if (!bas_comm_check_from(&packet)) return;
	//�p�P�b�g���s
	bool is_success = bas_comm_execute(&packet);
	//�������ĂȂ�ԐM
	if (0 == strcmp(SELF_NAME, packet.reciever))
	{
		//�������ł�����
		char result[16];
		if (is_success)
		{
			if (STATUS == packet.command)
			{
				//�X�e�[�^�X�ԑ�
				sprintf(result, "OK,%d,%d", state.run_no, state.err_code);
			}
			else if (NULL == packet.response)
			{
				//���X�|���X����
				strcpy(result, "OK");
			}
			else
			{
				//���X�|���X�L��
				sprintf(result, "OK,%s", packet.response);
			}
		}
		else
		{
			//�G���[����
			sprintf(result, "NG,%d,%d", state.run_no, state.err_code);
		}
		//�ԐM
		bas_send_message(SELF_NAME, packet.sender, packet.command | 0x20, result);
	}
}
