#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "csv_basic.h"
#include "bas_packet.h"
#include "script_reader.h"

static char* script_text = "10 I=5+(3*(1+2)+1)*2+4\n\
30 IF I%<10 THEN 20 ELSE 40\n\
40 S$=\"OK\"\n\
50 END\n\
60 S$+=\"##HELLO##\"\n";

/*
14 S$ = \"+\"\n\
15 FOR I=0 TO 5 STEP 2\n\
16   S$ += \"-\"\n\
17 NEXT\n\
*/
char msg[64];


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
	"SENDER,AXIS_Z1,W:14,END",
	//プログラム実行テスト
	"SENDER,AXIS_Z1,S:10",
	//-----------------------------
	NULL,
};

void main() {

	bas_init();

	for (int i = 0; msgs[i] != NULL; i++)
	{
		strcpy(msg, msgs[i]);
		bas_main(msg);
	}

	msg[0] = '\0';
	while (true)
	{
		bas_main(msg);
	}
 }
