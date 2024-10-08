#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "include/parser.h"

parser_T* initParser(lexer_T* lexer) {
  parser_T* parser = calloc(1, sizeof(parser_T)); // Allocate memory for the parser
  parser->lexer = lexer; // Set the lexer of the parser
  parser->currentToken = getNextToken(lexer); // Set the current token of the parser
  parser->prevToken = parser->currentToken; // Set the previous token of the parser
  parser->scope = initScope(); // Set the scope of the parser

  return parser;
}

void eat(parser_T* parser, int tokenType) {
  // Check if the current token is of the correct type
  if ((int) parser->currentToken->type != tokenType) {
    printf(
      "Unexpected token `%s` with type %d\n", (char*) parser->currentToken->val, 
      parser->currentToken->type
    );
    printf(
      "Expected token with type %d, which represents ", 
      tokenType
    );
    switch (tokenType) {
      case TOKEN_ID: printf("an identifier.\n"); break;
      case TOKEN_EQUALS: printf("=\n"); break;
		  case TOKEN_STRING: printf("a string.\n"); break;
		  case TOKEN_CHAR: printf("a character.\n"); break;
		  case TOKEN_INT: printf("an integer\n"); break;
		  case TOKEN_SEMI: printf(";\n"); break;
		  case TOKEN_LPAREN: printf("(\n"); break;
		  case TOKEN_RPAREN: printf(")\n"); break;
		  case TOKEN_LBRACE: printf("{\n"); break;
		  case TOKEN_RBRACE: printf("}\n"); break;
		  case TOKEN_LBRACKET: printf("[\n"); break;
      case TOKEN_RBRACKET: printf("]\n"); break;
		  case TOKEN_COMMA: printf(",\n"); break;
		  case TOKEN_PLUS: printf("+\n"); break;
		  case TOKEN_MINUS: printf("-\n"); break;
		  case TOKEN_MULTIPLY: printf("*\n"); break;
		  case TOKEN_DIVIDE: printf("/\n"); break;
		  case TOKEN_POW: printf("^\n"); break;
		  case TOKEN_MODULO: printf("%%\n"); break;
      case TOKEN_COLON: printf(":\n"); break;
		  case TOKEN_EOF: printf("EOF\n"); break;
      default: printf("an unknown token.\n"); break;
    }
    exit(1);
  }

  // Set the previous token to the current token and advance the current token
  parser->prevToken = parser->currentToken;
  parser->currentToken = getNextToken(parser->lexer);
}

AST_T* parseStatement(parser_T* parser, scope_T* scope, int type) {
  // Check the type of the current token and parse accordingly
  switch (type) {
    case ANY:
      switch (parser->currentToken->type) {
        case TOKEN_STRING: return parseString(parser, scope); break;
        case TOKEN_ID: return parseID(parser, scope); break;
        case TOKEN_CHAR: return parseChar(parser, scope); break;
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_INT: return parseIntExpr(parser, scope); break;
        default: return initAST(AST_NOOP); break;
      } break;

    case INT:
      switch (parser->currentToken->type) {
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_INT: return parseIntExpr(parser, scope); break;
        default: printf("Expected an integer, but got `%s` with type %d\n", (char*) parser->currentToken->val, parser->currentToken->type); exit(1);
      } break;

    case FLOAT:
      printf("Floats are currently unsupported.\n"); break;

    case CHAR:
      if (parser->currentToken->type != TOKEN_CHAR) {
        printf("Expected a character, but got `%s` with type %d\n", (char*) parser->currentToken->val, parser->currentToken->type);
        exit(1);
      } 
      return parseChar(parser, scope);
      break;

    case BOOL:
      if (parser->currentToken->type != TOKEN_ID) {
        printf("Expected a boolean, but got `%s` with type %d\n", (char*) parser->currentToken->val, parser->currentToken->type);
        exit(1);
      }
      return parseBool(parser, scope); 
      break;

    case STRING:
      if (parser->currentToken->type != TOKEN_STRING) {
        printf("Expected a string, but got `%s` with type %d\n", (char*) parser->currentToken->val, parser->currentToken->type);
        exit(1);
      } 
      return parseString(parser, scope);
      break;

    case VOID:
      printf("Void is currently unsupported.\n");
      break;
  }
  return initAST(AST_NOOP);
}

