#include <string.h>
#include <stdio.h>
#include "include/parser.h"

parser_T* initParser(lexer_T* lexer) {
  parser_T* parser = calloc(1, sizeof(struct PARSER_STRUCT));
  parser->lexer = lexer;
  parser->currentToken = lexerGetNextToken(lexer);
  parser->prevToken = parser->currentToken;

  return parser;
}

void parserEat(parser_T* parser, int tokenType) {
  if (parser->currentToken->type == tokenType) {
    parser->prevToken = parser->currentToken;
    parser->currentToken = lexerGetNextToken(parser->lexer);
  }
  else {
    printf(
      "Unexpected token `%s` with type %d\n", parser->currentToken->val, 
      parser->currentToken->type
    );
    exit(1);
  }
}

AST_T* parserParse(parser_T* parser) { 
  return parserParseStatements(parser);
}

AST_T* parserParseStatement(parser_T* parser) {
  switch (parser->currentToken->type) {
    case TOKEN_ID: return parserParseID(parser); break;
  }

  return initAST(AST_NOOP);
}

AST_T* parserParseStatements(parser_T* parser) {
  AST_T* compound = initAST(AST_COMPOUND);
  compound->compoundVal = calloc(1, sizeof(struct AST_STRUCT));
  
  AST_T* statement = parserParseStatement(parser);
  compound->compoundVal[0] = statement;
  compound->compoundSize += 1;

  while(parser->currentToken->type == TOKEN_SEMI) {
    parserEat(parser, TOKEN_SEMI);

    AST_T* statement = parserParseStatement(parser);

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

AST_T* parserParseExpression(parser_T* parser) {
  switch (parser->currentToken->type) {
    case TOKEN_STRING: return parserParseString(parser); break;
    case TOKEN_ID: return parserParseID(parser); break;
  }

  return initAST(AST_NOOP);
}

AST_T* parserParseFactor(parser_T* parser) {

}

AST_T* parserParseTerm(parser_T* parser) {

}

AST_T* parserParseFuncCall(parser_T* parser) {
  AST_T* funcCall = initAST(AST_FUNCTION_CALL);
  funcCall->funcCallName = parser->prevToken->val;

  parserEat(parser, TOKEN_LPAREN);

  funcCall->funcCallArgs = calloc(1, sizeof(struct AST_STRUCT));
  
  AST_T* expression = parserParseExpression(parser);
  funcCall->funcCallArgs[0] = expression;
  funcCall->funcCallArgsSize += 1;

  while(parser->currentToken->type == TOKEN_COMMA) {
    parserEat(parser, TOKEN_COMMA);

    AST_T* expression = parserParseExpression(parser);
    funcCall->funcCallArgsSize += 1;
    
    funcCall->funcCallArgs = realloc(
      funcCall->funcCallArgs, 
      (funcCall->funcCallArgsSize + 1) * sizeof(struct AST_STRUCT)
    );

    funcCall->funcCallArgs[funcCall->funcCallArgsSize - 1] = expression;
  }

  parserEat(parser, TOKEN_RPAREN);

  return funcCall;
}

AST_T* parserParseVarDef(parser_T* parser) {
  parserEat(parser, TOKEN_ID); // let
  char* varDefVarName = parser->currentToken->val;
  parserEat(parser, TOKEN_ID); // variable name
  parserEat(parser, TOKEN_EQUALS); // =
  AST_T* varDefVal = parserParseExpression(parser); // value

  // Create the AST node with its values
  AST_T* varDef = initAST(AST_VARIABLE_DEFINITION);
  varDef->varDefVarName = varDefVarName;
  varDef->varDefVal = varDefVal;

  return varDef;
}

AST_T* parserParseVar(parser_T* parser) {
  char* tokenVal = parser->currentToken->val;
  parserEat(parser, TOKEN_ID); // variable name
  
  if (parser->currentToken->type == TOKEN_LPAREN) 
    return parserParseFuncCall(parser);
  
  // Create the AST node with its values
  AST_T* var = initAST(AST_VARIABLE);
  var->varName = tokenVal;

  return var;
}

AST_T* parserParseString(parser_T* parser) {
  AST_T* string = initAST(AST_STRING);
  string->stringVal = parser->currentToken->val;
  
  parserEat(parser, TOKEN_STRING);
  return string;
}

AST_T* parserParseID(parser_T* parser) {
  if (strcmp(parser->currentToken->val, "let") == 0) {
    return parserParseVarDef(parser);
  } else {
    return parserParseVar(parser);
  }
}