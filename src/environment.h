#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <cassert>
#include <map>
#include <string>
#include "value.h"

class Environment {
private:
  Environment *m_parent;
  // Done: representation of environment (map of names to values)
  std::map<std::string, Value> variables;
  // copy constructor and assignment operator prohibited
  Environment(const Environment &);
  Environment &operator=(const Environment &);

public:
  Environment(Environment *parent = nullptr);
  ~Environment();

  // Done: add member functions allowing lookup, definition, and assignment
  void defineVar(const std::string &name);
  void setVar(const std::string &name, const Value &value);
  Value getVar(const std::string &name) const;
  bool isDefined(const std::string &name) const;


  // A2 MS1: Add binding
  void bind(const std::string &name, const Value &value);
  bool isDefinedInCurrentScope(const std::string &name) const;
};

#endif // ENVIRONMENT_H
