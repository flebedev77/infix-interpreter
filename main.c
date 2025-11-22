#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PROMPT_SIZE 150

#define SYNTAX_ERROR(msg) do { \
  fprintf(stderr, "%s:%d SYNTAX ERROR! %s\n", __FILE__, __LINE__, msg); \
  exit(EXIT_FAILURE); \
} while (0)

bool is_number(char c) {
  switch (c) {
    case '0': return true;
    case '.': return true;
    case '1': return true;
    case '2': return true;
    case '3': return true;
    case '4': return true;
    case '5': return true;
    case '6': return true;
    case '7': return true;
    case '8': return true;
    case '9': return true;
    default: return false;
  }
  return false;
}
bool is_operator(char c) {
  switch (c) {
    case '+': return true;
    case '-': return true;
    case '*': return true;
    case '/': return true;
    case '^': return true;
    case '%': return true;
    case '!': return true;
    default: return false;
  }
  return false;
}
bool is_alphabetic(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || c == '#';
}
bool is_bracket(char c) {
  switch (c) {
    case '{': return true;
    case '}': return true;
    case '(': return true;
    case ')': return true;
    case '[': return true;
    case ']': return true;
    default: return false;
  }
  return false;
}


enum TokenType {
  TOKEN_NULL = 0,
  TOKEN_NUM = 1,
  TOKEN_MUL = 2,
  TOKEN_ADD = 3,
  TOKEN_SUB = 4,
  TOKEN_DIV = 5,
  TOKEN_POW = 6, // TODO: Add remainder operator
  TOKEN_LPAREN = 7,
  TOKEN_RPAREN = 8,
  TOKEN_COMMAND = 9
};

bool is_operator_token(enum TokenType type) {
  if (type >= 2 && type <= 6) return true;
  return false;
}

int get_operator_token_precedence(enum TokenType type) {
  switch (type) {
    case TOKEN_SUB: return 1;
    case TOKEN_ADD: return 1;
    case TOKEN_MUL: return 2;
    case TOKEN_DIV: return 2;
    case TOKEN_POW: return 3;
    case TOKEN_LPAREN: return 4;
    case TOKEN_RPAREN: return 4;
    default: return -1;
  }
}

enum TokenType get_char_token_type(char c) {
  if (is_number(c)) return TOKEN_NUM;
  if (is_alphabetic(c)) return TOKEN_COMMAND;

  switch (c) {
    case '(': return TOKEN_LPAREN;
    case ')': return TOKEN_RPAREN;
    case '+': return TOKEN_ADD;
    case '-': return TOKEN_SUB;
    case '/': return TOKEN_DIV;
    case '*': return TOKEN_MUL;
    case '^': return TOKEN_POW;
  }

  return TOKEN_NULL;
}

typedef struct {
  enum TokenType type;
  double value;
  char* str;
  int str_len; // Has to be printed with the len, because the string is not null terminated since it is just a pointer into the prompt string
  int precedence;
} Token_t;

struct BinTreeNode {
  struct BinTreeNode *l, *r;
  Token_t token;
};

bool tokencmp(const char* str, Token_t token) {
  int str_len = strlen(str);
  if (str_len != token.str_len) return false;

  for (int i = 0; i < token.str_len; i++) {
    if (str[i] != token.str[i]) return false;
  }
  return true;
}

static Token_t tokens[PROMPT_SIZE] = {0};
static int tokens_len = 0;

void tokenise(char* str) {
  memset(tokens, 0, sizeof(Token_t) * PROMPT_SIZE);
  tokens_len = 0;

  int str_len = strlen(str);
  char* token_begin = str;
  int token_len = 0;

  enum TokenType current_token_type = TOKEN_NULL;

  for (int i = 0; i < str_len; i++) {
    char c = str[i];
    char nc = (i+1 < str_len) ? str[i+1] : 0;
    bool eot = (nc == ' ' || nc == 0);

    if (c == ' ') {
      token_begin++;
    }

    if (is_operator(c) || is_bracket(c)) {
      current_token_type = get_char_token_type(c);
      eot = true;
      token_len++;
    }

    if (is_alphabetic(c)) {
      current_token_type = TOKEN_COMMAND;
      token_len++;
    }

    if (is_number(c)) {
      current_token_type = TOKEN_NUM;
      token_len++;
    }

    if (current_token_type != get_char_token_type(nc)) {
      eot = true;
    }

    if (eot && current_token_type != TOKEN_NULL) {
      // TODO Implement my own atof that expects a string with a size instead of null terminated
      double value = 0.0; 
      if (current_token_type == TOKEN_NUM) {
        char buf[PROMPT_SIZE] = {0};
        memcpy(buf, token_begin, token_len);
        value = atof(buf);
      }
      tokens[tokens_len++] = (Token_t) {
        .type = current_token_type,
          .value = value,
          .str = token_begin,
          .str_len = token_len,
          .precedence = get_operator_token_precedence(current_token_type)
      };
      current_token_type = TOKEN_NULL;
      token_begin += token_len;
      token_len = 0;
    }

  }
}

