#ifndef _NC_LEX
#define _NC_LEX

#include<stdlib.h>
#include<string.h>
#include<stdint.h>

#ifndef LEX_SINGLE_THREADED
#define LEX_SINGLE_THREADED 1
#endif

#ifndef LEX_SYMBOLS
#define LEX_SYMBOLS {"+", "-", "*", "/", "++"}
#endif

#ifndef LEX_SPECIAL
#define LEX_SPECIAL {'n', '\n'}
#endif

// TODO: add string builder for strings (not words) -> add escape sequences!


char char_is_letter(char c);
inline char char_is_letter(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

char char_is_digit(char c);
inline char char_is_digit(char c) {
    return c >= '0' && c <= '9';
}

char char_is_symbol(char c);
inline char char_is_symbol(char c) {
    return (c >= 33 && c <= 47) || (c >= 58 && c <= 64) || (c >= 123 && c <= 126) || (c >= 91 && c <= 96);
}

char char_is_space(char c);
inline char char_is_space(char c) {
   return c == ' ' || c == '\n';
}

//
//
#define _STRING_ICAPACITY 20

typedef struct {
   char* content;
   size_t capacity;
   size_t length;
} string_t;

string_t string_new();
inline string_t string_new() {
   char* w = malloc(_STRING_ICAPACITY + 1);
   w[0] = 0;

   return (string_t) { .length = 0, .capacity = _STRING_ICAPACITY, .content = w };
}

void string_append(string_t* sb, char c);
inline void string_append(string_t* sb, char c) {
   if (sb->length + 1 > sb->capacity) {
      sb->content = realloc(sb->content, sb->capacity *= 1.5);
   }

   sb->content[sb->length++] = c;
   sb->content[sb->length]   = 0;
}


// LEX CODE -noclue

typedef struct { 
    const char* input; 
    size_t pos;

    string_t sb;
} lex_t;

lex_t lex_new(const char* input);
inline lex_t lex_new(const char* input) {
   return (lex_t) { .sb = string_new(), .input = input, .pos = 0 };
}

char lex_cchar(lex_t* lex);
inline char lex_cchar(lex_t* lex) {
   return lex->input[lex->pos];
}

char lex_nchar(lex_t* lex);
inline char lex_nchar(lex_t* lex) {
   return lex->input[lex->pos++];
}

char lex_is_done(lex_t* lex);
inline char lex_is_done(lex_t* lex) {
   return lex_cchar(lex) == 0;
}

char lex_peek(lex_t* lex, int offset);
inline char lex_peek(lex_t* lex, int offset) {
   return lex->input[lex->pos + offset];
}

void lex_move(lex_t* lex, int shift);
inline void lex_move(lex_t* lex, int shift) {
   lex->pos += shift;
}

void lex_skip_spaces(lex_t* lex);
inline void lex_skip_spaces(lex_t* lex) {
   while(char_is_space(lex_cchar(lex))) { lex->pos++; }
}

char lex_check_s(lex_t* lex, const char* seq) {
   size_t x = 0;

   while(lex_peek(lex, x) != 0 && (lex_peek(lex, x) == seq[x])) x++;

   if (seq[x] == 0) {
      lex_move(lex, x);
      return 1;
   }

   return 0;
}

char* lex_read_string(lex_t* lex);
/*inline char* lex_read_string(lex_t* lex) {
   if (lex_cchar(lex) != '"') return NULL;
   lex->pos++;

   size_t x = 0;
   while(lex_cchar(lex) != '"') {
      if(lex_cchar(lex) == 0) {
         printf("Expected \"");
	 exit(1);
      }

      lex->pos++;
      x++;
   }

   if(x == 0) return NULL;

   char* w = malloc(x + 1);
   memcpy(w, lex->input + lex->pos - x, x);
   w[x] = 0;
   
   lex->pos++; // consume last "

   return w;
}*/
inline char* lex_read_string(lex_t* lex) {
   if(lex_cchar(lex) != '"') return NULL;
   lex->pos++;

   lex->sb.length = 0; // reset the string builder, no need to allocate more!
   
   char c;
   while((c = lex_cchar(lex)) != '"') {
      if (c == 0) {
         printf("Expected \"\n");
	 exit(1);
      }

      if(c == '\\') {
         // special characters!!
         char c = lex->input[++lex->pos];

	 const char  special_characters[] = LEX_SPECIAL;

	 for(uint32_t i = 0; i < sizeof(special_characters); i += 2) {
	     if(special_characters[i] == c) {
	         string_append(&lex->sb, special_characters[i+1]);
		 goto _OK_;
	     }
	 }

	 printf("Unspecified special character \\%c\n", c);
	 exit(1);
_OK_:;
      }
      else {
         string_append(&lex->sb, c);
      }

      lex->pos++;
   }
   lex->pos++;

   if(lex->sb.length == 0) return NULL;

   char* w = malloc(lex->sb.length + 1);
   memcpy(w, lex->sb.content, lex->sb.length);
   w[lex->sb.length] = 0;
  
   return w;
}

char* lex_read_word(lex_t* lex);
inline char* lex_read_word(lex_t* lex) {
   size_t x = 0;

   while(char_is_letter(lex_cchar(lex)) || lex_cchar(lex) == '_') { x++; lex->pos++; }

   if(0 == x) return NULL;

   char* w = malloc(x + 1);
   memcpy(w, lex->input + lex->pos - x, x);
   w[x] = 0;

   return w;
}

typedef enum {
   NUMBER_FLOAT,
   NUMBER_DOUBLE,
   NUMBER_UINT,
} number_type;

// used for tokenization, so since the - will be separated from the number anyways, might as well use uint
typedef struct {
   union {
       uint64_t uint_v;
       float   float_v;
       double double_v;
   };

   number_type type;
} number_t;

number_t lex_read_number(lex_t* lex);
inline number_t lex_read_number(lex_t* lex) {
   number_t num = { .type = NUMBER_UINT, .uint_v = 0 };
   
   size_t start_pos = lex->pos;
   while(char_is_digit(lex_cchar(lex))) {
      num.uint_v = num.uint_v * 10 + lex_cchar(lex) - '0';
      lex->pos++;
   }

   if (lex_cchar(lex) != '.')
      return num;

   lex->pos++;

   num.type = NUMBER_DOUBLE;
   while(char_is_digit(lex_cchar(lex))) {
      lex->pos++;
   }

   #if !LEX_SINGLE_THREADED 
	void* buffer = malloc(lex->pos - start_pos);
   #else
	static char buffer[32];
	if (lex->pos - start_pos > sizeof(buffer)) {
	    printf("Too many floating-point digits!");
	    exit(1);
	}
   #endif
   memcpy(buffer, lex->input + start_pos, lex->pos - start_pos);
   
   num.double_v = atof(buffer);
   #if !LEX_SINGLE_THREADED 
       free(buffer);
   #endif

   if(lex_cchar(lex) == 'f') {
       num.float_v = num.double_v;
       num.type = NUMBER_FLOAT;
       lex->pos++;
       return num;
   }
   return num;
}

uint64_t lex_read_int(lex_t* lex);
inline uint64_t lex_read_int(lex_t* lex) {
   uint64_t n = 0;

   while(char_is_digit(lex_cchar(lex))) {
      n = n * 10 + lex_cchar(lex) - '0';
      lex->pos++;
   }

   return n;
}

typedef struct { 
   const char* c;
   size_t len;
} symbol_t;

symbol_t lex_read_symbol(lex_t* lex);
inline symbol_t lex_read_symbol(lex_t* lex) {
   const char* _symbols[]  = LEX_SYMBOLS;
   const size_t _sym_count = sizeof(_symbols) / sizeof(char*);
   
   symbol_t symbol;
   for(size_t i = 0; i < _sym_count; i++) {
      size_t x = lex->pos;
      if(lex_check_s(lex, _symbols[i])) {
          symbol.c   = _symbols[i];
	  symbol.len = lex->pos - x;
          return symbol;
      }
   }
   
   symbol.c   = lex->input + lex->pos++;
   symbol.len = 1;
   return symbol;
}

char symbol_match(symbol_t symbol, const char* sym);
inline char symbol_match(symbol_t symbol, const char* sym) {
   size_t x = 0;
   while(x < symbol.len && symbol.c[x] == sym[x] && sym[x] != 0) x++;
   return x == symbol.len;
}  

typedef enum {
   TOKEN_NUMBER,
   TOKEN_STRING,
   TOKEN_SYMBOL,
   TOKEN_LITERAL,
   TOKEN_NULL
} token_type;

typedef struct {
   union {
       char*     literal_v;
       char*     string_v;
       symbol_t  symbol_v; 
       number_t  number_v;
   };

   token_type type;
} token_t;

token_t lex_read_token(lex_t* lex) {
   lex_skip_spaces(lex);

   token_t t = {0};

   // no more characters
   if(lex_is_done(lex)) {
      t.type = TOKEN_NULL;
      return t;
   }
   
   char c = lex_cchar(lex);
   if(char_is_letter(c)) {
      t.type = TOKEN_LITERAL;
      t.literal_v = lex_read_word(lex);
   }
   else if(char_is_digit(c)) {
      t.type = TOKEN_NUMBER;
      t.number_v = lex_read_number(lex);
   }
   else if(c == '"') {
      t.type = TOKEN_STRING;
      t.string_v = lex_read_string(lex);
   }
   else if(char_is_symbol(c)) {
      t.type = TOKEN_SYMBOL;
      t.symbol_v = lex_read_symbol(lex);
   }
   else {
      printf("Unexpected Token starting with %c!\n", c);
      exit(1);
   }

   return t;
}

#endif
