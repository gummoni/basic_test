//=============================================================================
//�ʐM�v���g�R��
//=============================================================================
#include "config.h"
#include "bas_comm.h"
#include "rpn.h"

//�X�N���v�g�Ǎ���
static bool bas_comm_read(BAS_PACKET* packet)
{
	char* addr = packet->prm1;
	int address = strtol(addr, NULL, 0);
	packet->response = program_areas[address];

	return true;
}

//�X�N���v�g������
static bool bas_comm_write(BAS_PACKET* packet)
{
	char* addr = packet->prm1;
	char* value = packet->prm2;
	int address = strtol(addr, NULL, 0);
	strcpy(program_areas[address], value);
	return true;
}

//�X�N���v�g���s
static bool bas_comm_start(BAS_PACKET* packet)
{
	char* addr = packet->prm1;
	int run_no = strtol(addr, NULL, 0);
	if (0 != state.err_no) return false;
	if (0 != state.run_no) return false;
	state.run_no = run_no;
	return true;
}

//���f
static bool bas_comm_abort(BAS_PACKET* packet)
{
	if (0 != state.err_no) return false;
	state.run_no = 0;
	return true;
}

//�G���[�N���A
static bool bas_comm_errclear(BAS_PACKET* packet)
{
	state.err_no = state.run_no = state.stp_no = 0;
	return true;
}

//�X�e�[�^�X�擾(�ԐM�����̒��ŕԐM�d�����쐬���Ă���)
static bool bas_comm_status(BAS_PACKET* packet)
{
	return true;
}

//�t�@�[���Ǎ���
static bool bas_comm_load(BAS_PACKET* packet)
{
	//TODO
	return true;
}

//�t�@�[��������
static bool bas_comm_save(BAS_PACKET* packet)
{
	//TODO
	return true;
}

//�ʐM�p�P�b�g��M���
#define PACKET_BAS_COMMAND_TABLE 9
static BAS_COMM_TABLE packet_command_table[PACKET_BAS_COMMAND_TABLE] =
{
	{ READ		, bas_comm_read			},		// �p�����[�^�Ǎ��݁i�߂�l�F�l�j
	{ WRITE		, bas_comm_write		},		// �p�����[�^�����݁i�߂�l�F�Ȃ��j
	{ START		, bas_comm_start		},		// �J�n�R�}���h�i�߂�l�F�Ȃ��j
	{ ABORT		, bas_comm_abort		},		// ���f�R�}���h�i�߂�l�F�Ȃ��j
	{ ERR_CLEAR	, bas_comm_errclear		},		// �G���[�����i�߂�l�F�Ȃ��j
	{ INVOKE	, rpn_execute			},		// �v�Z�R�}���h�i�߂�l�F�l�j
	{ STATUS	, bas_comm_status       },		// �X�e�[�^�X�擾
	{ LOAD		, bas_comm_load			},		// �t�@�[���Ǎ���
	{ SAVE		, bas_comm_save			},		// �t�@�[���ۑ�
};

//�p�P�b�g���s
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
