#include "string.h"
#include "exceptions.h"

String::String(std::string a_string)
  : ValRep(VALREP_STRING)
  , m_string(a_string) {
}

String::~String() {
}

Value String::substr(int start, int end, Location loc) const {
  return Value(new String(m_string.substr(start, end)));
}

Value String::strcat(Value b_string) const {
  std::string b = b_string.get_string()->m_string;
  return Value(new String(m_string + b));
}

std::string String::get_actual_string() {
  return m_string;
}

