#include <stdio.h>

// example use case of lex

// this is how you define custom symbols (more than a single character)
#define LEX_SYMBOLS {">=", "<=", "==", "!=", "||", "&&", "!=", "+k"}

// this is how you define special characters!
// they are defined in pairs: 'n', '\n' means that when the lexerer sees \n (inside double quotes of course) it inserts the newline character instead
// '\\', '\\' means that when the lexerer sees \\, it replaces it with \
// '"', '"'   means that when the lexerer sees \", it replaces it with "
// and so on!
#define LEX_SPECIAL {'n', '\n', '\\', '\\', '"', '"'}

#include "lex.h"

int main() {
   lex_t lex = lex_new("2 + 3.14-10.9f \"hello \\nworld\"NO! \"\\\" okay\" +k\n2");
   
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
         if(symbol_match(token.symbol_v, "+"))
	    printf("{+}\n");
	 else if(symbol_match(token.symbol_v, "+k")) // use this to check your symbols!
	    printf("roadblocks\n");
	 else 
	    printf("%c\n", token.symbol_v.c[0]);
      }
      else if (token.type == TOKEN_STRING)
	 printf("\"%s\"\n", token.string_v);
   }
   // IGNORE THE MEMORY LEAK, THIS IS JUST AN EXAMPLE
   // TOKEN_LITERAL and TOKEN_STRING allocate memory!
   return 0;
}

