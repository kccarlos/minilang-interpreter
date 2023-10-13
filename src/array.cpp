#include "array.h"
#include "value.h"
#include "exceptions.h"


Array::Array(std::vector<Value> array)
    : ValRep(VALREP_ARRAY)
    , m_array(array)
    , m_size(array.size()) {
}

Array::~Array() {
}

Value Array::set(int index, const Value& val, const Location &location) {
  if (index >= 0 && index < m_size) {
    m_array[index] = val;
    return val;
  }
  EvaluationError::raise(location,
                         "Array index out of bound: %d\n", index);
}

Value Array::push(const Value& val) {
  m_array.push_back(val);
  ++m_size;
  return val;
}

Value Array::pop(const Location &location) {
  if (m_size == 0) {
    EvaluationError::raise(location,
                           "Popping an empty array \n");
  }
  Value last_val = m_array.back();
  m_array.pop_back();
  --m_size;
  return last_val;
}

Value Array::get(int index, const Location &location) const {
  if (index >= 0 && index < m_size) {
    return m_array[index];
  }
  EvaluationError::raise(location, "Array index out of bound: %d\n", index);
}
