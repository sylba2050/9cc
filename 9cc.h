#include <stdio.h>

enum {
  TK_NUM = 256,
  TK_EOF,
};

enum {
    ND_NUM = 256,
};

typedef struct {
    int ty;
    int val;
    char *input;
} Token;

typedef struct Node{
    int ty;
    struct Node *lhs;
    struct Node *rhs;
    int val;
} Node;

extern int pos;
extern Token tokens[100];

extern Node *new_node(int, Node *, Node *);
extern Node *new_node_num(int);
extern Node *term();
extern Node *mul();
extern Node *expr();
extern void tokenize(char *);
extern void gen(Node *);
extern int consume(int);
extern void error(char *, ...);
extern void error_at(char *, char *);

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char* msg) {
    fprintf(stderr, "%s\n", loc);
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ %s\n", msg);
    exit(1);
}

int consume(int ty) {
  if (tokens[pos].ty != ty)
    return 0;
  pos++;
  return 1;
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_NUM;
    node->val = val;
    return node;
}

Node *mul() {
  Node *node = term();

  for (;;) {
    if (consume('*'))
      node = new_node('*', node, term());
    else if (consume('/'))
      node = new_node('/', node, term());
    else
      return node;
  }
}

Node *expr() {
    Node *node = mul();

    for (;;) {
        if (consume('+'))
            node = new_node('+', node, mul());
        else if (consume('-'))
            node = new_node('-', node, mul());
        else
            return node;
    }
}

Node *term() {
    if (consume('(')) {
        Node *node = expr();
        if (!consume(')'))
            error_at(tokens[pos].input, "開きカッコに対応する閉じカッコがありません");

        return node;
    }

    if (tokens[pos].ty == TK_NUM)
        return new_node_num(tokens[pos++].val);

    error_at(tokens[pos].input, "数値でも開きカッコでもないトークンです");
}

void tokenize(char *p) {
    int i = 0;
    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
            tokens[i].ty = *p;
            tokens[i].input = p;
            i++;
            p++;
            continue;
        }

        if (isdigit(*p)) {
            tokens[i].ty = TK_NUM;
            tokens[i].input = p;
            tokens[i].val = strtol(p, &p, 10);
            i++;
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    tokens[i].ty = TK_EOF;
    tokens[i].input = p;
}

void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->ty) {
  case '+':
    printf("  add rax, rdi\n");
    break;
  case '-':
    printf("  sub rax, rdi\n");
    break;
  case '*':
    printf("  imul rdi\n");
    break;
  case '/':
    printf("  cqo\n");
    printf("  idiv rdi\n");
  }

  printf("  push rax\n");
}

