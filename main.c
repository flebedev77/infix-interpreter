#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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
    case TOKEN_ADD: return 2;
    case TOKEN_MUL: return 3;
    case TOKEN_DIV: return 4;
    case TOKEN_POW: return 5;
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
  }

  return TOKEN_NULL;
}

typedef struct {
  enum TokenType type;
  double value;
  char* str;
  int str_len; // Has to be printed with the len, because the string is not null terminated since it is just a pointer into the prompt string
  bool parsed;
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

static struct BinTreeNode expression_tree[PROMPT_SIZE] = {0};
static int expression_tree_root_node_index = 0;
static int tree_len = 0;
static Token_t tokens[PROMPT_SIZE] = {0};
static int tokens_len = 0;

void inset_into_expression_tree(Token_t* l, Token_t* r, Token_t* root) {
  root->parsed = true;
  l->parsed = true;
  r->parsed = true;
  expression_tree[tree_len++] = (struct BinTreeNode){
    .l = 0,
    .r = 0,
    .token = *root
  };
  expression_tree[tree_len++] = (struct BinTreeNode){
    .l = 0,
    .r = 0,
    .token = *l
  };
  expression_tree[tree_len++] = (struct BinTreeNode){
    .l = 0,
    .r = 0,
    .token = *r
  };
  expression_tree[tree_len-3].l = &expression_tree[tree_len-2];
  expression_tree[tree_len-3].r = &expression_tree[tree_len-1];
  // if (tree_len != 3) {
  //   expression_tree[tree_len-4].r = &expression_tree[tree_len-3];
  // }
}

void tokens_to_expression_tree() {
  memset(expression_tree, 0, sizeof(struct BinTreeNode) * PROMPT_SIZE);
  tree_len = 0;

  expression_tree_root_node_index = 0;

  for (int i = 0; i < tokens_len; i++) {
    bool prev_token_avaliable = (i-1 >= 0);
    bool next_token_avaliable = (i+1 < tokens_len);
    Token_t* prev_token = (prev_token_avaliable) ? &tokens[i-1] : (Token_t*) {0};
    Token_t* next_token = (next_token_avaliable) ? &tokens[i+1] : (Token_t*) {0};
    Token_t* token = &tokens[i];

    if (is_operator_token(token->type)) {
      if (!prev_token_avaliable || !next_token_avaliable || prev_token->type != TOKEN_NUM || next_token->type != TOKEN_NUM)
        SYNTAX_ERROR("Operator expected 2 literal numerical arguments");

      if (!prev_token->parsed) {
        inset_into_expression_tree(prev_token, next_token, token);
      } else {
        token->parsed = true;
        next_token->parsed = true;
        expression_tree[tree_len++] = (struct BinTreeNode){
          .l = 0,
          .r = 0,
          .token = *token
        };
        expression_tree[tree_len++] = (struct BinTreeNode){
          .l = 0,
          .r = 0,
          .token = *next_token
        };
        expression_tree[tree_len-2].l = &expression_tree[expression_tree_root_node_index];
        expression_tree[tree_len-2].r = &expression_tree[tree_len-1];
        expression_tree_root_node_index = tree_len-2;
      }

    }
  }
}

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
      tokens[tokens_len++] = (Token_t) {
        .type = current_token_type,
        .value = 0,
        .str = token_begin,
        .str_len = token_len,
        .parsed = false
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

void print_tree(struct BinTreeNode* root, int level) {
    if (root == NULL) return;


    print_tree(root->r, level + 1);
    
    for(int i = 0; i < level; i++) 
        printf("             ");  // Print spaces for indentation

    print_token(root->token);

    print_tree(root->l, level + 1);
}

void print_help() {
  printf("Arithmetic expression solver\nex\n");
  printf("2+3\n2*3\n2/3\n2-3\n2^3\n");
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

      switch (token.type) { // TODO: move this into the tree parser
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
        case TOKEN_NUM:
          break;
        case TOKEN_MUL:
          break;
        case TOKEN_ADD:
          break;
        case TOKEN_SUB:
          break;
        case TOKEN_DIV:
          break;
        case TOKEN_POW:
          break;
        case TOKEN_LPAREN:
          break;
        case TOKEN_RPAREN:
          break;
        default: break;
      }
    }

    tokens_to_expression_tree();
    printf("%d\n", expression_tree_root_node_index);
    print_tree(&expression_tree[expression_tree_root_node_index], 0);

  }

}
