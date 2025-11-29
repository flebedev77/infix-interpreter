#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <unistd.h>
#include <ctype.h>
#include <termios.h>

// As found in the termios man page - (The read buffer will only accept 4095 chars)
// No reason to set these constants to anything else
#define PROMPT_SIZE 4095
#define OUTPUT_SIZE 4095
#define PROMPT_HISTORY_SIZE 20
#define PROMPT_STRING "> "

#define SYNTAX_ERROR(msg) do { \
  fprintf(stderr, "%s:%d SYNTAX ERROR! %s\n\r", __FILE__, __LINE__, msg); \
} while (0)
// exit(EXIT_FAILURE); \

#define PI 3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628
double deg_to_rad(double x) { return x * PI/180; }
double rad_to_deg(double x) { return x * 180/PI; }
double cel_to_fah(double x) { return (x * 9/5) + 32; }
double fah_to_cel(double x) { return (x - 32) * 5/9; }

int nibble_to_int(char nib) {
  switch (nib) {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'A': return 10;
    case 'a': return 10;
    case 'B': return 11;
    case 'b': return 11;
    case 'C': return 12;
    case 'c': return 12;
    case 'D': return 13;
    case 'd': return 13;
    case 'E': return 14;
    case 'e': return 14;
    case 'F': return 15;
    case 'f': return 15;
  }
  return 0;
}

int hex_to_int(char* hex, int len) {
  hex += 2;
  len -= 2;
  int acc = 0;
  for (int i = 0; i < len; i++) {
    char c = hex[i];
    acc *= 16;
    acc += nibble_to_int(c);
  }
  return acc;
}

int bin_to_int(char* bin, int len) {
  bin += 2;
  len -= 2;
  int acc = 0;
  for (int i = 0; i < len; i++) {
    char c = bin[i];
    acc *= 2;
    acc += (c == '0') ? 0 : 1;
  }
  return acc;
}

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
    case '<': return true;
    case '>': return true;
    case '=': return true;
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

enum OutputType {
  OUTPUT_DEC,
  OUTPUT_HEX,
  OUTPUT_BIN,
  OUTPUT_BASE64,
  OUTPUT_BASE32,
  OUTPUT_BASE16
};

enum TokenType {
  TOKEN_NULL = 0,
  TOKEN_NUM = 1,
  TOKEN_MUL = 2,
  TOKEN_ADD = 3,
  TOKEN_SUB = 4,
  TOKEN_DIV = 5,
  TOKEN_POW = 6,
  TOKEN_REM = 7,
  TOKEN_BSL = 8,
  TOKEN_BSR = 9,
  TOKEN_COMMAND = 10,
  TOKEN_LPAREN = 11,
  TOKEN_RPAREN = 12
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
    case TOKEN_BSR: return 4;
    case TOKEN_BSL: return 4;
    case TOKEN_COMMAND: return 5;
    case TOKEN_LPAREN: return 6;
    case TOKEN_RPAREN: return 6;
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
    case '<': return TOKEN_BSL;
    case '>': return TOKEN_BSR;
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
    case TOKEN_BSL: printf("TOKEN_BSL "); break;
    case TOKEN_BSR: printf("TOKEN_BSR "); break;
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

void print_help(bool advanced) {
  printf("Arithmetic expression solver\n");
  if (advanced) {
    printf("UNFINISHED\n");
  } else {
    printf("(2+3)*3/3-3^2\n");
    printf("There are also some basic functions avaliable\nex\n");
    printf("hex(2+3)\nbin(2*3)\ndec(0xFF)\n");
    printf("To exit C-c or type exit\n");
  }
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
  bool currently_hex = false,
       currently_binary = false;

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

      if ((current_token_type == TOKEN_BSL || current_token_type == TOKEN_BSR) &&
          get_char_token_type(nc) == current_token_type)
        eot = false; 
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
      if ((currently_hex || currently_binary) && (get_char_token_type(nc) == TOKEN_NUM || get_char_token_type(nc) == TOKEN_COMMAND)) eot = false;
      if (current_token_type == TOKEN_NUM && token_begin[0] == '0' && get_char_token_type(nc) == TOKEN_COMMAND) {
        if (nc == 'x') currently_hex = true;
        else if (nc == 'b') currently_binary = true;
        eot = false;
      }
    }

    if (eot && current_token_type != TOKEN_NULL) {
      // TODO: Implement my own atof that expects a string with a size instead of null terminated
      double value = 0.0; 
      if (current_token_type == TOKEN_NUM) {
        char buf[PROMPT_SIZE] = {0};
        memcpy(buf, token_begin, token_len);
        value = atof(buf);
      }

      if (currently_hex) {
        value = (double)hex_to_int(token_begin, token_len);
        current_token_type = TOKEN_NUM;
      } else if (currently_binary) {
        value = (double)bin_to_int(token_begin, token_len);
        current_token_type = TOKEN_NUM;
      }

      tokens[tokens_len++] = (Token_t) {
        .type = current_token_type,
          .value = value,
          .str = token_begin,
          .str_len = token_len,
          .precedence = get_operator_token_precedence(current_token_type)
      };
      current_token_type = TOKEN_NULL;
      currently_hex = false;
      currently_binary = false;
      token_begin += token_len;
      token_len = 0;

      Token_t* lt = &tokens[tokens_len-1];
      if (tokencmp("true", *lt)) {
        lt->type = TOKEN_NUM;
        lt->value = 1.0;
      } else if (tokencmp("false", *lt)) {
        lt->type = TOKEN_NUM;
        lt->value = 0.0;
      } else if (tokencmp("PI", *lt) || tokencmp("pi", *lt)) {
        lt->type = TOKEN_NUM;
        lt->value = PI;
      }

      if (debug) print_token(tokens[tokens_len-1]);
    }

  }

  if (debug) printf("\n");
}


