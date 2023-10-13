#include "cpputil.h"
#include "exceptions.h"
#include "valrep.h"
#include "function.h"
#include "value.h"
#include "array.h"
#include "string.h"

Value::Value(int ival)
  : m_kind(VALUE_INT) {
  m_atomic.ival = ival;
}

Value::Value(Function *fn)
  : m_kind(VALUE_FUNCTION)
  , m_rep(fn) {
  m_rep = fn;
  m_rep->add_ref(); // Added: increment reference counting
}

Value::Value(IntrinsicFn intrinsic_fn)
  : m_kind(VALUE_INTRINSIC_FN) {
  m_atomic.intrinsic_fn = intrinsic_fn;
}

Value::Value(Array *arr)
  : m_kind(VALUE_ARRAY)
  , m_rep(arr) {
  m_rep = arr;
  m_rep->add_ref();
}

Value::Value(String *str)
  : m_kind(VALUE_STRING)
  , m_rep(str) {
  m_rep = str;
  m_rep->add_ref();
}

Value::Value(const Value &other)
  : m_kind(VALUE_INT) {
  // Just use the assignment operator to copy the other Value's data
  *this = other;
}

Value::~Value() {
  // Done: handle reference counting (detach from ValRep, if any)
  detach();
}

Value &Value::operator=(const Value &rhs) {
  if (this != &rhs &&
      !(is_dynamic() && rhs.is_dynamic() && m_rep == rhs.m_rep)) {
    // Done: handle reference counting (detach from previous ValRep, if any)
    detach();
    m_kind = rhs.m_kind;
    if (is_dynamic()) {
      // attach to rhs's dynamic representation
      m_rep = rhs.m_rep;
      // Done: handle reference counting (attach to the new ValRep)
      m_rep->add_ref();
    } else {
      // copy rhs's atomic representation
      m_atomic = rhs.m_atomic;
    }
  }
  return *this;
}

Function *Value::get_function() const {
  assert(m_kind == VALUE_FUNCTION);
  return m_rep->as_function();
}

Array *Value::get_array() const {
  assert(m_kind == VALUE_ARRAY);
  return m_rep->as_array();
}

String *Value::get_string() const {
  assert(m_kind == VALUE_STRING);
  return m_rep->as_string();
}

std::string Value::as_str() const {
  switch (m_kind) {
  case VALUE_INT:
    return cpputil::format("%d", m_atomic.ival);
  case VALUE_FUNCTION:
    return cpputil::format("<function %s>", m_rep->as_function()->get_name().c_str());
  case VALUE_INTRINSIC_FN:
    return "<intrinsic function>";
  case VALUE_ARRAY:
    return array_as_str();
  case VALUE_STRING:
    return m_rep->as_string()->get_actual_string();
  default:
    // this should not happen
    RuntimeError::raise("Unknown value type %d", int(m_kind));
  }
}

// Done: implementations of additional member functions
void Value::detach() {
  if (is_dynamic()) {
    m_rep->remove_ref();
    if (m_rep->get_num_refs() == 0) {
      delete m_rep;
    }
    m_rep = nullptr;
    m_kind = VALUE_INT; // reset to VALUE_INT
  }
}

std::string Value::array_as_str() const {
  std::string result = "[";
  for (int i = 0; i < m_rep->as_array()->len(); i++) {
    if (i > 0)
      result += ", ";
    result += m_rep->as_array()->get(i, Location()).as_str();
  }
  result += "]";
  return result;
}
