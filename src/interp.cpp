#include <cassert>
#include <algorithm>
#include <memory>
#include <set>
#include "ast.h"
#include "node.h"
#include "exceptions.h"
#include "function.h"
#include "interp.h"

Interpreter::Interpreter(Node *ast_to_adopt)
  : m_ast(ast_to_adopt) {
}

Interpreter::~Interpreter() {
  delete m_ast;
}

void Interpreter::analyzeHelper(Node* node, std::set<std::string>& definedVariables) {
  int tag = node->get_tag();

  // Handle variable definition
  if (tag == AST_VARDEF) {
    Node* varNode = node->get_kid(0);
    std::string identifier = varNode->get_str();
    definedVariables.insert(identifier);
  }

  // Handle variable reference
  else if (tag == AST_VARREF) {
    std::string identifier = node->get_str();
    if (definedVariables.find(identifier) == definedVariables.end()) {
      EvaluationError::raise(node->get_loc(),
                             "%s", ("Reference to undefined name '" + identifier + "'").c_str());
    }
  }

  // Recur for each child node
  for (unsigned i = 0; i < node->get_num_kids(); ++i) {
    analyzeHelper(node->get_kid(i), definedVariables);
  }
}

void Interpreter::analyze() {
  // Done: implement
  std::set<std::string> definedVariables = {
      "print", "println", "readint", "mkarr", "len", "get", "set", "push", "pop", "substr", "strcat", "strlen"
  };

  analyzeHelper(m_ast, definedVariables);
}


Value Interpreter::execute() {
  // Done: implement
  Environment* global_env = new Environment();

  // Bind intrinsic functions
  global_env->bind("print", Value(&intrinsic_print));
  global_env->bind("println", Value(&intrinsic_println));
  global_env->bind("readint", Value(&intrinsic_readint));
  global_env->bind("mkarr", &array_mkarr);
  global_env->bind("len", &array_len);
  global_env->bind("get", &array_get);
  global_env->bind("set", &array_set);
  global_env->bind("push", &array_push);
  global_env->bind("pop", &array_pop);
  global_env->bind("substr", &string_substr);
  global_env->bind("strcat", &string_strcat);
  global_env->bind("strlen", &string_strlen);

  // Will hold the value of the last statement executed
  Value result;

  // Execute each statement in the unit
  int nkids = m_ast -> get_num_kids();
  for (int i = 0; i < nkids; i++) {
    Node *statm_ast = m_ast->get_kid(i);
    if (statm_ast->get_tag() == AST_FUNCTION) {
      result = create_function(statm_ast, global_env);
    } else {
      result = evaluate(statm_ast->get_kid(0), global_env);
    }
  }
  delete global_env;
  return result;
}

Value Interpreter::execute(Node *node, Environment *env) {

  // Will hold the value of the last statement executed
  Value result;

  // Execute each statement in the unit
  int nkids = node -> get_num_kids();
  for (int i = 0; i < nkids; i++) {
    Node *statm_ast = node->get_kid(i);
    result = evaluate(statm_ast->get_kid(0), env);
  }
  return result;
}

Value Interpreter::create_function(Node* node, Environment* env) {
  // if astnode is function definition
  Node* identifierNode = node->get_kid(0);
  std::string fn_name = identifierNode->get_str();

  std::vector<std::string> param_names;
  Node* paramListNode = node->get_kid(1);
  int numParams = paramListNode->get_num_kids();
  for (int i = 0; i < numParams; i++) {
    Node* paramNode = paramListNode->get_kid(i);
    std::string param_name = paramNode->get_str();
    param_names.push_back(param_name);
  }

  Node *body;
  if (node->get_num_kids() == 3) {
    body = node->get_kid(2);
  } else {
    EvaluationError::raise(node->get_loc(), "No function body found");
  }

  // create function object
  Value fn_val(new Function(fn_name, param_names, env, body));

  // bind function to environment
  env->bind(fn_name, Value(fn_val));
  Value value(0);
  return value;
}

