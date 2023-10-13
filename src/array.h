#ifndef ARRAY_H
#define ARRAY_H

#include <vector>
#include "valrep.h"
#include "value.h"

class Value;

class Array : public ValRep {
private:
  int m_size;
  std::vector<Value> m_array;


public:
  Array(std::vector<Value> array);
  virtual ~Array();

  int len() const {return m_size;};
  Value get(int index, const Location &location) const;
  Value set(int index, const Value& val, const Location &location);
  Value push(const Value& val);
  Value pop(const Location &location);

};
#endif //ARRAY_H