void print_token(Token_t token) {
  switch (token.type) {
    case TOKEN_NUM: printf("TOKEN_NUM "); break;
    case TOKEN_MUL: printf("TOKEN_MUL "); break;
    case TOKEN_ADD: printf("TOKEN_ADD "); break;
    case TOKEN_SUB: printf("TOKEN_SUB "); break;
    case TOKEN_DIV: printf("TOKEN_DIV "); break;
    case TOKEN_POW: printf("TOKEN_POW "); break;
    case TOKEN_LPAREN: printf("TOKEN_LPAREN "); break;
    case TOKEN_RPAREN: printf("TOKEN_RPAREN "); break;
    case TOKEN_COMMAND: printf("TOKEN_COMMAND "); break;
    case TOKEN_NULL: printf("TOKEN_NULL "); break;
    default: printf("TOKEN_UNKNOWN "); break;
  }
  for (int j = 0; j < token.str_len; j++) {
    putchar(token.str[j]);
  }
  putchar('\n');
}

double evaluate_tokens(bool debug) { // Shunting Yard Algorithm
  Token_t* operator_stack[PROMPT_SIZE] = {0};
  int operator_stack_len = 0;
  Token_t* output_queue[PROMPT_SIZE] = {0};
  int output_queue_len = 0;

  for (int i = 0; i < tokens_len; i++) {
    Token_t* token = &tokens[i];

    if (token->type == TOKEN_NUM) {
      output_queue[output_queue_len++] = token;
    } else if (token->type == TOKEN_COMMAND) {
      operator_stack[operator_stack_len++] = token;
    } else if (is_operator_token(token->type)) {
      if (operator_stack_len > 0) {
        Token_t* o2 = operator_stack[operator_stack_len-1];
        while (operator_stack_len > 0 && o2->type != TOKEN_LPAREN &&
            (o2->precedence > token->precedence ||
             o2->precedence == token->precedence)
            ) {
          output_queue[output_queue_len++] = operator_stack[--operator_stack_len];
        }
      }
      operator_stack[operator_stack_len++] = token;
    } else if (token->type == TOKEN_LPAREN) {
      operator_stack[operator_stack_len++] = token;
    } else if (token->type == TOKEN_RPAREN) {
      while (operator_stack_len > 0 && operator_stack[operator_stack_len-1]->type != TOKEN_LPAREN) {
        output_queue[output_queue_len++] = operator_stack[--operator_stack_len];
      }
      if (operator_stack_len > 0 && operator_stack[operator_stack_len-1]->type == TOKEN_LPAREN) operator_stack_len--;
    }
  }
  while (operator_stack_len > 0 && operator_stack[operator_stack_len-1]->type != TOKEN_LPAREN) {
    output_queue[output_queue_len++] = operator_stack[--operator_stack_len];
  }

  double evaluation_stack[PROMPT_SIZE] = {0};
  int evaluation_stack_len = 0;

  for (int i = 0; i < output_queue_len; i++) {
    Token_t* token = output_queue[i];
    if (debug) print_token(*token);

    if (token->type == TOKEN_NUM) {
      evaluation_stack[evaluation_stack_len++] = token->value;
    } else if (is_operator_token(token->type)) {
      if (evaluation_stack_len < 2) SYNTAX_ERROR("Infix expression expected left and right number literal");
      double b = evaluation_stack[--evaluation_stack_len];
      double a = evaluation_stack[--evaluation_stack_len];
      double ans = 0.0;

      if (debug) printf("%0.2f %0.2f\n", a, b);

      switch (token->type) {
        case TOKEN_MUL: ans = a * b; break;
        case TOKEN_DIV: ans = a / b; break;
        case TOKEN_ADD: ans = a + b; break;
        case TOKEN_SUB: ans = a - b; break;
        case TOKEN_POW: ans = pow(a, b); break;
        default: SYNTAX_ERROR("Operator not implemented");
      }
      evaluation_stack[evaluation_stack_len++] = ans;
    } else {
      SYNTAX_ERROR("Unhandled token. How did this happen?");
    }
  }

  if (evaluation_stack_len != 1) SYNTAX_ERROR("Unfinished expression");

  return evaluation_stack[0];
}

void print_help() {
  printf("Arithmetic expression solver\nex\n");
  printf("(2+3)*3/3-3^2\n");
  printf("There are also some basic functions avaliable\nex\n");
  printf("hex(2+3)\nbin(2*3)\ndec(0xFF)\n");
  printf("To exit C-c or type exit\n");
}

int main() {
  while (1) {
    printf("> ");
    char prompt[PROMPT_SIZE] = {0};
    fgets(prompt, PROMPT_SIZE, stdin);
    prompt[strlen(prompt)-1] = 0;

    static bool debug = false;
    
    tokenise(prompt);

    for (int i = 0; i < tokens_len; i++) {
      Token_t token = tokens[i];
      bool next_token_avaliable = (i+1 < tokens_len);
      Token_t next_token = (next_token_avaliable) ? tokens[i+1] : (Token_t){0};

      if (debug) {
        print_token(token);
      }

      switch (token.type) { // TODO: move this into the parser
        case TOKEN_COMMAND:
          if (tokencmp("help", token)) {
            print_help();
          } else if (tokencmp("debug", token)) {
            if (tokencmp("on", next_token) && next_token_avaliable) debug = true;
            else if (tokencmp("off", next_token) && next_token_avaliable) debug = false; 
            else debug = !debug;
          } else if (tokencmp("exit", token)) {
            exit(0);
          }
          break;
        default: break;
      }
    }

    printf("%0.2f\n", evaluate_tokens(debug));
  }

}
