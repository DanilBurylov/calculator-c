#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>


#define MAX_EXPRESSION_LENGTH 256

// ���� ⮪���� (����⮢ ��ࠦ����)
typedef enum {
    NUMBER,      // ��᫮
    OPERATOR,    // ������ (+, -, *, /, ^)
    FUNCTION,    // �㭪�� (sin, cos, tg, ln, sqrt)
    LEFT_PAREN,  // ����� ᪮���
    RIGHT_PAREN  // �ࠢ�� ᪮���
} TokenType;

// ������� ��� �࠭���� ⮪���
typedef struct {
    TokenType type;           // ��� ⮪���
    char value[32];           // ���祭�� ⮪��� (��ப�)
    int precedence;           // �ਮ��� ������
    bool rightAssociative;    // �ࠢ�� ���樠⨢����� (��� �⥯��� ^)
} Token;

// ������� �⥪� ��� �����⬠ ��⨭��
typedef struct {
    Token data[MAX_EXPRESSION_LENGTH];  // ����� �⥪�
    int top;                            // ������ ���設� �⥪�
} Stack;

// ������� ��।� ��� �뢮��
typedef struct {
    Token data[MAX_EXPRESSION_LENGTH];  // ����� ��।�
    int front, rear;                   // ������� ��砫� � ���� ��।�
} Queue;

// ���樠������ �⥪�
void initStack(Stack* s) {
    s->top = -1;
}

// �஢�ઠ �� ������ �⥪�
bool isStackEmpty(Stack* s) {
    return s->top == -1;
}

// ���������� ����� � �⥪
void push(Stack* s, Token token) {
    if (s->top >= MAX_EXPRESSION_LENGTH - 1) {
        fprintf(stderr, "Stack overflow\n");
        exit(EXIT_FAILURE);
    }
    s->data[++(s->top)] = token;
}

// �����祭�� ����� �� �⥪�
Token pop(Stack* s) {
    if (isStackEmpty(s)) {
        fprintf(stderr, "Stack underflow\n");
        exit(EXIT_FAILURE);
    }
    return s->data[(s->top)--];
}

// ��ᬮ�� ���孥�� ����� �⥪�
Token peek(Stack* s) {
    if (isStackEmpty(s)) {
        fprintf(stderr, "Stack is empty\n");
        exit(EXIT_FAILURE);
    }
    return s->data[s->top];
}

// ���樠������ ��।�
void initQueue(Queue* q) {
    q->front = q->rear = -1;
}

// ���������� ����� � ��।�
void enqueue(Queue* q, Token token) {
    if (q->rear >= MAX_EXPRESSION_LENGTH - 1) {
        fprintf(stderr, "Queue overflow\n");
        exit(EXIT_FAILURE);
    }
    if (q->front == -1) q->front = 0;
    q->data[++(q->rear)] = token;
}

// �஢�ઠ �� ������ ��।�
bool isQueueEmpty(Queue* q) {
    return q->front == -1 || q->front > q->rear;
}

// �����祭�� ����� �� ��।�
Token dequeue(Queue* q) {
    if (isQueueEmpty(q)) {
        fprintf(stderr, "Queue underflow\n");
        exit(EXIT_FAILURE);
    }
    Token token = q->data[q->front];
    if (q->front == q->rear) {
        q->front = q->rear = -1;
    }
    else {
        q->front++;
    }
    return token;
}

// �஢�ઠ, ���� �� ��ப� �㭪樥�
bool isFunction(const char* s) {
    return strcmp(s, "sin") == 0 || strcmp(s, "cos") == 0 ||
        strcmp(s, "tg") == 0 || strcmp(s, "ln") == 0 ||
        strcmp(s, "sqrt") == 0;
}

// �஢�ઠ, ���� �� ᨬ��� �����஬
bool isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
}

// ����祭�� �ਮ��� ������
int getPrecedence(char op) {
    switch (op) {
    case '+': case '-': return 2;
    case '*': case '/': return 3;
    case '^': return 4;
    default: return 0;
    }
}

// ������ �室���� ��ࠦ���� �� ⮪���
int tokenize(const char* expr, Token* tokens) {
    int i = 0, tokenCount = 0;
    while (expr[i] != '\0') {
        // �ய�᪠�� �஡���
        if (isspace(expr[i])) {
            i++;
            continue;
        }

        // ��ࠡ�⪠ �ᥫ
        if (isdigit(expr[i]) || (expr[i] == '.' && isdigit(expr[i + 1]))) {
            int j = 0;
            tokens[tokenCount].type = NUMBER;
            while (isdigit(expr[i]) || expr[i] == '.') {
                tokens[tokenCount].value[j++] = expr[i++];
            }
            tokens[tokenCount].value[j] = '\0';
            tokenCount++;
            continue;
        }

        // ��ࠡ�⪠ �㭪権
        if (isalpha(expr[i])) {
            int j = 0;
            tokens[tokenCount].type = FUNCTION;
            while (isalpha(expr[i])) {
                tokens[tokenCount].value[j++] = expr[i++];
            }
            tokens[tokenCount].value[j] = '\0';
            if (!isFunction(tokens[tokenCount].value)) {
                fprintf(stderr, "Unknown function: %s\n", tokens[tokenCount].value);
                exit(EXIT_FAILURE);
            }
            tokenCount++;
            continue;
        }

        // ��ࠡ�⪠ �����஢
        if (isOperator(expr[i])) {
            tokens[tokenCount].type = OPERATOR;
            tokens[tokenCount].value[0] = expr[i];
            tokens[tokenCount].value[1] = '\0';
            tokens[tokenCount].precedence = getPrecedence(expr[i]);
            tokens[tokenCount].rightAssociative = (expr[i] == '^');
            tokenCount++;
            i++;
            continue;
        }

        // ��ࠡ�⪠ ᪮���
        if (expr[i] == '(') {
            tokens[tokenCount].type = LEFT_PAREN;
            tokens[tokenCount].value[0] = expr[i];
            tokens[tokenCount].value[1] = '\0';
            tokenCount++;
            i++;
            continue;
        }

        if (expr[i] == ')') {
            tokens[tokenCount].type = RIGHT_PAREN;
            tokens[tokenCount].value[0] = expr[i];
            tokens[tokenCount].value[1] = '\0';
            tokenCount++;
            i++;
            continue;
        }

        fprintf(stderr, "Invalid character: %c\n", expr[i]);
        exit(EXIT_FAILURE);
    }
    return tokenCount;
}

