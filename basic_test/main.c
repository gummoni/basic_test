#include "config.h"
#include "bas_script.h"
#include "bas.h"


void main() {

	bas_init();

	for (int i = 0; i < 100; i++)
	{
		bas_job();
	}
	printf("-------\n");
 }
