#include "monitor/expr.h"

#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <string>

#include "dut.h"

namespace {

class Parser {
 public:
  explicit Parser(const char *input) : input_(input == nullptr ? "" : input) {}

  uint32_t Parse(bool *success) {
    const uint32_t value = ParseExpr(success);
    SkipSpaces();
    if (!*success || *input_ != '\0') {
      *success = false;
      return 0;
    }
    return value;
  }

 private:
  const char *input_;

  void SkipSpaces() {
    while (*input_ != '\0' && std::isspace(static_cast<unsigned char>(*input_))) {
      ++input_;
    }
  }

  uint32_t ParseExpr(bool *success) {
    uint32_t lhs = ParseTerm(success);
    while (*success) {
      SkipSpaces();
      if (*input_ != '+' && *input_ != '-') {
        break;
      }
      const char op = *input_++;
      const uint32_t rhs = ParseTerm(success);
      if (!*success) {
        return 0;
      }
      lhs = (op == '+') ? (lhs + rhs) : (lhs - rhs);
    }
    return lhs;
  }

  uint32_t ParseTerm(bool *success) {
    uint32_t lhs = ParseFactor(success);
    while (*success) {
      SkipSpaces();
      if (*input_ != '*' && *input_ != '/') {
        break;
      }
      const char op = *input_++;
      const uint32_t rhs = ParseFactor(success);
      if (!*success || (op == '/' && rhs == 0)) {
        *success = false;
        return 0;
      }
      lhs = (op == '*') ? (lhs * rhs) : (lhs / rhs);
    }
    return lhs;
  }

  uint32_t ParseFactor(bool *success) {
    SkipSpaces();
    if (*input_ == '(') {
      ++input_;
      const uint32_t value = ParseExpr(success);
      SkipSpaces();
      if (!*success || *input_ != ')') {
        *success = false;
        return 0;
      }
      ++input_;
      return value;
    }
    if (*input_ == '-') {
      ++input_;
      return 0u - ParseFactor(success);
    }
    if (*input_ == '$') {
      return ParseRegister(success);
    }
    return ParseNumber(success);
  }

  uint32_t ParseRegister(bool *success) {
    ++input_;
    std::string reg_name;
    while (std::isalnum(static_cast<unsigned char>(*input_)) || *input_ == '_') {
      reg_name.push_back(*input_++);
    }
    uint32_t value = 0;
    if (sim_get_reg_value(reg_name.c_str(), &value)) {
      return value;
    }
    *success = false;
    return 0;
  }

  uint32_t ParseNumber(bool *success) {
    SkipSpaces();
    char *end = nullptr;
    const unsigned long value = std::strtoul(input_, &end, 0);
    if (end == input_) {
      *success = false;
      return 0;
    }
    input_ = end;
    return static_cast<uint32_t>(value);
  }
};

}  // namespace

uint32_t expr_eval(const char *expr, bool *success) {
  if (success == nullptr) {
    return 0;
  }
  *success = true;
  Parser parser(expr);
  return parser.Parse(success);
}
