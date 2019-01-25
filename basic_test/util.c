#include "config.h"
#include "util.h"

//送信バッファ
static char send_buf[PROGRAM_LINE_COUNT];

//返信電文作成
char* make_message(char* from, char* to, char cmd, char* message)
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

	return send_buf;
}

//パラメータ解析
bool parse_parameter(BAS_PACKET* packet, char* msg, char separator)
{
	packet->prm1 = msg;
	packet->prm2 = packet->prm3 = packet->prm4 = packet->prm5 = packet->prm6 = NULL;
	char** prms = &packet->prm1;

	bool qout = false;
	for (;; msg++)
	{
		char ch = *msg;
		if ('"' == ch)
		{
			qout ^= true;
		}
		if ((separator == ch) && !qout)
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

//ラベル照合
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

//ラベル位置取得（行番号も含む）
int label_search(char* label, bool* is_label)
{
	int idx;
	char ch = label[0];

	if ('0' <= ch && ch <= '9')
	{
		//数字
		*is_label = false;
		return strtol(label, NULL, 0);
	}
	if ('*' == ch)
	{
		//ジャンプ先
		for (idx = 0; idx < PROGRAM_LINE_MAX; idx++)
		{
			if (label_compare(label, &program_areas[idx][0]))
			{
				*is_label = true;
				return idx;
			}
		}
	}
	//該当なし
	state.err_no = err_jump;
	return 0;
}

//宛先確認（トピックID取得）
bool get_listen_id(char* topic, int* result)
{
	int i;
	for (i = 0; i < 10; i++)
	{
		if (0 == strcmp(topic, program_areas[i]))
		{
			*result = i;
			return true;
		}
	}

	*result = -1;
	return false;
}

//パケットクリア
void clear_packet(BAS_PACKET* packet)
{
	packet->recv_length = 0;
	packet->state = 0;
	packet->is_quot = false;
	packet->sender = packet->recv_buff;
	packet->reciever = NULL;
	packet->command = '\0';
	packet->prm1 = NULL;
	packet->prm2 = NULL;
	packet->prm3 = NULL;
	packet->prm4 = NULL;
	packet->prm5 = NULL;
	packet->prm6 = NULL;
	packet->response = NULL;
}
