#ifndef TOKEN_H
#define TOKEN_H

typedef struct TOKEN_STRUCT {
  enum {
    TOKEN_ID,
    TOKEN_EQUALS,
		TOKEN_STRING,
		TOKEN_SEMI,
		TOKEN_LPAREN,
		TOKEN_RPAREN,
		TOKEN_COMMA,
		TOKEN_EOF
  } type;

	char* val;
} token_T;

token_T* initToken(int type, char* val);

#endif