/*
MIT License

Copyright (c) 2025 NoClueBruh/Xaroulios

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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

static inline char char_is_letter(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static inline char char_is_digit(char c) {
    return c >= '0' && c <= '9';
}

static inline char char_is_symbol(char c) {
    return (c >= 33 && c <= 47) || (c >= 58 && c <= 64) || (c >= 123 && c <= 126) || (c >= 91 && c <= 96);
}

//
//
#define _STRING_ICAPACITY 20

typedef struct {
   char* content;
   uint32_t capacity;
   uint32_t length;
} string_t;

static inline string_t string_new() {
   char* w = malloc(_STRING_ICAPACITY + 1);
   w[0] = 0;

   return (string_t) { .length = 0, .capacity = _STRING_ICAPACITY, .content = w };
}

static inline void string_append(string_t* sb, char c) {
   if (sb->length + 1 > sb->capacity) {
      sb->content = realloc(sb->content, sb->capacity *= 1.5);
   }

   sb->content[sb->length++] = c;
   sb->content[sb->length]   = 0;
}


// LEX CODE -noclue

typedef struct { 
    const char* input; 
    uint32_t pos;

    uint32_t line_change_pos;
    uint32_t line;

    string_t sb;
} lex_t;

static inline lex_t lex_new(const char* input) {
   return (lex_t) { .sb = string_new(), .line = 1, .line_change_pos = 0, .input = input, .pos = 0 };
}

static inline void lex_free(lex_t* lex) {
   free(lex->sb.content);
}

static inline char lex_cchar(lex_t* lex) {
   return lex->input[lex->pos];
}

static inline char lex_nchar(lex_t* lex) {
   return lex->input[lex->pos++];
}

static inline char lex_is_done(lex_t* lex) {
   return lex_cchar(lex) == 0;
}

static inline char lex_peek(lex_t* lex, int offset) {
   return lex->input[lex->pos + offset];
}

static inline void lex_move(lex_t* lex, int shift) {
   lex->pos += shift;
}

static inline void lex_skip_spaces(lex_t* lex) {
    while(1) {
       char c = lex_cchar(lex);
       
       if(c == '\n') {
          lex->line_change_pos = lex->pos;
	  lex->line++;
       }
       else if(c == 0 || c > ' ')
	  break;

       lex->pos++;
   }
}

static inline char lex_check_s(lex_t* lex, const char* seq) {
   size_t x = 0;

   while(lex_peek(lex, x) != 0 && (lex_peek(lex, x) == seq[x])) x++;

   if (seq[x] == 0) {
      lex_move(lex, x);
      return 1;
   }

   return 0;
}

static char* lex_read_string(lex_t* lex) {
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

	 printf("Unspecified special character \\%c (line=%d, column=%d)\n", c, lex->line, lex->pos - lex->line_change_pos);
	 exit(1);
_OK_:;
      }
      else if (c >= 32) {
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

static inline char* lex_read_word(lex_t* lex) {
   uint32_t x = 0;

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

static number_t lex_read_number(lex_t* lex) {
   number_t num = { .type = NUMBER_UINT, .uint_v = 0 };
   
   uint32_t start_pos = lex->pos;
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
	    printf("Too many floating-point digits! (line=%d)\n", lex->line);
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

static inline uint64_t lex_read_int(lex_t* lex) {
   uint64_t n = 0;

   while(char_is_digit(lex_cchar(lex))) {
      n = n * 10 + lex_cchar(lex) - '0';
      lex->pos++;
   }

   return n;
}

typedef struct { 
   const char* c;
   uint32_t len;
} symbol_t;

static inline symbol_t lex_read_symbol(lex_t* lex) {
   const char* _symbols[]  = LEX_SYMBOLS;
   const uint16_t _sym_count = sizeof(_symbols) / sizeof(char*);
   
   symbol_t symbol;
   for(uint16_t i = 0; i < _sym_count; i++) {
      uint32_t x = lex->pos;
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

static inline char symbol_match(symbol_t symbol, const char* sym) {
   uint32_t x = 0;
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

   uint32_t line;
   uint32_t column;

   token_type type;
} token_t;

static token_t lex_read_token(lex_t* lex) {
   lex_skip_spaces(lex);

   token_t t = {0};
    
   uint32_t token_start = lex->pos;

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
      printf("Unexpected Token starting with %c (line=%d)!\n", c, lex->line);
      exit(1);
   }

   t.line   = lex->line;
   t.column = token_start - lex->line_change_pos;

   return t;
}

#endif
