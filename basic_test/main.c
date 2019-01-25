#include "config.h"
#include "bas_script.h"
#include "bas.h"


void main() {

	bas_init();

	while (true)
	{
		bas_job();
	}
 }
