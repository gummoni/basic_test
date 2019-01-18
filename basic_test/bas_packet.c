#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "bas_packet.h"
#include "csv_basic.h"

//送信バッファ
char send_buf[PROGRAM_LINE_COUNT];

//受信バッファ
char recv_buf[PROGRAM_LINE_COUNT];

//BAS翻訳機の状態
BASIC_STATE state;


//返信電文作成
void bas_send_message(char* from, char* to, char cmd, char* message)
{
	//電文例：SENDER,RECIEVER,1:PARAMETER...\n	
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

//パラメータ解析
bool bas_parse_parameter(BAS_PACKET* packet, char* msg)
{
	packet->opcode = msg;
	packet->prm1 = packet->prm2 = packet->prm3 = packet->prm4 = NULL;
	char** prms = &packet->prm1;
	for (;; msg++)
	{
		switch (*msg)
		{
		case ',':
			*(msg++) = '\0';
			*(prms++) = msg;
			break;
		case '\n':
			*msg = '\0';
			return true;
		case '\0':
			return false;
		}
	}
}

