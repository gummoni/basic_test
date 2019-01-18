#include <stdio.h>
#include <string.h>
#include "dictionary.h"
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
	"SENDER,AXIS_Z1,R:10\n",
	"SENDER,AXIS_Z1,W:10,HELLOOOO\n",
	"SENDER,AXIS_Z1,N:I=5+(3*(1+2)+1)*2+4\n",
	"SENDER,AXIS_Z1,N:I=I*2+1\n",
	"SENDER,AXIS_Z1,N:I\n",
	"SENDER,AXIS_Z1,R:10\n",
	NULL,
};

void main() {

	dic_clear();
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
