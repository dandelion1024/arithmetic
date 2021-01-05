#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256

enum Type {
    INTEGER,
    BIN_OP,
    UNARY_OP,
    SEPARATOR
};

typedef struct {
    int       data;
    enum Type type;
} Token;

typedef struct {
    Token elements[N];
    int   top;
    int   capacity;
} Stack;

typedef struct {
    Token elements[N];
    int   front;
    int   rear;
    int   capacity;
} Queue;

Stack* alloc_stack()
{
    Stack* s    = (Stack*)malloc(sizeof(Stack));
    s->top      = -1;
    s->capacity = N;
    return s;
}

Queue* alloc_queue()
{
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = 0;
    q->capacity        = N;
    return q;
}

bool sempty(Stack* s) { return s->top == -1; }

int ssize(Stack* s) { return s->top + 1; }

void push(Stack* s, Token t) { s->elements[++s->top] = t; }

Token pop(Stack* s) { return s->elements[s->top--]; }

Token top(Stack* s) { return s->elements[s->top]; }

bool qempty(Queue* q) { return q->front == q->rear; }

int qlength(Queue* q) { return (q->rear - q->front + q->capacity) % q->capacity; }

void enqueue(Queue* q, Token t)
{
    q->elements[q->rear] = t;
    q->rear              = (q->rear + 1) % q->capacity;
}

Token dequeue(Queue* q)
{
    Token t  = q->elements[q->front];
    q->front = (q->front + 1) % q->capacity;
    return t;
}

bool is_operator(char ch)
{
    switch (ch) {
    case '+':
    case '-':
    case '*':
    case '/':
        return true;
    default:
        return false;
    }
}

int priority(Token token)
{
    if (token.type == INTEGER)
        return -1;
    if (token.data == ')')
        return 4;
    if (token.type == UNARY_OP)
        return 3;
    if (token.data == '*' || token.data == '/')
        return 2;
    if (token.data == '+' || token.data == '-')
        return 1;
    if (token.data == '(')
        return 0;
    return -1;
}

int get_input(Token inorder[])
{
    int  ch, i = 0, number = 0;
    bool success = true;

    ch = getchar();

    while (ch != '\n' && ch != EOF) {
        if (i >= N) {
            fprintf(stderr, "expression is too long (>%d)\n", N);
            exit(-1);
        }

        if (isspace(ch)) {
            ch = getchar();
            continue;
        }

        number = 0;

        if (isdigit(ch)) {
            number += (ch - '0');
            while (isdigit(ch = getchar()))
                number = number * 10 + (ch - '0');
            inorder[i].data = number;
            inorder[i].type = INTEGER;
            ++i;
        } else if (ch == '(' || ch == ')') {
            inorder[i].data = ch;
            inorder[i].type = SEPARATOR;
            ++i;
            ch = getchar();
        } else if (is_operator(ch)) {
            inorder[i].data = ch;
            inorder[i].type = (inorder[i - 1].type == BIN_OP) ? UNARY_OP : BIN_OP;
            ++i;
            ch = getchar();
        } else {
            fprintf(stderr, "Invalid input '%c'(ascii: %d)\n", ch, ch);
            success = false;
        }
    }

    return success ? i : -1;
}

Queue* parse_postorder(Token inorder[], int n)
{
    Stack* s = alloc_stack();
    Queue* q = alloc_queue();

    for (int i = 0; i < n; ++i) {
        if (inorder[i].type == INTEGER)
            enqueue(q, inorder[i]);
        else if (inorder[i].type == BIN_OP || inorder[i].type == UNARY_OP) {
            while (!sempty(s) && top(s).type != SEPARATOR
                   && priority(inorder[i]) <= priority(top(s)))
                enqueue(q, pop(s));
            push(s, inorder[i]);
        } else if (inorder[i].data == '(')
            push(s, inorder[i]);
        else if (inorder[i].data == ')') {
            while (top(s).data != '(') enqueue(q, pop(s));
            pop(s);
        } else {
            free(s);
            free(q);
            return NULL;
        }
    }

    while (!sempty(s)) enqueue(q, pop(s));

    free(s);

    return q;
}

int calc(Token op, int x1, int x2)
{
    if (op.type == UNARY_OP) return op.data == '-' ? -x1 : x1;

    switch (op.data) {
    case '+': return x1 + x2;
    case '-': return x1 - x2;
    case '*': return x1 * x2;
    case '/': return x1 / x2;
    default:
        fprintf(stderr, "error, invalid operation %d %d(%c) %d!\n", x1, op.data, op.data, x2);
        exit(-1);
    }
}

int get_result(Queue* postorder_q)
{
    Stack* s      = alloc_stack();
    int    result = 0;
    Token  x1, x2, op;
    Token  tmp_token;
    bool   success = true;

    while (!qempty(postorder_q)) {
        push(s, dequeue(postorder_q));

        if (top(s).type == BIN_OP) {
            op = pop(s);
            if (ssize(s) < 2) {
                success = false;
                break;
            }

            x2     = pop(s);
            x1     = pop(s);
            result = calc(op, x1.data, x2.data);

            tmp_token.data = result;
            tmp_token.type = INTEGER;
            push(s, tmp_token);
        } else if (top(s).type == UNARY_OP) {
            op = pop(s);

            if (ssize(s) < 1) {
                success = false;
                break;
            }

            x1     = pop(s);
            result = calc(op, x1.data, 0);

            tmp_token.data = result;
            tmp_token.type = INTEGER;
            push(s, tmp_token);
        }
    }

    free(s);

    if (!success) {
        perror("syntax error\n");
        free(postorder_q);
        exit(-1);
    }

    return result;
}

void output_tokens(Token tokens[], int n)
{
    for (int i = 0; i < n; ++i) {
        if (tokens[i].type == INTEGER)
            printf("%d ", tokens[i].data);
        else
            printf("%c ", tokens[i].data);
    }
    printf("\n");
}

int main()
{
    Token  inorder[N];
    int    n = get_input(inorder);
    Queue* q = parse_postorder(inorder, n);

    if (q == NULL) {
        perror("syntax error.\n");
        exit(0);
    }

    output_tokens(q->elements, qlength(q));
    printf("%d\n", get_result(q));

    return 0;
}
