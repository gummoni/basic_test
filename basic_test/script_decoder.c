#include <stdbool.h>
#include "script_decoder.h"
#include "script_reader.h"
#include "cmd.h"


//�擪�̍s�ԍ���T��
static inline bool decode_seek_no()
{
	while (true)
	{
		if (!reader_next()) return false;
		if (CUR_NEWLINE == reader.token) continue;
		if (VAR_NUM != reader.token) return false;
		return true;
	}
}

//�X�N���v�g��́i��ʁj
bool decoder_execute(void)
{
	if (!decode_seek_no())	return false;
	if (!reader_next())		return false;
	switch (reader.token)
	{
	case SYN_STR:			return cmd_exe(reader.cmd);
	case SYN_NUM:			return rpn_num();
	case CUR_LABEL:			return reader_seek_to_newline();
	case CUR_NEWLINE:		return true;
	}
	return false;
}
