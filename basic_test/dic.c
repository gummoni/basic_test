#include "config.h"
#include "dic.h"

#define DIC_IDX_KEY		0
#define DIC_IDX_VAL		1
static char _dictionary[VARIABLE_MEMORY_SIZE][2][VARIABLE_NAME_LENGTH];    //1kB[RAM]


//•Ï”‚Ì¯•ÊƒVƒ“ƒ{ƒ‹‚ğíœ
static void chomp_symbol(char* msg)
{
	int idx = strlen(msg) - 1;
	char ch = msg[idx];
	if ('%' == ch || '$' == ch) msg[idx] = '\0';
}

//«‘ŒŸõ
static bool search_key(char* key, int* result)
{
	chomp_symbol(key);

	int idx = 0;
	for (idx = 0; idx < VARIABLE_MEMORY_SIZE; idx++) {
		char* dic_key = _dictionary[idx][DIC_IDX_KEY];
		if ((0 == strcmp(key, dic_key)) || (0 == strlen(dic_key)))
		{
			*result = idx;
			return true;
		}
	}
	//–”t
	state.err_no = err_var_full;
	return false;
}

//«‘‰Šú‰»
void dic_clear(void) {
	int i;
	for (i = 0; i < VARIABLE_MEMORY_SIZE; i++) {
		_dictionary[i][DIC_IDX_KEY][0] = _dictionary[i][DIC_IDX_VAL][0] = '\0';
	}
}

//«‘‚©‚ç’læ“¾
char* dic_get(char* key) {
	int idx = 0;
	if (search_key(key, &idx))
	{
		return _dictionary[idx][DIC_IDX_VAL];
	}
	else
	{
		//–”t‚Ì‚É—ˆ‚é
		return "\0";
	}
}

//«‘‚É“o˜^
char* dic_set(char* key, char* value) {
	int idx;
	if (search_key(key, &idx))
	{
		char* dic_key = _dictionary[idx][DIC_IDX_KEY];
		char* dic_val = _dictionary[idx][DIC_IDX_VAL];
		strcpy(dic_key, key);
		strcpy(dic_val, value);
		return dic_val;
	}
	else
	{
		//–”t‚Ì‚É—ˆ‚é
		return "\0";
	}
}
