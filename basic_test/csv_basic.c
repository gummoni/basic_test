#include "config.h"
#include "csv_basic.h"
#include "dictionary.h"
#include "bas_comm.h"
#include "bas_script.h"
#include "bas_packet.h"

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

void bas_init(void)
{
	dic_clear();
	state.run_no = state.err_code = 0;
}

//�R�}���h���
void bas_main(char* recv_message)
{
	bas_comm_job(recv_message);
 	bas_script_job();
}
