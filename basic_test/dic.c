#include "config.h"
#include "dic.h"
#include "bas_property.h"

#define DIC_IDX_KEY		0
#define DIC_IDX_VAL		1
static char _dictionary[VARIABLE_MEMORY_SIZE][2][VARIABLE_NAME_LENGTH];    //1kB[RAM]
static char _tmp[16];	//データ

//辞書取得
static char* internal_dic_get(char* key)
{
	//ビルドインパラメータ
	if (0 == strcmp(key, "POS"))	return itoa(POS, _tmp, 10);
	if (0 == strcmp(key, "SPD"))	return itoa(SPD, _tmp, 10);
	if (0 == strcmp(key, "ENC"))	return itoa(ENC, _tmp, 10);
	if (0 == strcmp(key, "AD0"))	return itoa(AD0, _tmp, 10);
	if (0 == strcmp(key, "AD1"))	return itoa(AD1, _tmp, 10);
	if (0 == strcmp(key, "AD2"))	return itoa(AD2, _tmp, 10);
	if (0 == strcmp(key, "AD3"))	return itoa(AD3, _tmp, 10);
	if (0 == strcmp(key, "TORQUE"))	return itoa(TORQUE, _tmp, 10);
	if (0 == strcmp(key, "POWER"))	return itoa(POWER, _tmp, 10);
	if (0 == strcmp(key, "DIR"))	return itoa(DIR, _tmp, 10);
	if (0 == strcmp(key, "MOVING"))	return itoa(MOVING, _tmp, 10);
	if (0 == strcmp(key, "REFL"))	return itoa(REFL, _tmp, 10);
	if (0 == strcmp(key, "REFR"))	return itoa(REFR, _tmp, 10);
	return NULL;
}

//辞書設定
static char* internal_dic_set(char* key, char* value)
{
	if (0 == strcmp(key, "TORQUE")) {
		bas_set_torque((0 == strcmp(value, "1") ? 1 : 0));
		return value;
	}
	if (0 == strcmp(key, "POWER"))
	{
		bas_set_power((0 == strcmp(value, "1") ? 1 : 0));
		return value;
	}
	if (0 == strcmp(key, "DIR"))
	{
		bas_set_dir((0 == strcmp(value, "1") ? 1 : 0));
		return value;
	}
	return NULL;
}

//変数の識別シンボルを削除
static void chomp_symbol(char* msg)
{
	int idx = strlen(msg) - 1;
	char ch = msg[idx];
	if ('%' == ch || '$' == ch) msg[idx] = '\0';
}

//辞書検索
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
	//満杯
	state.err_no = err_var_full;
	return false;
}

//辞書初期化
void dic_clear(void) {
	int i;
	for (i = 0; i < VARIABLE_MEMORY_SIZE; i++) {
		_dictionary[i][DIC_IDX_KEY][0] = _dictionary[i][DIC_IDX_VAL][0] = '\0';
	}
}

//辞書から値取得
char* dic_get(char* key) {
	int idx = 0;
	char * result = internal_dic_get(key);

	if (NULL != result) return result;

	if (search_key(key, &idx))
	{
		return _dictionary[idx][DIC_IDX_VAL];
	}
	else
	{
		//満杯の時に来る
		return "\0";
	}
}

//辞書に登録
char* dic_set(char* key, char* value) {
	int idx = 0;
	char* result = internal_dic_set(key, value);

	if (NULL != result) return result;

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
		//満杯の時に来る
		return "\0";
	}
}
