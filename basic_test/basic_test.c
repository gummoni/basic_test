#include <stdio.h>
#include "dictionary.h"
#include "script_reader.h"
#include "csv_basic.h"

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


static char msg1[64] = "SENDER,AXIS_Z1,R:10\n";
static char msg2[64] = "SENDER,AXIS_Z1,W:10,HELLOOOO\n";
static char msg3[64] = "SENDER,AXIS_Z1,W:10,KK\n";
static char msg4[64] = "SENDER,AXIS_Z1,R:10\n";

void main() {

	bas_main(msg1);
	bas_main(msg2);
	bas_main(msg3);
	bas_main(msg4);






	printf("Hello world\n");

	reader_init(script_text);
	dic_clear();

	while (0 != decoder_execute()) {
		printf("---");
	}
	printf(dic_get("I%"));
	printf(dic_get("S$"));
	printf("\n============\n\n\n\n");
 }