Value Interpreter::evaluate(Node* node, Environment* env) {
  if (!node) {
    return {0}; // Return default (0) value for null node
  }
  int tag = node->get_tag();
  switch (tag) {
    case AST_INT_LITERAL: {
      // if astnode is literal
      // return literal value encoded by astnode
      int intValue = std::stoi(node->get_str());
      Value value(intValue);
      return value;
    };
    case AST_VARREF: {
      // astnode is variable reference
      // return result of looking up value of variable
      std::string identifier = node->get_str();
      Value value = env->getVar(check_exists(identifier, env, node));
      return value;
    };
    case AST_ASSIGN: {
      // if astnode is variable assignment
      // childval ← evaluate(astnode.children[0])
      Node* varNode = node->get_kid(0);
      Node* exprNode = node->get_kid(1);
      Value childval = evaluate(exprNode, env);
      std::string identifier = varNode->get_str();
      // update value of variable and return childval
      env->setVar(check_exists(identifier, env, node), childval);
      return childval;
    };
    case AST_VARDEF: {
      // astnode is variable definition
      // define a new variable in the environment, this statement return 0
      Node* varNode = node->get_kid(0);
      std::string identifier = varNode->get_str();
      if (env->isDefinedInCurrentScope(identifier)) {
        EvaluationError::raise(node->get_loc(),
                               "%s", ("Variable '" + identifier + "' already defined").c_str());
      };
      env->defineVar(identifier);
      Value value(0);
      return value;
    };
    case AST_IF: {
      int nkids = node->get_num_kids();
      Node* conditionNode = node->get_kid(0);
      Node* blockNode = node->get_kid(1);
      Value conditionValue = evaluate(conditionNode, env);
      if (nkids == 2) {
        // if (condition) {block}
        if (conditionValue.get_ival() != 0) {
          // if condition is true, execute block
          Environment* new_env = new Environment(env);
          Value result = execute(blockNode, new_env);
          delete new_env;
        }
      } else if (nkids == 3) {
        // if (condition) {block} else {block}
        Node* elseBlockNode = node->get_kid(2);
        if (conditionValue.get_ival() != 0) {
          // if condition is true, execute block
          Environment* new_env = new Environment(env);
          Value result = execute(blockNode, new_env);
          delete new_env;
        } else {
          // if condition is false, execute else block
          Environment* new_env = new Environment(env);
          Value result = execute(elseBlockNode, new_env);
          delete new_env;
        }
      }
      return Value(0);
    };
    case AST_WHILE: {
      Node* conditionNode = node->get_kid(0);
      Node* blockNode = node->get_kid(1);
      Environment* new_env = new Environment(env);
      Value conditionValue = evaluate(conditionNode, new_env);
      delete new_env;
      while (conditionValue.get_ival() != 0) {
        Environment* new_env = new Environment(env);
        Value result = execute(blockNode, new_env);
        conditionValue = evaluate(conditionNode, new_env);
        delete new_env;
      }
      return Value(0);
    };
    case AST_FNCALL: {
      // if astnode is function call
      Node* identifierNode = node->get_kid(0);
      std::string identifier = identifierNode->get_str();

      // get function from environment
      Value functionValue = env->getVar(check_exists(identifier, env, node));
      enum ValueKind kind = functionValue.get_kind();
      switch (kind) {
        case VALUE_FUNCTION: {
          // if function is user-defined
          Function* function = functionValue.get_function();

          // Function call environment
          Environment* fncall_env = new Environment(function -> get_parent_env());

          // check number of arguments
          Node* argListNode = node->get_kid(1);
          int numArgs = argListNode->get_num_kids();

          if (numArgs != function->get_num_params()) {
            EvaluationError::raise(node->get_loc(),
                                   "%s", ("Function '" + identifier + "' requires " +
                                          std::to_string(function->get_num_params()) + " arguments").c_str());
          }

          // prepare arguments
          for (int i = 0; i < numArgs; i++) {
            Node* argNode = argListNode->get_kid(i);
            Value argValue = evaluate(argNode, env);
            std::string param_name = function->get_param_name(i);
            fncall_env->bind(param_name, argValue);
          }

          Environment *block_env = new Environment(fncall_env);

          // execute function
          Value result = execute(function->get_body(), block_env);

          // delete function call environment
          delete fncall_env;
          delete block_env;

          return result;

        };
        case VALUE_INTRINSIC_FN: {
          // if function is intrinsic
          IntrinsicFn function = functionValue.get_intrinsic_fn();

          // prepare arguments
          Node* argListNode = node->get_kid(1);
          int numArgs = argListNode->get_num_kids();
          Value arguments[numArgs];
          for (int i = 0; i < numArgs; i++) {
            Node* argNode = argListNode->get_kid(i);
            Value argValue = evaluate(argNode, env);
            arguments[i] = argValue;
          }

          // call intrinsic function
          Value result = function(arguments, numArgs, node->get_loc(), this);
          return result;
        };
        default:
          EvaluationError::raise(node->get_loc(), "Invalid function type");
      }

    };
    case AST_STRING_LITERAL: {
      // if astnode is string literal
      return Value(new String(node->get_str()));
    };
    default:
      // astnode is binary operation
      Value left = evaluate_and_check_numeric(node, env, 0);

      // Done: support for short-circuiting and result casting
      if (tag == AST_LOGICAL_AND) {
        if (left.get_ival() == 0) {
          return Value(0);
        }
        Value right = evaluate_and_check_numeric(node, env, 1);
        return Value(right.get_ival() ? 1 : 0);
      } else if (tag == AST_LOGICAL_OR) {
        if (left.get_ival() != 0) {
          return Value(1);
        }
        Value right = evaluate_and_check_numeric(node, env, 1);
        return Value(right.get_ival() ? 1 : 0);
      }
      Value right = evaluate_and_check_numeric(node, env, 1);

      switch (tag) {
        case AST_ADD: {
          int res = right.get_ival() + left.get_ival();
          return Value(res);
        };
        case AST_SUB: {
          int res = left.get_ival() - right.get_ival();
          return Value(res);
        };
        case AST_MULTIPLY: {
          int res = right.get_ival() * left.get_ival();
          return Value(res);
        };
        case AST_DIVIDE: {
          if (right.get_ival() == 0) {
            EvaluationError::raise(node->get_loc(), "Attempt to divide by 0");
          }
          int res = left.get_ival() / right.get_ival();
          return Value(res);
        };
        case AST_LESS: {
          int res = left.get_ival() < right.get_ival();
          return Value(res);
        };
        case AST_LESSEQUAL: {
          int res = left.get_ival() <= right.get_ival();
          return Value(res);
        };
        case AST_GREATER: {
          int res = left.get_ival() > right.get_ival();
          return Value(res);
        };
        case AST_GREATEREQUAL: {
          int res = left.get_ival() >= right.get_ival();
          return Value(res);
        };
        case AST_ISEQUAL: {
          int res = left.get_ival() == right.get_ival();
          return Value(res);
        };
        case AST_ISNOTEQUAL: {
          int res = left.get_ival() != right.get_ival();
          return Value(res);
        };
        default:
          break;
      }
      throw std::runtime_error("Invalid AST node to evaluate");
  }
  return Value(0);
}


Value Interpreter::evaluate_and_check_numeric(Node* node, Environment* env, int i) {
  Value result = evaluate(node->get_kid(i), env);

  if (!result.is_numeric()) {
    EvaluationError::raise(node->get_loc(), "Cannot perform arithmetic calculation on non-numeric values");
  }

  return result;
}

std::string Interpreter::check_exists(std::string identifier, Environment* env, Node *node) {
  if (!env->isDefined(identifier)) {
    EvaluationError::raise(node->get_loc(),
                           "%s", ("Function not defined before invoking '" + identifier + "'").c_str());
  } else {
    return identifier;
  }
}