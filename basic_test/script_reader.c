#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "script_reader.h"
#include "dictionary.h"


script_reader reader;
char tmp_key[VARIABLE_NAME_LENGTH];
char tmp_value[VARIABLE_NAME_LENGTH];
char tmp_eval_left[VARIABLE_NAME_LENGTH];
char tmp_eval_right[VARIABLE_NAME_LENGTH];
script_token tmp_eval_op;


static inline char chk_char() {
  return *reader.rp;
}
static inline void seek(int offset) {
  reader.rp += offset;
}

static inline char get_char(void) {
  char ch = *reader.rp;
  if ('\0' == ch) return 0;
  reader.rp++;
  return ch;
}

static inline char skip_space(void) {
  while (1) {
    char ch = *reader.rp;
    if ('\0' == ch) return 0;
    reader.rp++;
    if ((' ' == ch) || ('\t' == ch)) continue;
    return ch;
  }
}

static inline int parse_newline(void) {
  reader.token = CUR_NEWLINE;
  reader.context[0] = '\0';
  while (1) {
    char ch = get_char();
    if ('\r' == ch || '\n' == ch) continue;
    if ('\0' != ch) seek(-1);
    return TRUE;
  }
}

static inline int parse_num(char ch) {
  int idx = 0;
  reader.token = VAR_NUM;
  reader.context[idx++] = ch;

  if ('x' == chk_char()) {
    reader.context[idx++] = 'x';
    seek(+1);
    while (1) {
      ch = toupper(get_char());
      if (('0' <= ch && ch <= '9') || ('A' <= ch && ch <= 'F')) {
        reader.context[idx++] = ch;
      } else {
        break;
      }
    }
  } else {
    while (1) {
      ch = get_char();
      if ('0' <= ch && ch <= '9') {
        reader.context[idx++] = ch;
      } else {
        break;
      }
    }
  }
  if (0 != ch) seek(-1);
  reader.context[idx] = '\0';
  return TRUE;
}

static inline int check_cmd(void) {
	char* command = reader.context;
	if (0 == strcmp("IF", command)) return TRUE;
	if (0 == strcmp("THEN", command)) return TRUE;
	if (0 == strcmp("ELSE", command)) return TRUE;
	if (0 == strcmp("ELSEIF", command)) return TRUE;
	if (0 == strcmp("FOR", command)) return TRUE;
	if (0 == strcmp("TO", command)) return TRUE;
	if (0 == strcmp("STEP", command)) return TRUE;
	if (0 == strcmp("NEXT", command)) return TRUE;
	if (0 == strcmp("GOTO", command)) return TRUE;
	if (0 == strcmp("GOSUB", command)) return TRUE;
	if (0 == strcmp("RETURN", command)) return TRUE;
	if (0 == strcmp("END", command)) return TRUE;
	if (0 == strcmp("REQUEST", command)) return TRUE;
	if (0 == strcmp("NOTIFY", command)) return TRUE;
	if (0 == strcmp("REM", command)) return TRUE;
	return FALSE;
}

static inline int parse_cmd(char ch) {
  int idx = 0;
  reader.context[idx++] = ch;

  while (1) {
    ch = get_char();
    if ('A' <= ch && ch <= 'Z' || '_' == ch) {
      reader.context[idx++] = ch;
	} else if ('%' == ch) {
		reader.context[idx++] = '%';
		reader.token = SYN_NUM;
		break;
	} else if ('$' == ch) {
      reader.context[idx++] = ch;
      reader.token = SYN_STR;
      break;
    } else {
      if (0 != ch) seek(-1);
 	  if (check_cmd()) {
		  reader.token = SYN_CMD;
	  } else {
		  reader.context[idx++] = '%';
		  reader.token = SYN_NUM;
	  }
      break;
    }
  }
  reader.context[idx] = '\0';
  return TRUE;
}

static inline int parse_str(void) {
  int idx = 0;

  reader.token = VAR_STR;
  while (1) {
    char ch = get_char();
    if ('\\' == ch) {
      ch = get_char();
      if ('\t' == ch) ch = '\t';
      else if ('\r' == ch) ch = '\r';
      else if ('\n' == ch) ch = '\n';
      else if ('\"' == ch) ch = '\"';
    } else if ('\"' == ch) {
      break;
    }
    reader.context[idx++] = ch;
  }
  reader.context[idx] = '\0';
  return TRUE;
}

static inline int parse_plus(void) {
  reader.token = CALC_PLUS;
  strcpy(reader.context, "+");
  return TRUE;
}

static inline int parse_minus(void) {
  reader.token = CALC_MINUS;
  strcpy(reader.context, "-");
  return TRUE;
}

