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

#define PI 3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628

double deg_to_rad(double x) { return x * PI/180; }
double rad_to_deg(double x) { return x * 180/PI; }
double cel_to_fah(double x) { return (x * 9/5) + 32; }
double fah_to_cel(double x) { return (x - 32) * 5/9; }

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
  TOKEN_POW = 6,
  TOKEN_REM = 7,
  TOKEN_COMMAND = 8,
  TOKEN_LPAREN = 9,
  TOKEN_RPAREN = 10
};

bool is_operator_token(enum TokenType type) {
  if (type >= 2 && type <= TOKEN_COMMAND) return true;
  return false;
}

int get_operator_token_precedence(enum TokenType type) {
  switch (type) {
    case TOKEN_SUB: return 1;
    case TOKEN_ADD: return 1;
    case TOKEN_MUL: return 2;
    case TOKEN_DIV: return 2;
    case TOKEN_POW: return 3;
    case TOKEN_COMMAND: return 4;
    case TOKEN_LPAREN: return 5;
    case TOKEN_RPAREN: return 5;
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
    case '%': return TOKEN_REM;
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

void print_token(Token_t token) {
  switch (token.type) {
    case TOKEN_NUM: printf("TOKEN_NUM "); break;
    case TOKEN_MUL: printf("TOKEN_MUL "); break;
    case TOKEN_ADD: printf("TOKEN_ADD "); break;
    case TOKEN_SUB: printf("TOKEN_SUB "); break;
    case TOKEN_DIV: printf("TOKEN_DIV "); break;
    case TOKEN_POW: printf("TOKEN_POW "); break;
    case TOKEN_REM: printf("TOKEN_REM "); break;
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

bool tokencmp(const char* str, Token_t token) {
  int str_len = strlen(str);
  if (str_len != token.str_len) return false;

  for (int i = 0; i < token.str_len; i++) {
    if (str[i] != token.str[i]) return false;
  }
  return true;
}

bool debug = false;
static Token_t tokens[PROMPT_SIZE] = {0};
static int tokens_len = 0;

void tokenise(char* str) {
  if (debug) printf("TOKENISER\n");

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
      // TODO: Implement my own atof that expects a string with a size instead of null terminated
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

      if (debug) print_token(tokens[tokens_len-1]);
    }

  }

  if (debug) printf("\n");
}


double evaluate_tokens() { // Shunting Yard Algorithm
  if (debug) printf("PARSER\n"); 

  Token_t* operator_stack[PROMPT_SIZE] = {0};
  int operator_stack_len = 0;
  Token_t* output_queue[PROMPT_SIZE] = {0};
  int output_queue_len = 0;

  for (int i = 0; i < tokens_len; i++) {
    Token_t* token = &tokens[i];

    if (token->type == TOKEN_NUM) {
      output_queue[output_queue_len++] = token;
    } else if (is_operator_token(token->type)) {
      if (operator_stack_len > 0) {
        Token_t* o2 = operator_stack[operator_stack_len-1];
        while (operator_stack_len > 0 && o2->type != TOKEN_LPAREN &&
            (o2->precedence > token->precedence ||
            (o2->precedence == token->precedence))
            ) {
          output_queue[output_queue_len++] = operator_stack[--operator_stack_len];
          o2 = operator_stack[operator_stack_len-1];
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
  if (operator_stack_len > 0 && operator_stack[operator_stack_len-1]->type == TOKEN_LPAREN) operator_stack_len--;

  if (debug) {
    for (int i = 0; i < output_queue_len; i++) {
      print_token(*output_queue[i]);
    }
  }

  double evaluation_stack[PROMPT_SIZE] = {0};
  int evaluation_stack_len = 0;

  for (int i = 0; i < output_queue_len; i++) {
    Token_t* token = output_queue[i];

    if (token->type == TOKEN_NUM) {
      evaluation_stack[evaluation_stack_len++] = token->value;
    } else if (is_operator_token(token->type)) {
      if (token->type == TOKEN_COMMAND) {
        bool has_arg = (evaluation_stack_len > 0); 
        double arg = has_arg ? evaluation_stack[--evaluation_stack_len] : -1.0;
        double return_val = 0.0;
        if (tokencmp("exit", *token)) {
          exit((has_arg) ? (int)arg : 0);
          return_val = -1;
        } else if (tokencmp("debug", *token)) {
          if (arg >= 1) debug = true;
          else debug = false;
          if (debug) printf("%0.2f %b\n", arg, debug);
          return_val = (double)debug;

        } else if (tokencmp("sin", *token)) { return_val = sin(deg_to_rad(arg));
        } else if (tokencmp("cos", *token)) { return_val = cos(deg_to_rad(arg));
        } else if (tokencmp("deg", *token)) { return_val = rad_to_deg(arg);
        } else if (tokencmp("rad", *token)) { return_val = deg_to_rad(arg);
        } else if (tokencmp("fah", *token)) { return_val = cel_to_fah(arg);
        } else if (tokencmp("cel", *token)) { return_val = fah_to_cel(arg);
        }

        evaluation_stack[evaluation_stack_len++] = return_val;
      } else {
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
          case TOKEN_REM: ans = (int)a % (int)b; break;
          default: SYNTAX_ERROR("Operator not implemented");
        }
        evaluation_stack[evaluation_stack_len++] = ans;
      }
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

    
    tokenise(prompt);

    // TODO: make evaluate_tokens return a string which can be printed directly to support hex
    printf("%0.2f\n", evaluate_tokens());
  }

}
