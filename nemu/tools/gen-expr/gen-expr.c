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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

// this should be enough
static char buf[65536] = {};
static char c_buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static int idx = 0;
static int c_idx = 0;
static bool gen_ok = true;

static uint32_t choose(uint64_t x) {
    return (uint32_t) (rand() % x);
}

static void append_to(char *dst, int *pos, const char *fmt, ...) {
    if (!gen_ok) return;

    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(dst + *pos, sizeof(buf) - *pos, fmt, ap);
    va_end(ap);

    if (n < 0 || n >= (int)sizeof(buf) - *pos) {
        gen_ok = false;
        return;
    }
    *pos += n;
}

static void append_both(const char *s) {
    append_to(buf, &idx, "%s", s);
    append_to(c_buf, &c_idx, "%s", s);
}

static uint32_t rand_u32() {
    return ((uint32_t)rand() << 16) ^ (uint32_t)rand();
}

static void append_digits_with_separators(const char *digits) {
    for (int i = 0; digits[i] != '\0'; i ++) {
        append_to(buf, &idx, "%c", digits[i]);
        if (digits[i + 1] != '\0' && choose(4) == 0) {
            append_to(buf, &idx, "_");
        }
    }
}

static void gen_num_text(uint32_t val) {
    char digits[32];

    switch (choose(3)) {
        case 0:
            snprintf(digits, sizeof(digits), "%u", val);
            append_digits_with_separators(digits);
            break;
        case 1:
            snprintf(digits, sizeof(digits), "%x", val);
            append_to(buf, &idx, "0x");
            append_digits_with_separators(digits);
            break;
        default:
            if (val == 0) {
                append_to(buf, &idx, "0");
            }
            else {
                snprintf(digits, sizeof(digits), "%o", val);
                append_to(buf, &idx, "0");
                append_digits_with_separators(digits);
            }
            break;
    }
    append_to(c_buf, &c_idx, "%uu", val);
}

static uint32_t gen_rand_expr_depth(int depth);

static uint32_t gen_nonzero_expr(int depth) {
    for (int i = 0; i < 100; i ++) {
        int old_idx = idx;
        int old_c_idx = c_idx;
        bool old_ok = gen_ok;

        uint32_t val = gen_rand_expr_depth(depth);
        if (gen_ok && val != 0) return val;

        idx = old_idx;
        c_idx = old_c_idx;
        gen_ok = old_ok;
        buf[idx] = '\0';
        c_buf[c_idx] = '\0';
    }

    gen_num_text(1);
    return 1;
}

static uint32_t gen_rand_expr_depth(int depth) {
    if (depth > 5 || choose(4) == 0) {
        uint32_t val = rand_u32();
        gen_num_text(val);
        return val;
    }

    switch (choose(4)) {
        case 0: {
            append_both("(");
            uint32_t val = gen_rand_expr_depth(depth + 1);
            append_both(")");
            return val;
        }
        case 1: {
            append_both("+ ");
            return gen_rand_expr_depth(depth + 1);
        }
        case 2: {
            append_both("- ");
            return 0 - gen_rand_expr_depth(depth + 1);
        }
        default: {
            append_both("(");
            uint32_t lhs = gen_rand_expr_depth(depth + 1);
            char op = "+-*/"[choose(4)];
            append_to(buf, &idx, " %c ", op);
            append_to(c_buf, &c_idx, " %c ", op);
            uint32_t rhs = (op == '/') ? gen_nonzero_expr(depth + 1) : gen_rand_expr_depth(depth + 1);
            append_both(")");

            switch (op) {
                case '+': return lhs + rhs;
                case '-': return lhs - rhs;
                case '*': return lhs * rhs;
                default: return lhs / rhs;
            }
        }
    }
}

static void gen_rand_expr() {
    gen_rand_expr_depth(0);
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    idx = 0;
    c_idx = 0;
    gen_ok = true;
    gen_rand_expr();
    buf[idx] = '\0';
    c_buf[c_idx] = '\0';

    if (!gen_ok) continue;

    sprintf(code_buf, code_format, c_buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);
    if (ret != 1) continue;

    printf("%u %s\n", result, buf);
  }
  return 0;
}
