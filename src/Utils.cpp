#include "Utils.h"
#include "RunAnalysis.h"

const char *WHITESPACES = " \t\n\r";

namespace dataflow {

std::string variable(const Value *Val) {
  std::string Code;
  raw_string_ostream SS(Code);
  Val->print(SS);
  Code.erase(0, Code.find_first_not_of(WHITESPACES));
  auto RetVal = Code.substr(0, Code.find_first_of(WHITESPACES));
  if (RetVal == "ret" || RetVal == "br" || RetVal == "store") {
    return Code;
  }
  if (RetVal == "i1" || RetVal == "i8" || RetVal == "i32" || RetVal == "i64") {
    RetVal = Code;
  }
  return RetVal;
}

bool isNumber(const std::string &s) {
  char *endPtr;
  std::strtol(s.c_str(), &endPtr, 10);
  return endPtr != s.c_str() && *endPtr == '\0';
}

std::vector<std::string> splitString(const std::string &input, char delimiter) {
  std::vector<std::string> result;
  std::stringstream ss(input);
  std::string token;

  while (getline(ss, token, delimiter)) {
    result.push_back(token);
  }

  return result;
}

void removeWhitespace(std::string &input) {
  input.erase(
      std::remove_if(input.begin(), input.end(),
                     [](char c) { return std::isspace(c) || c == '\0'; }),
      input.end());
}

std::string sliceString(const std::string &str, int i, int j) {
  return str.substr(i, j - i + 1);
}

bool isValidCVariableName(const std::string &str) {
  if (str.empty() || !std::isalpha(str[0]) && str[0] != '_') {
    return false;
  }

  for (int i = 1; i < str.length(); i++) {
    if (!std::isalnum(str[i]) && str[i] != '_') {
      return false;
    }
  }

  return true;
}

bool hasOnlyOneBalancedParentheses(const std::string &str) {
  std::stack<char> parenthesesStack;
  int balancedPairs = 0;

  for (char c : str) {
    if (c == '(') {
      parenthesesStack.push(c);
    } else if (c == ')') {

      // Unbalanced: encountered closing parenthesis without an opening one
      if (parenthesesStack.empty()) {
        return false;
      }
      parenthesesStack.pop();
      balancedPairs++;
    }
  }

  return parenthesesStack.empty() && balancedPairs == 1;
}

} // namespace dataflow