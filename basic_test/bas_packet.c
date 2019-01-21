#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "bas_packet.h"
#include "csv_basic.h"

//���M�o�b�t�@
static char send_buf[PROGRAM_LINE_COUNT];

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

