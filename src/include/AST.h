#ifndef AST_H
#define AST_H
#include <stdlib.h>

/**
 * @brief An AST, or Abstract Syntax Tree, is a data structure commonly used in programming language compilers and interpreters.
 *        It represents the structure of the source code in a hierarchical manner, capturing the structure and relationships between different elements of the code.
 */

typedef struct AST_STRUCT {
  enum {
    AST_VARIABLE_DEFINITION, // let var = val;
    AST_VARIABLE, // var
    AST_FUNCTION_DEFINITION, // func name(args) { body };
    AST_FUNCTION_CALL, // name(args);
    AST_STRING, // "val"
    AST_INT, // 1234
    AST_COMPOUND, // { statements }
    AST_BINOP, // Binary Operator
    AST_STATEMENT_RETURN, // ret val;
    AST_NOOP // No operation
  } type;

  struct SCOPE_STRUCT* scope;

  // For variable definitions
  char* varDefVarName;
  struct AST_STRUCT* varDefVal;

  // For variable references
  char* varName;
  struct AST_STRUCT* varVal;

  // For function definitions
  char* funcDefName;
  struct AST_STRUCT** funcDefArgs;
  size_t funcDefArgsSize;
  struct AST_STRUCT* funcDefBody;

  // For function calls
  char* funcCallName;
  struct AST_STRUCT** funcCallArgs;
  size_t funcCallArgsSize;
  
  // For strings
  char* stringVal;

  // For numbers
  long numVal;

  // For compound statements
  struct AST_STRUCT** compoundVal;
  size_t compoundSize;

  // For BINOPS (binary operations)
  struct AST_STRUCT* value;
  struct AST_STRUCT* left;
  struct AST_STRUCT* right;
  int op;
} AST_T;

AST_T* initAST(int type);

#endif