void evaluate_tokens(char* output) { // Shunting Yard Algorithm
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
          if (operator_stack_len > 0) o2 = operator_stack[operator_stack_len-1];
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

  enum OutputType output_type = OUTPUT_DEC;
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
        } else if (tokencmp("help", *token)) { print_help((arg == 1));
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
        } else if (tokencmp("hex", *token)) { output_type = OUTPUT_HEX; return_val = arg;
        } else if (tokencmp("dec", *token)) { output_type = OUTPUT_DEC; return_val = arg;
        } else if (tokencmp("bin", *token)) { output_type = OUTPUT_BIN; return_val = arg;
        } else {
          SYNTAX_ERROR("Unknown function or command");
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
          case TOKEN_BSL: ans = (int)a << (int)b; break;
          case TOKEN_BSR: ans = (int)a >> (int)b; break;
          default: SYNTAX_ERROR("Operator not implemented");
        }
        evaluation_stack[evaluation_stack_len++] = ans;
      }
    } else {
      SYNTAX_ERROR("Unhandled token. How did this happen?");
    }
  }

  if (evaluation_stack_len != 1) SYNTAX_ERROR("Unfinished expression");

  switch (output_type) {
    case OUTPUT_DEC: snprintf(output, OUTPUT_SIZE, "%0.3f", evaluation_stack[0]); break;
    case OUTPUT_HEX: snprintf(output, OUTPUT_SIZE, "0x%X", (int)evaluation_stack[0]); break;
    case OUTPUT_BIN: snprintf(output, OUTPUT_SIZE, "0b%b", (int)evaluation_stack[0]); break;
    default: SYNTAX_ERROR("Unknown output type");
  }
}

struct termios original_spec = {0};

void close_terminal() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_spec);
}

void setup_terminal() {
  struct termios spec = {0};
  tcgetattr(STDIN_FILENO, &spec);
  original_spec = spec;
  // cfmakeraw(&spec);
  spec.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &spec);
  atexit(close_terminal);
}


void handle_keyboard(char* prompt, char* prompt_history, int current_history_index) {
  int initial_history_ind = current_history_index;
  char c;
  int i = 0;
  bool editing_middle = false;
  while (read(STDIN_FILENO, &c, 1) == 1) {
    if (c == '\n' || c == '\b') {
      write(STDOUT_FILENO, "\n", 1);
      break;
    }
    if (debug) printf("%d ", c);
    if (c == 127 && i > 0) { // Backspace
      // printf("\r\033[%dC", i + 2);
      prompt[--i] = 0;
      printf("\033[1D \033[1D");
      fflush(stdout);
    } else if (c == '\033') {
      char nc[2];
      read(STDIN_FILENO, nc, 2);
      if (nc[0] == '[') {
        switch (nc[1]) {
          case 'D': 
            if (i > 0) {
              i--;
              editing_middle = true;
              printf("\033[1D"); 
            }
            break; //left
          case 'C': 
            // TODO replace strlen with a variable
            int prompt_len = strlen(prompt);
            if (i < prompt_len) {
              i++;
              printf("\033[1C");
            } 
            if (i >= prompt_len) editing_middle = false;
            break; //right
          case 'A': 
            if (current_history_index > 0) {
              current_history_index--;
            }
            memcpy(prompt, &prompt_history[current_history_index * PROMPT_SIZE], PROMPT_SIZE);
            printf("\033[2K\r%s%s", PROMPT_STRING, prompt);
            break; //up
          case 'B':
            if (current_history_index < initial_history_ind-1) {
              current_history_index++;
              memcpy(prompt, &prompt_history[current_history_index * PROMPT_SIZE], PROMPT_SIZE);
            } else {
              memset(prompt, 0, PROMPT_SIZE);
            }
            printf("\033[2K\r%s%s", PROMPT_STRING, prompt);
            break; //down
        }
        fflush(stdout);
      }
    } else if (!iscntrl(c)) {
      prompt[i++] = c;
      putchar(c);
      fflush(stdout);
    } 

  }
}

int main() {
  char* prompt_storage = (char*)malloc(sizeof(char) * PROMPT_SIZE * PROMPT_HISTORY_SIZE);
  int prompt_storage_index = 0;

  if (prompt_storage == NULL) {
    printf("Error allocating memory for prompt history, it will be disabled\n");
  }

  setup_terminal();

  while (1) {
    printf("\r%s", PROMPT_STRING);
    fflush(stdout);
    char prompt[PROMPT_SIZE] = {0};
    char output[OUTPUT_SIZE] = {0};

    handle_keyboard(prompt, prompt_storage, prompt_storage_index);
    // TODO: avoid this copy by storing the actual prompt as a part in the history memory
    memcpy(&prompt_storage[prompt_storage_index * PROMPT_SIZE], prompt, PROMPT_SIZE);
    prompt_storage_index = (prompt_storage_index + 1) % PROMPT_HISTORY_SIZE;
    // fgets(prompt, PROMPT_SIZE, stdin);
    // prompt[strlen(prompt)-1] = 0;

    tokenise(prompt);
    evaluate_tokens(output);

    printf("%s\n", output);
    fflush(stdout);
  }
}