static inline int parse_mul(void) {
  reader.token = CALC_MUL;
  strcpy(reader.context, "*");
  return TRUE;
}

static inline int parse_div(void) {
  reader.token = CALC_DIV;
  strcpy(reader.context, "/");
  return TRUE;
}

static inline int parse_equ(void) {
  char ch = get_char();
  if ('=' == ch) {
    reader.token = OPE_EQ;
    strcpy(reader.context, "==");
  } else {
    if ('\0' != ch) seek(-1);
    reader.token = CALC_EQUAL;
    strcpy(reader.context, "=");
  }
  return TRUE;
}

static inline int parse_gt(void) {
	reader.token = PRI_GT;
	return TRUE;
}

static inline int parse_ge(void) {
	reader.token = PRI_GE;
	return TRUE;
}

static inline int parse_ge_ne_gt(void) {
  char ch = get_char();
  if ('=' == ch) {
    reader.token = OPE_GE;
    strcpy(reader.context, "<=");
  } else if ('>' == ch) {
    reader.token = OPE_NE;
    strcpy(reader.context, "<>");
  } else {
    if ('\0' != ch) seek(-1);
    reader.token = OPE_GT;
    strcpy(reader.context, "<");
  }
  return TRUE;
}

static inline int parse_le_lt(void) {
  char ch = get_char();
  if ('=' == ch) {
    reader.token = OPE_LE;
    strcpy(reader.context, ">=");
  } else {
    if ('\0' != ch) seek(-1);
    reader.token = OPE_LT;
    strcpy(reader.context, ">");
  }
  return TRUE;
}

static inline int parse_label(void) {
  int idx = 0;
  reader.token = CUR_LABEL;
  reader.context[idx++] = '@';

  while (1) {
    char ch = get_char();
    if (('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9') || '_' == ch) {
      reader.context[idx++] = ch;
    } else {
      if (0 != ch) seek(-1);
      reader.context[idx] = '\0';
      return TRUE;
    }
  }
}

static inline void reader_seek_top() {
  reader.rp = reader.text;
}

int reader_seek_to(char* label) {
  strcpy(tmp_key, label);
  reader_seek_top();

  while (1) {
    char* old_rp = reader.rp;

    if (FALSE == reader_next()) return FALSE;
    if (VAR_NUM == reader.token) {
      //1.check line number
      if (0 == strcmp(tmp_key, reader.context)) {
        reader.rp = old_rp;
        return TRUE;
      }

      //2.check label
      if (FALSE == reader_next()) return FALSE;
      if (CUR_LABEL == reader.token) {
        if (0 == strcmp(tmp_key, reader.context)) {
          reader.rp = old_rp;
          return TRUE;
        }
      }
    }

    //3.skip to newline
    if (FALSE == reader_seek_to_newline()) return FALSE;
  }
}

int reader_seek_to_newline(void) {
  while (1) {
    if (CUR_NEWLINE == reader.token) return TRUE;
    if (FALSE == reader_next()) return FALSE;
  }
}

void reader_init(char* text) {
  reader.text = reader.rp = text;
}

int reader_next(void) {
  char ch = toupper(skip_space());
  if ('(' == ch) return parse_gt();
  if (')' == ch) return parse_ge();
  if ('\r' == ch || '\n' == ch) return parse_newline();
  if ('0' <= ch && ch <= '9') return parse_num(ch);
  if ('A' <= ch && ch <= 'Z' || '_' == ch) return parse_cmd(ch);
  if ('"' == ch) return parse_str();
  if ('+' == ch) return parse_plus();
  if ('-' == ch) return parse_minus();
  if ('*' == ch) return parse_mul();
  if ('/' == ch) return parse_div();
  if ('=' == ch) return parse_equ();
  if ('<' == ch) return parse_ge_ne_gt();
  if ('>' == ch) return parse_le_lt();
  if ('@' == ch) return parse_label();

  reader.token = CUR_NEWLINE;
  return FALSE;
}

int reader_get(int flags) {
	if (FALSE == reader_next()) return FALSE;
	return (0 != (flags && reader.token));
}

//���݂̕�������擾
int reader_get_string(char* dst) {
	if (FALSE == reader_next()) return FALSE;
	if (0 != (reader.token && (SYN_STR | VAR_STR))) return FALSE;
	strcpy(dst, reader.context);
	return TRUE;
}

//���݂̒l���擾
int reader_get_value(int* dst) {
	if (FALSE == reader_next()) return FALSE;
	if (0 != (reader.token && SYN_NUM)) {
		*dst = strtol(reader.context, NULL, 0);
	}
	else if (0 != (reader.token && VAR_NUM)) {
		*dst = strtol(dic_get(reader.context), NULL, 0);
	}

	strcpy(dst, reader.context);
	return eval();
}