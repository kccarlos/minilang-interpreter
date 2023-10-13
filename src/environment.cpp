#include "environment.h"
#include "exceptions.h"

Environment::Environment(Environment *parent)
  : m_parent(parent) {
  assert(m_parent != this);
}

Environment::~Environment() {
}

// TODO: implement member functions
void Environment::defineVar(const std::string &name) {
  Value value = Value(0);
  variables[name] = value;
}

void Environment::setVar(const std::string &name, const Value &value) {
  if (variables.find(name) != variables.end()) {
    variables[name] = value;
    return;
  }
  if (m_parent) {
    m_parent->setVar(name, value);
  };
}

Value Environment::getVar(const std::string &name) const {
  if (variables.find(name) != variables.end()) {
    return variables.at(name);
  }
  if (m_parent) {
    return m_parent->getVar(name);
  };
}

bool Environment::isDefined(const std::string &name) const {
  if (variables.find(name) != variables.end()) {
    return true;
  }
  if (m_parent) {
    return m_parent->isDefined(name);
  } else {
    return false;
  }
}

bool Environment::isDefinedInCurrentScope(const std::string &name) const {
  return variables.find(name) != variables.end();
}

// A2 MS1: Add binding
void Environment::bind(const std::string &name, const Value &value) {
  variables[name] = value;
}