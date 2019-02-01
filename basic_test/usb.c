#include "config.h"
#include "usb.h"

static BAS_PACKET usb_packet;


//転送データ
static char* usb_msgs[] = {
	//IFテスト
	"SENDER,AXIS_Z1,W:10,I=0\n",
	"SENDER,AXIS_Z1,W:11,IF I<>0 *TEST\n",
	"SENDER,AXIS_Z1,W:12,I=1\n",
	"SENDER,AXIS_Z1,W:13,INVOKE USER I\n",
	"SENDER,AXIS_Z1,W:14,GOTO 22\n",
	"SENDER,AXIS_Z1,W:15,*TEST\n",
	"SENDER,AXIS_Z1,W:16,GOSUB *TEST2 11\n",
	"SENDER,AXIS_Z1,W:17,GOTO 13\n",
	"SENDER,AXIS_Z1,W:18,RETURN\n",
	"SENDER,AXIS_Z1,W:19,*TEST2 J\n",
	"SENDER,AXIS_Z1,W:20,I=J\n",
	"SENDER,AXIS_Z1,W:21,RETURN\n",
	"SENDER,AXIS_Z1,W:22,A=1\n",
	"SENDER,AXIS_Z1,W:23,RETURN\n",
	//	"SENDER,AXIS_Z1,W:24,C$=\"A==1 AND B==1\"",
	//	"SENDER,AXIS_Z1,W:25,IF C$ 26 27",
		"SENDER,AXIS_Z1,W:26,RETURN\n",
		"SENDER,AXIS_Z1,W:27,RETURN\n",
		"SENDER,AXIS_Z1,S:10\n",


		//変数（数字）計算テスト
		"SENDER,AXIS_Z1,I:I=5+(3*(1+2)+1)*2+4\n",
		"SENDER,AXIS_Z1,I:I=I*2+1\n",
		"SENDER,AXIS_Z1,I:I\n",

		//変数（文字列）計算テスト
		"SENDER,AXIS_Z1,I:J$=\"12+34\"\n",
		"SENDER,AXIS_Z1,I:J$=\"GOOD=\" + J$\n",
		"SENDER,AXIS_Z1,I:J$\n",

		//アドレスRWテスト
		"SENDER,AXIS_Z1,R:10\n",
		"SENDER,AXIS_Z1,W:10,I=0\n",
		"SENDER,AXIS_Z1,R:10\n",
		"SENDER,AXIS_Z1,W:11,I=I+1\n",
		"SENDER,AXIS_Z1,W:12,IF I<10 11 *JMP\n",
		"SENDER,AXIS_Z1,W:13,*JMP\n",
		"SENDER,AXIS_Z1,W:14,INVOKE USER I\n",
		"SENDER,AXIS_Z1,W:15,GOSUB 20\n",
		"SENDER,AXIS_Z1,W:16,GOTO 30\n",
		"SENDER,AXIS_Z1,W:20,INVOKE USER I\n",
		"SENDER,AXIS_Z1,W:21,RETURN\n",
		"SENDER,AXIS_Z1,W:30,RETURN\n",

		//プログラム実行テスト
		"SENDER,AXIS_Z1,S:10\n",
		//-----------------------------
		NULL,
};
static int usb_i = 0;


//1行読込み
bool usb_read(char** msg, int* length)
{
	return false;
	//*msg = usb_msgs[usb_i];
	//if (NULL != *msg)
	//{
	//	usb_i++;
	//	*length = strlen(*msg);
	//	return true;
	//}
	//return false;
}

//データ送信
void usb_write(char* msg, int length)
{
	printf("USB:%s", msg);
}