AST_T* parseStatements(parser_T* parser, scope_T* scope) {
  // Create a compound AST node to hold the statements and allocate memory for the statements
  AST_T* compound = initAST(AST_COMPOUND);
  compound->compoundVal = calloc(1, sizeof(struct AST_STRUCT));
  
  // Parse the first statement
  AST_T* statement = parseStatement(parser, scope, ANY);
  statement->scope = scope;

  // Add the statement to the compound node
  compound->compoundVal[0] = statement;
  compound->compoundSize += 1;
  compound->scope = scope;  

  // Parse the rest of the statements, using a semicolon as the seperator
  while (parser->currentToken->type == TOKEN_SEMI) {
    eat(parser, TOKEN_SEMI);

    AST_T* statement = parseStatement(parser, scope, ANY);

    // If there is another statement, increase the size of the compound and reallocate memory for the compound's values
    if (statement) {
      compound->compoundSize += 1;    

      compound->compoundVal = realloc(
        compound->compoundVal, 
        (compound->compoundSize + 1) * sizeof(struct AST_STRUCT)
      );

      compound->compoundVal[compound->compoundSize - 1] = statement;
    }
  }

  return compound;
}

AST_T* parseFuncDef(parser_T* parser, scope_T* scope) {
  AST_T* funcDef = initAST(AST_FUNCTION_DEFINITION);

  eat(parser, TOKEN_ID); // func
  char* funcName = parser->currentToken->val;
  funcDef->funcDefName = calloc(
    strlen(funcName) + 1, 
    sizeof(char)
  );
  strcpy(funcDef->funcDefName, funcName);

  eat(parser, TOKEN_ID); // function name

  eat(parser, TOKEN_LPAREN); // (

  if (parser->currentToken->type != TOKEN_RPAREN) {
    // The arguments of the function
    funcDef->funcDefArgs = calloc(1, sizeof(struct AST_STRUCT*));

    AST_T* arg = parseVar(parser, scope);

    funcDef->funcDefArgsSize += 1;
    funcDef->funcDefArgs[funcDef->funcDefArgsSize - 1] = arg;

    while (parser->currentToken->type == TOKEN_COMMA) {
      eat(parser, TOKEN_COMMA); // ,
      funcDef->funcDefArgsSize += 1;

      funcDef->funcDefArgs = realloc(
        funcDef->funcDefArgs, 
        funcDef->funcDefArgsSize * sizeof(struct AST_STRUCT*)
      );    

      AST_T* arg = parseVar(parser, scope);   
      funcDef->funcDefArgs[funcDef->funcDefArgsSize - 1] = arg;
    }
  }

  eat(parser, TOKEN_RPAREN); // )
  eat(parser, TOKEN_LBRACE); // {  

  // The body of the function
  funcDef->funcDefBody = parseStatements(parser, scope);

  eat(parser, TOKEN_RBRACE); // }

  funcDef->scope = scope;
  visitFuncDef(funcDef);

  return funcDef;
}

AST_T* parseFuncCall(parser_T* parser, scope_T* scope) {
  // Parse a function call and create an AST node with the function name and arguments as the value
  AST_T* funcCall = initAST(AST_FUNCTION_CALL);
  funcCall->funcCallName = (char*) parser->prevToken->val;

  bool isBuiltIn = false;
  if (strcmp(funcCall->funcCallName, "print") == 0 || strcmp(funcCall->funcCallName, "println") == 0 || strcmp(funcCall->funcCallName, "clear") == 0 || strcmp(funcCall->funcCallName, "exit") == 0) 
    isBuiltIn = true;
  
    
  AST_T* funcDef = scopeGetFuncDef(scope, funcCall->funcCallName);
  
  if (!funcDef && !isBuiltIn) {
    printf("Undefined function `%s`\n", funcCall->funcCallName);
    exit(1);
  }

  eat(parser, TOKEN_LPAREN);

  // If there are arguments
  if (parser->currentToken->type != TOKEN_RPAREN) {
    funcCall->funcCallArgs = calloc(1, sizeof(struct AST_STRUCT));
    
    AST_T* statement = parseStatement(parser, scope, ANY);
    funcCall->funcCallArgs[0] = statement;
    funcCall->funcCallArgsSize += 1;

    if (!isBuiltIn)
      funcDef->funcDefArgs[0]->type = statement->type;

    // Go through the arguments of the function
    while(parser->currentToken->type == TOKEN_COMMA) {
      eat(parser, TOKEN_COMMA);

      AST_T* statement = parseStatement(parser, scope, ANY);
      funcCall->funcCallArgsSize += 1;
      
      funcCall->funcCallArgs = realloc(
        funcCall->funcCallArgs, 
        (funcCall->funcCallArgsSize + 1) * sizeof(struct AST_STRUCT)
      );

      if (!isBuiltIn && funcCall->funcCallArgsSize > funcDef->funcDefArgsSize) {
        printf("Invalid amount of arguments passed into function `%s`\n", funcCall->funcCallName);
        exit(1);
      }

      funcCall->funcCallArgs[funcCall->funcCallArgsSize - 1] = statement;

      if (!isBuiltIn)
        funcDef->funcDefArgs[funcCall->funcCallArgsSize - 1]->type = statement->type;
    }
  }

  eat(parser, TOKEN_RPAREN);

  funcCall->scope = scope; // Add it to the scope

  return funcCall;
}

