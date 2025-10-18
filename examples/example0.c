#include <stdio.h>

#define LEX_SYMBOLS {">=", "<=", "==", "!=", "||", "&&", "!=", "+k"}
#define LEX_SPECIAL {'n', '\n', '\\', '\\', '"', '"'}

#include "lex.h"

int main() {
   lex_t lex = lex_new("2 + 3.14-10.9f \"he\\\\llo \\nworld\"NO! \"\\\" okay\" +k\n2");
   
   token_t token;
   while((token = lex_read_token(&lex)).type != TOKEN_NULL) {
      printf("[%d (line=%d, column=%d)]: ", token.type, token.line, token.column);
      if (token.type == TOKEN_NUMBER) {
         number_t num = token.number_v;
	 if (num.type == NUMBER_UINT)
            printf("%ld\n", num.uint_v);
         else if(num.type == NUMBER_DOUBLE)
	    printf("%lf\n", num.double_v);
	 else if(num.type == NUMBER_FLOAT)
	    printf("%ff\n", num.float_v);
      }
      else if (token.type == TOKEN_LITERAL)
         printf("%s\n", token.literal_v);
      else if (token.type == TOKEN_SYMBOL) {
         if(symbol_match(token.symbol_v, "+")) // this no work
	    printf("{+}\n");
	 else if(symbol_match(token.symbol_v, "+k"))
	    printf("roadblocks\n");
	 else 
	    printf("%c\n", token.symbol_v.c[0]);
      }
      else if (token.type == TOKEN_STRING)
	 printf("\"%s\"\n", token.string_v);
   }
   return 0;
}

