#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/visitor.h"
#include "include/scope.h"

// Built-in functions
static AST_T* builtinFuncPrint(AST_T** args, size_t argsSize) {
  // Output the arguments as arg1 arg2 arg3
  // There is no space at the end

  int i = 0;
  while (i < argsSize - 1) {
    AST_T* visited = visit(args[i]);
    switch (visited->type) {
      case STRING: printf("%s ", visited->stringVal); break;
      case INT: printf("%ld ", visited->intVal); break;
      case CHAR: printf("%c ", visited->charVal); break;
      case BOOL: printf("%s ", visited->boolVal ? "true" : "false"); break;
      default: printf("%p ", (void*) visited); break;
    }

    i++;
  }

  AST_T* visited = visit(args[i]);
  switch (visited->type) {
    case STRING: printf("%s", visited->stringVal); break;
    case INT: printf("%ld", visited->intVal); break;
    case CHAR: printf("%c", visited->charVal); break;
    case BOOL: printf("%s", visited->boolVal ? "true" : "false"); break;
    default: printf("%p", (void*) visited); break;
  }

  return initAST(AST_NOOP);
}

static AST_T* builtinFuncPrintln(AST_T** args, size_t argsSize) {
  // Output the arguments as arg1 arg2 arg3
  // There is a new line created at the end

  // If there are no arguments, print a new line
  if(argsSize == 0) {
    printf("\n");
    return initAST(AST_NOOP);
  }

  int i = 0;
  while (i < argsSize - 1) {
    AST_T* visited = visit(args[i]);
    switch (visited->type) {
      case STRING: printf("%s ", visited->stringVal); break;
      case INT: printf("%ld ", visited->intVal); break;
      case CHAR: printf("%c ", visited->charVal); break;
      case BOOL: printf("%s ", visited->boolVal ? "true" : "false"); break;
      default: printf("%p ", (void*) visited); break;
    }

    i++;
  }

  AST_T* visited = visit(args[i]);
  switch (visited->type) {
    case STRING: printf("%s\n", visited->stringVal); break;
    case INT: printf("%ld\n", visited->intVal); break;
    case CHAR: printf("%c\n", visited->charVal); break;
    case BOOL: printf("%s\n", visited->boolVal ? "true" : "false"); break;
    default: printf("%p\n", (void*) visited); break;
  }

  return initAST(AST_NOOP);
}

static AST_T* builtinFuncClear(size_t argsSize) {
  // Clear the terminal

  // If there are arguments, print an error message
  if (argsSize != 0) {
    printf("Invalid amount of arguments passed into function `clear`\n");
    exit(1);
  }
  
  system("clear");

  return initAST(AST_NOOP);
}

static AST_T* builtinFuncExit(AST_T** args, size_t argsSize) {
  // Exit the program with a status code
  // If there are no arguments, exit with code 0 silently
  if(argsSize == 0) {
    exit(0);
  }

  // If there are more than one argument, print an error message and exit
  if(argsSize > 1) {
    printf("Invalid amount of arguments passed into function `exit`\n");
    exit(1);
  }

  // Get the argument
  AST_T* visited = visit(args[0]);

  // If the argument isn't an integer, print an error message and exit
  if (visited->type != INT) {
    printf("Invalid argument passed into function `exit`\n");
    exit(1);
  }
  
  // Exit with the argument's value
  printf("Exited with code %ld.", visited->intVal); 
  exit(visited->intVal);

  return initAST(AST_NOOP);
}

AST_T* visit(AST_T* node) {
  // Check the type of the node and visit accordingly
  switch (node->type) {
    case AST_VARIABLE_DEFINITION: return visitVarDef(node); break;
    case AST_VARIABLE: return visitVar(node); break;
    case AST_FUNCTION_DEFINITION: return visitFuncDef(node); break;
    case AST_FUNCTION_CALL: return visitFuncCall(node); break;
    case AST_COMPOUND: return visitCompound(node); break;
    case AST_STATEMENT_RETURN: printf("Implementing return soon.\n"); exit(1); break;
    default: return node; break;
  }
}

AST_T* visitVarDef(AST_T* node) {
  scopeAddVarDef(node->scope, node); // Add the variable definition to the scope

  return node;
}

AST_T* visitVar(AST_T* node) {
  AST_T* varDef = scopeGetVarDef(node->scope, node->varName); // Get the variable definition from the scope

  // If the variable definition is not found, send an error
  if (!varDef) {
    printf("Variable `%s` not found.\n", node->varName);
    exit(1);
  }

  // If the variable definition is found, return its value
  return visit(varDef->varDefVal);
}

AST_T* visitFuncDef(AST_T* node) {
  scopeAddFuncDef(node->scope, node); // Add the function definition to the scope

  return node;
}

AST_T* visitFuncCall(AST_T* node) {
  // Built-in functions
  if (strcmp(node->funcCallName, "print") == 0)
    return builtinFuncPrint(node->funcCallArgs, node->funcCallArgsSize);
  if (strcmp(node->funcCallName, "println") == 0)
    return builtinFuncPrintln(node->funcCallArgs, node->funcCallArgsSize);
  if (strcmp(node->funcCallName, "clear") == 0)
    return builtinFuncClear(node->funcCallArgsSize);
  if (strcmp(node->funcCallName, "exit") == 0)
    return builtinFuncExit(node->funcCallArgs, node->funcCallArgsSize);
  
  // Custom functions
  AST_T* funcDef = scopeGetFuncDef(node->scope, node->funcCallName);

  // Not found
  if (!funcDef)
    return (void*) 0;
  

  // Invalid amount of arguments called
  if (node->funcCallArgsSize != funcDef->funcDefArgsSize) {
    printf("Invalid amounmt of arguments passed into function `%s`\n", node->funcCallName);
    exit(1);
  }

  // Go through the arguments
  for (size_t i = 0; i < funcDef->funcDefArgsSize; i++) {
    // Get the variable and its value from the defined arguments
    AST_T* var = (AST_T*) funcDef->funcDefArgs[i];
    AST_T* val = (AST_T*) node->funcCallArgs[i];

    // Create a new variable definition with the value of the argument passed in the call
    AST_T* varDef = initAST(AST_VARIABLE_DEFINITION);

    // Give the variable its value
    varDef->varDefVal = val;
    varDef->type = val->type;
    
    // Copy the name of the defined argument into the new variable definition
    varDef->varDefVarName = calloc(strlen(var->varName) + 1, sizeof(char));
    strcpy(varDef->varDefVarName, var->varName);

    // Add it to the function's scope
    scopeAddVarDef(funcDef->funcDefBody->scope, varDef);
  }
    
  // Found
  return visit(funcDef->funcDefBody);
}

AST_T* visitCompound(AST_T* node) {
  for (size_t i = 0; i < node->compoundSize; i++) {
    AST_T* child = node->compoundVal[i];
    if (child->type == AST_VARIABLE_DEFINITION) {
      visitVarDef(child);
    }
  }

  for (size_t i = 0; i < node->compoundSize; i++) {
    AST_T* child = node->compoundVal[i];
    if (child->type != AST_VARIABLE_DEFINITION) {
      visit(child);
    }
  }

  return node;
}