AST_T* parseVarDef(parser_T* parser, scope_T* scope) {
  AST_T* varDef = initAST(AST_VARIABLE_DEFINITION);

  eat(parser, TOKEN_ID); // let
  char* varDefVarName = parser->currentToken->val;
  varDef->varDefVarName = varDefVarName;
  eat(parser, TOKEN_ID); // variable name

  if (parser->currentToken->type == TOKEN_COLON) {
    eat(parser, TOKEN_COLON); // :
    if (strcmp(parser->currentToken->val, "int") == 0)
      varDef->type = INT;
    else if (strcmp(parser->currentToken->val, "float") == 0)
      varDef->type = FLOAT;
    else if (strcmp(parser->currentToken->val, "char") == 0)
      varDef->type = CHAR;
    else if (strcmp(parser->currentToken->val, "bool") == 0)
      varDef->type = BOOL;
    else if (strcmp(parser->currentToken->val, "str") == 0)
      varDef->type = STRING;
    else if (strcmp(parser->currentToken->val, "any") == 0)
      varDef->type = ANY;
    else {
      printf("Unknown type `%s`\n", (char*) parser->currentToken->val);
      exit(1);
    }
    eat(parser, TOKEN_ID); // type
  }
  else 
    varDef->type = ANY; // Default type is ANY
  
  eat(parser, TOKEN_EQUALS); // =

  varDef->varDefVal = parseStatement(parser, scope, varDef->type); // value;  

  varDef->scope = scope; // Add it to the scope
  visitVarDef(varDef);

  return varDef;
}

AST_T* parseNewVarDef(parser_T* parser, scope_T* scope) {
  // Parse a new variable definition and create an AST node with the variable name and value as the value
  eat(parser, TOKEN_ID); // rnew
  AST_T* varDef = scopeGetVarDef(scope, parser->currentToken->val);

  if (!varDef) {
    printf("Undefined variable `%s`\n", (char*) parser->currentToken->val);
    exit(1);
  }

  eat(parser, TOKEN_ID); // variable name

  eat(parser, TOKEN_EQUALS); // =

  varDef->varDefVal = parseStatement(parser, scope, varDef->type); // value;  

  return varDef;
}

AST_T* parseVar(parser_T* parser, scope_T* scope) {
  // Parse a variable and create an AST node with the variable name as the value
  char* tokenVal = (char*) parser->currentToken->val;
  eat(parser, TOKEN_ID); // variable name

  // If a variable has a number attached to it (e.g. var1, var2, var3, etc.)
  if (parser->currentToken->type == TOKEN_INT) {
    char* buff = (char*) malloc(strlen(tokenVal) + 1);
    sprintf(buff, "%ld", (intptr_t) parser->currentToken->val);
    strcat(tokenVal, buff);
    eat(parser, TOKEN_INT);
  }
  
  // Parse function arguments
  if (parser->currentToken->type == TOKEN_LPAREN)
    return parseFuncCall(parser, scope);
  
  // Create the AST node with its values
  AST_T* var = initAST(AST_VARIABLE);
  var->varName = tokenVal;
  var->scope = scope; // Add it to the scope

  return var;
}

