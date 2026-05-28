/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256,
  TK_NUM
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},

  {"0[xX][0-9a-fA-F](_?[0-9a-fA-F])*", TK_NUM},
  {"0(_?[0-7])*", TK_NUM},
  {"[1-9](_?[0-9])*", TK_NUM},

  {"\\+", '+'},
  {"-", '-'},
  {"\\*", '*'},
  {"/", '/'},

  {"\\(", '('},
  {"\\)", ')'},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;
  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[64];
} Token;

static Token tokens[256] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //    i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;
          default:
            if (nr_token >= ARRLEN(tokens)) {
              printf("too many tokens\n");
              return false;
            }
            if (substr_len >= sizeof(tokens[nr_token].str)) {
              printf("token is too long: %.*s\n", substr_len, substr_start);
              return false;
            }
            tokens[nr_token].type = rules[i].token_type;
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token ++;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static int parse_idx = 0;
static bool parse_ok = true;

static word_t parse_add();

static void parse_error(const char *msg) {
  if (parse_ok) {
    printf("bad expression near token %d: %s\n", parse_idx, msg);
  }
  parse_ok = false;
}

static int peek() {
  return parse_idx < nr_token ? tokens[parse_idx].type : 0;
}

static int digit_value(char ch) {
  if (ch >= '0' && ch <= '9') return ch - '0';
  if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
  if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
  return -1;
}

static word_t parse_number(const char *s) {
  int base = 10;
  int pos = 0;
  word_t val = 0;

  if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
    base = 16;
    pos = 2;
  }
  else if (s[0] == '0') {
    base = 8;
  }

  for (; s[pos] != '\0'; pos ++) {
    if (s[pos] == '_') continue;
    int digit = digit_value(s[pos]);
    if (digit < 0 || digit >= base) {
      parse_error("invalid number");
      return 0;
    }
    val = val * base + digit;
  }
  return val;
}

static word_t parse_primary() {
  if (!parse_ok) return 0;

  if (peek() == TK_NUM) {
    word_t val = parse_number(tokens[parse_idx].str);
    parse_idx ++;
    return val;
  }

  if (peek() == '(') {
    parse_idx ++;
    word_t val = parse_add();
    if (peek() != ')') {
      parse_error("expect ')'");
      return 0;
    }
    parse_idx ++;
    return val;
  }

  parse_error("expect number or '('");
  return 0;
}

static word_t parse_unary() {
  if (peek() == '+') {
    parse_idx ++;
    return parse_unary();
  }
  if (peek() == '-') {
    parse_idx ++;
    return 0 - parse_unary();
  }
  return parse_primary();
}

static word_t parse_mul() {
  word_t val = parse_unary();

  while (parse_ok && (peek() == '*' || peek() == '/')) {
    int op = peek();
    parse_idx ++;
    word_t rhs = parse_unary();
    if (op == '*') {
      val *= rhs;
    }
    else {
      if (rhs == 0) {
        parse_error("division by zero");
        return 0;
      }
      val /= rhs;
    }
  }

  return val;
}

static word_t parse_add() {
  word_t val = parse_mul();

  while (parse_ok && (peek() == '+' || peek() == '-')) {
    int op = peek();
    parse_idx ++;
    word_t rhs = parse_mul();
    if (op == '+') {
      val += rhs;
    }
    else {
      val -= rhs;
    }
  }

  return val;
}

word_t expr(char *e, bool *success) {
  *success = false;

  if (!make_token(e)) {
    return 0;
  }

  if (nr_token == 0) {
    printf("empty expression\n");
    return 0;
  }

  parse_idx = 0;
  parse_ok = true;
  word_t res = parse_add();
  if (parse_ok && parse_idx != nr_token) {
    parse_error("unexpected token");
  }
  *success = parse_ok;
  return res;
}
