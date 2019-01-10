#include <stdio.h>
#include "stack.h"
#include "dictionary.h"
#include "rpn.h"
#include "script_reader.h"
#include "script_decoder.h"

static char* script_text = "10 I=5+(3*(1+2)+1)*2+4\n\
14 S$ = \"+\"\n\
15 FOR I=0 TO 5 STEP 2\n\
16   S$ += \"-\"\n\
17 NEXT\n\
30 IF I%<10 THEN 20 ELSE 40\n\
40 S$=\"OK\"\n\
50 END\n\
60 S$+=\"##HELLO##\"\n";


void main() {
	printf("Hello world");

	reader_init(script_text);
	dic_clear();

	while (0 != decoder_execute()) {
		printf("---");
	}
	printf(dic_get("I%"));
	printf(dic_get("S$"));
	printf("============");
 }