AST_T* parseString(parser_T* parser, scope_T* scope) {
  // Parse a string and create an AST node with the string as the value
  AST_T* string = initAST(STRING);
  string->stringVal = parser->currentToken->val;
  
  eat(parser, TOKEN_STRING);

  while (parser->currentToken->type == TOKEN_PLUS) {
    eat(parser, TOKEN_PLUS);
    switch (parser->currentToken->type) {
      case TOKEN_STRING:
        string->stringVal = strcat(string->stringVal, (char*) parser->currentToken->val);
        eat(parser, TOKEN_STRING);
        break;

      case TOKEN_ID:
        if (!scopeGetVarDef(scope, (char*) parser->currentToken->val)) {
          printf("Variable to concatenate does not exist.\n");
          exit(1);
        }

        string->stringVal = strcat(string->stringVal, (char*) scopeGetVarDef(scope, parser->currentToken->val)->varDefVal->stringVal);
        eat(parser, TOKEN_ID);
        break;
        
      default:
        printf("Concantinating non-string types with strings is currently unsupported.\n");
        exit(1);
    }
  }

  string->scope = scope;

  return string;
}

AST_T* parseBool(parser_T* parser, scope_T* scope) {
  // Parse a boolean and create an AST node with the boolean as the value
  AST_T* boolean = initAST(BOOL);

  if (strcmp(parser->currentToken->val, "false") == 0)
    boolean->boolVal = false;
  else if (strcmp(parser->currentToken->val, "true") == 0)
    boolean->boolVal = true;
  else {
    printf("Expected a boolean, but got `%s` with type %d\n", (char*) parser->currentToken->val, parser->currentToken->type);
    exit(1);
  }
  
  // Move past the boolean
  eat(parser, TOKEN_ID);

  boolean->scope = scope;

  return boolean;
}

AST_T* parseChar(parser_T* parser, scope_T* scope) {
  // Parse a character and create an AST node with the character as the value
  AST_T* character = initAST(CHAR);
  character->charVal = *(char*) parser->currentToken->val;
  
  // Move past the character
  eat(parser, TOKEN_CHAR);
  
  character->scope = scope;

  return character;
}


AST_T* parseIntExpr(parser_T* parser, scope_T* scope) {
  list_T* numList = initList();
  list_T* opList = initList();

  // Check if the first number is positive or negative and assign the value accordingly
  switch (parser->currentToken->type) {
    case TOKEN_MINUS:
      eat(parser, TOKEN_MINUS);
      push(&numList, -1 * (intptr_t) parser->currentToken->val);
      break;
    case TOKEN_PLUS:
      eat(parser, TOKEN_PLUS);
    default:
      push(&numList, (intptr_t) parser->currentToken->val);
      eat(parser, TOKEN_INT);
      break;
  }


  // Check if there are more operations to perform on the number
  bool keepRepeat = true;
  while (keepRepeat) {
    switch (parser->currentToken->type) {
      // If the token is a valid operation token, push it to the operation list and push the number to the number list
      case TOKEN_PLUS:
      case TOKEN_MINUS:
      case TOKEN_MULTIPLY:
      case TOKEN_DIVIDE:
      case TOKEN_POW:
      case TOKEN_MODULO:
        push(&opList, parser->currentToken->type);
        eat(parser, parser->currentToken->type);
        
        if (parser->currentToken->type == TOKEN_ID) {
          if (!scopeGetVarDef(scope, (char*) parser->currentToken->val)) {
            printf("Variable to perform operation on does not exist.\n");
            exit(1);
          }

          push(&numList, (intptr_t) scopeGetVarDef(scope, parser->currentToken->val)->varDefVal->intVal);
          eat(parser, TOKEN_ID);
        }
        else {
          push(&numList, (intptr_t) parser->currentToken->val);
          eat(parser, TOKEN_INT);
        }
        break;

      default:
        keepRepeat = false;
        break;
    }    
  }

  // Initialize the AST node with the integer value
  AST_T* num = initAST(INT);

  // Evaluate the expression and assign the value to the AST node
  num->intVal = eval(&opList, &numList);

  num->scope = scope;

  return num;
}

AST_T* parseID(parser_T* parser, scope_T* scope) {
  // Check the current identifier and parse accordingly
  if (strcmp(parser->currentToken->val, "let") == 0)
    return parseVarDef(parser, scope);
  
  if (strcmp(parser->currentToken->val, "func") == 0)
    return parseFuncDef(parser, scope);
  
  if (strcmp(parser->currentToken->val, "rnew") == 0)
    return parseNewVarDef(parser, scope);

  if (strcmp(parser->currentToken->val, "true") == 0 || strcmp(parser->currentToken->val, "false") == 0)
    return parseBool(parser, scope);
  
  return parseVar(parser, scope);
}