// ������ ��⨭�� ��� �८�ࠧ������ � ������ ������� ������
void shuntingYard(Token* tokens, int tokenCount, Queue* output) {
    Stack operatorStack;
    initStack(&operatorStack);

    for (int i = 0; i < tokenCount; i++) {
        Token token = tokens[i];

        if (token.type == NUMBER) {
            enqueue(output, token);
        }
        else if (token.type == FUNCTION) {
            push(&operatorStack, token);
        }
        else if (token.type == OPERATOR) {
            while (!isStackEmpty(&operatorStack) &&
                operatorStack.data[operatorStack.top].type != LEFT_PAREN &&
                (operatorStack.data[operatorStack.top].precedence > token.precedence ||
                    (operatorStack.data[operatorStack.top].precedence == token.precedence && !token.rightAssociative))) {
                enqueue(output, pop(&operatorStack));
            }
            push(&operatorStack, token);
        }
        else if (token.type == LEFT_PAREN) {
            push(&operatorStack, token);
        }
        else if (token.type == RIGHT_PAREN) {
            while (!isStackEmpty(&operatorStack) &&
                operatorStack.data[operatorStack.top].type != LEFT_PAREN) {
                enqueue(output, pop(&operatorStack));
            }
            if (isStackEmpty(&operatorStack)) {
                fprintf(stderr, "Mismatched parentheses\n");
                exit(EXIT_FAILURE);
            }
            pop(&operatorStack); // ����塞 ����� ᪮���
            if (!isStackEmpty(&operatorStack) &&
                operatorStack.data[operatorStack.top].type == FUNCTION) {
                enqueue(output, pop(&operatorStack));
            }
        }
    }

    // ��७�ᨬ ��⠢訥�� ������� � ��室��� ��।�
    while (!isStackEmpty(&operatorStack)) {
        if (operatorStack.data[operatorStack.top].type == LEFT_PAREN) {
            fprintf(stderr, "Mismatched parentheses\n");
            exit(EXIT_FAILURE);
        }
        enqueue(output, pop(&operatorStack));
    }
}

// ���᫥��� ��ࠦ���� � ���⭮� ����᪮� �����
double evaluateRPN(Queue* output) {
    Stack evalStack;
    initStack(&evalStack);

    while (!isQueueEmpty(output)) {
        Token token = dequeue(output);

        if (token.type == NUMBER) {
            push(&evalStack, token);
        }
        else if (token.type == OPERATOR || token.type == FUNCTION) {
            double a, b, result;
            if (token.type == FUNCTION) {
                a = atof(pop(&evalStack).value);
                if (strcmp(token.value, "sin") == 0) result = sin(a);
                else if (strcmp(token.value, "cos") == 0) result = cos(a);
                else if (strcmp(token.value, "tg") == 0) result = tan(a);
                else if (strcmp(token.value, "ln") == 0) result = log(a);
                else if (strcmp(token.value, "sqrt") == 0) result = sqrt(a);
            }
            else {
                b = atof(pop(&evalStack).value);
                a = atof(pop(&evalStack).value);
                switch (token.value[0]) {
                case '+': result = a + b; break;
                case '-': result = a - b; break;
                case '*': result = a * b; break;
                case '/': result = a / b; break;
                case '^': result = pow(a, b); break;
                }
            }

            Token resultToken;
            resultToken.type = NUMBER;
            snprintf(resultToken.value, sizeof(resultToken.value), "%f", result);
            push(&evalStack, resultToken);
        }
    }

    if (evalStack.top != 0) {
        fprintf(stderr, "Invalid expression\n");
        exit(EXIT_FAILURE);
    }

    return atof(pop(&evalStack).value);
}

int main() {
    char expr[MAX_EXPRESSION_LENGTH];
    Token tokens[MAX_EXPRESSION_LENGTH];
    Queue output;

    printf("������ ��⥬���᪮� ��ࠦ����: ");
    fgets(expr, sizeof(expr), stdin);
    expr[strcspn(expr, "\r\n")] = '\0';

    int tokenCount = tokenize(expr, tokens);
    initQueue(&output);
    shuntingYard(tokens, tokenCount, &output);
    double result = evaluateRPN(&output);
    printf("�������: %g\n", result);

    return 0;
}