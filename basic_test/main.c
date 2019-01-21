#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "csv_basic.h"
#include "bas_packet.h"
#include "script_reader.h"

//転送データ
static char* msgs[] = {
	//変数（数字）計算テスト
	"SENDER,AXIS_Z1,N:I=5+(3*(1+2)+1)*2+4\n",
	"SENDER,AXIS_Z1,N:I=I*2+1\n",
	"SENDER,AXIS_Z1,N:I\n",
	//変数（文字列）計算テスト
	"SENDER,AXIS_Z1,N:J$=\"12+34\"\n",
	"SENDER,AXIS_Z1,N:J$=\"GOOD=\" + J$\n",
	"SENDER,AXIS_Z1,N:J$\n",
	//アドレスRWテスト
	"SENDER,AXIS_Z1,R:10\n",
	"SENDER,AXIS_Z1,W:10,I=0\n",
	"SENDER,AXIS_Z1,R:10\n",
	"SENDER,AXIS_Z1,W:11,I=I+1",
	"SENDER,AXIS_Z1,W:12,IF I<10 11 13\n",
	"SENDER,AXIS_Z1,W:13,NOTIFY USER I\n",
	"SENDER,AXIS_Z1,W:14,\n",
	"SENDER,AXIS_Z1,W:15,GOSUB 20\n",
	"SENDER,AXIS_Z1,W:16,GOTO 30\n",
	"SENDER,AXIS_Z1,W:20,NOTIFY USER I\n",
	"SENDER,AXIS_Z1,W:21,RETURN\n",
	"SENDER,AXIS_Z1,W:30,END\n",
	//プログラム実行テスト
	"SENDER,AXIS_Z1,S:10",
	//-----------------------------
	NULL,
};

void main() {
	char msg[64];

	bas_init();

	//プログラム書込み&実行
	for (int i = 0; msgs[i] != NULL; i++)
	{
		strcpy(msg, msgs[i]);
		bas_main(msg);
	}

	//スクリプト実行中
	msg[0] = '\0';
	while (0 < state.run_no)
	{
		printf("%d:", state.run_no);
		bas_main(msg);
		printf("\n");
	}
	printf("\n\n\n\n\n\n\n\n");
 }
