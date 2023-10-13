#ifndef INTERP_H
#define INTERP_H

#include <set>
#include "value.h"
#include "exceptions.h"
#include "array.h"
#include "string.h"
#include "environment.h"

class Node;
class Location;

class Interpreter {
private:
  Node *m_ast;

public:
  Interpreter(Node *ast_to_adopt);
  ~Interpreter();

  void analyze();
  Value execute();
  Value execute(Node *node, Environment *env);

  // DONE: add intrinsic functions definitions
  static Value intrinsic_print(Value args[], unsigned num_args,
                               const Location &loc, Interpreter *interp) {
    if (num_args != 1)
      EvaluationError::raise(loc, "Wrong number of arguments passed to print function");
    printf("%s", args[0].as_str().c_str());
    return Value();
  }

  static Value intrinsic_println(Value args[], unsigned num_args,
                               const Location &loc, Interpreter *interp) {
    if (num_args != 1)
      EvaluationError::raise(loc, "Wrong number of arguments passed to println function");
    printf("%s\n", args[0].as_str().c_str());
    return Value();
  }

  static Value intrinsic_readint(Value args[], unsigned num_args,
                                 const Location &loc, Interpreter *interp) {
    // Check if any arguments are passed, and raise an error if so
    if (num_args != 0) {
      EvaluationError::raise(loc, "readint does not take any arguments");
    }

    int input_value;
    int read_result = scanf("%d", &input_value);

    // Check for read errors or unexpected input format
    if (read_result != 1) {
      EvaluationError::raise(loc, "Failed to read an integer from standard input");
    }

    return Value(input_value);
  }

  // Functions for array
  static Value array_mkarr(Value args[], unsigned num_args,
                               const Location &loc, Interpreter *interp) {
    std::vector <Value> values;
    for(unsigned i=0; i<num_args; i++){
      values.push_back(args[i]);
    }
    return Value(new Array(values));
  }

  static Value array_len(Value args[], unsigned num_args,
                                const Location &loc, Interpreter *interp) {
    if (num_args != 1)
      EvaluationError::raise(loc, "Wrong number of arguments passed to array length function");
    if (args[0].get_kind() != VALUE_ARRAY)
      EvaluationError::raise(loc, "First argument to array len function must be an array");
    return Value(args[0].get_array()->len());
  }

  static Value array_get(Value args[], unsigned num_args,
                                const Location &loc, Interpreter *interp) {
    if (num_args != 2)
      EvaluationError::raise(loc, "Wrong number of arguments passed to array get function");
    if (args[0].get_kind() != VALUE_ARRAY)
      EvaluationError::raise(loc, "First argument to array get function must be an array");
    if (args[1].get_kind() != VALUE_INT)
      EvaluationError::raise(loc, "Second argument to array get function must be an integer");
    int index = args[1].get_ival();
    return args[0].get_array()->get(index, loc);
  }

  static Value array_set(Value args[], unsigned num_args,
                                const Location &loc, Interpreter *interp) {
    if (num_args != 3)
      EvaluationError::raise(loc, "Wrong number of arguments passed to array set function");
    if (args[0].get_kind() != VALUE_ARRAY)
      EvaluationError::raise(loc, "First argument to array set function must be an array");
    if (args[1].get_kind() != VALUE_INT)
      EvaluationError::raise(loc, "Second argument to array set function must be an integer");
    int index = args[1].get_ival();
    return args[0].get_array()->set(index, args[2], loc);
  }

  static Value array_push(Value args[], unsigned num_args,
                                const Location &loc, Interpreter *interp) {
    if (num_args != 2)
      EvaluationError::raise(loc, "Wrong number of arguments passed to array push function");
    if (args[0].get_kind() != VALUE_ARRAY)
      EvaluationError::raise(loc, "First argument to array push function must be an array");
    return args[0].get_array()->push(args[1]);
  }

  static Value array_pop(Value args[], unsigned num_args,
                                const Location &loc, Interpreter *interp) {
    if (num_args != 1)
      EvaluationError::raise(loc, "Wrong number of arguments passed to array pop function");
    if (args[0].get_kind() != VALUE_ARRAY)
      EvaluationError::raise(loc, "First argument to array pop function must be an array");
    return args[0].get_array()->pop(loc);
  }

  // functions for string
  static Value string_substr(Value args[], unsigned num_args,
                               const Location &loc, Interpreter *interp) {
    if (num_args != 3)
      EvaluationError::raise(loc, "Wrong number of arguments passed to string substr function");
    if (args[0].get_kind() != VALUE_STRING)
      EvaluationError::raise(loc, "First argument to string substr function must be a string");
    if (args[1].get_kind() != VALUE_INT)
      EvaluationError::raise(loc, "Second argument to string substr function must be an integer");
    if (args[2].get_kind() != VALUE_INT)
      EvaluationError::raise(loc, "Third argument to string substr function must be an integer");
    int start = args[1].get_ival();
    int end = args[2].get_ival();
    return args[0].get_string()->substr(start, end, loc);
  }

  static Value string_strcat(Value args[], unsigned num_args,
                               const Location &loc, Interpreter *interp) {
    if (num_args != 2)
      EvaluationError::raise(loc, "Wrong number of arguments passed to string strcat function");
    if (args[0].get_kind() != VALUE_STRING)
      EvaluationError::raise(loc, "First argument to string strcat function must be a string");
    if (args[1].get_kind() != VALUE_STRING)
      EvaluationError::raise(loc, "Second argument to string strcat function must be a string");
    return args[0].get_string()->strcat(args[1]);
  }

  static Value string_strlen(Value args[], unsigned num_args,
                               const Location &loc, Interpreter *interp) {
    if (num_args != 1)
      EvaluationError::raise(loc, "Wrong number of arguments passed to string length function");
    if (args[0].get_kind() != VALUE_STRING)
      EvaluationError::raise(loc, "First argument to string length function must be a string");
    return Value(args[0].get_string()->strlen());
  }


private:
  // DONE: private member functions
  Value evaluate(Node *node, Environment *env);
  void analyzeHelper(Node *node, std::set<std::string> &definedVariables);
  Value create_function(Node* node, Environment* env);
  Value evaluate_and_check_numeric(Node *node, Environment *env, int i);
  std::string check_exists(std::string identifier, Environment* env, Node *node);
};

#endif // INTERP_H
