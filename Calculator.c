#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>


#define MAX_EXPRESSION_LENGTH 256

// Типы токенов (элементов выражения)
typedef enum {
    NUMBER,      // Число
    OPERATOR,    // Оператор (+, -, *, /, ^)
    FUNCTION,    // Функция (sin, cos, tg, ln, sqrt)
    LEFT_PAREN,  // Левая скобка
    RIGHT_PAREN  // Правая скобка
} TokenType;

// Структура для хранения токена
typedef struct {
    TokenType type;           // Тип токена
    char value[32];           // Значение токена (строка)
    int precedence;           // Приоритет оператора
    bool rightAssociative;    // Правая ассоциативность (для степени ^)
} Token;

// Структура стека для алгоритма Шунтинга
typedef struct {
    Token data[MAX_EXPRESSION_LENGTH];  // Данные стека
    int top;                            // Индекс вершины стека
} Stack;

// Структура очереди для вывода
typedef struct {
    Token data[MAX_EXPRESSION_LENGTH];  // Данные очереди
    int front, rear;                   // Индексы начала и конца очереди
} Queue;

// Инициализация стека
void initStack(Stack* s) {
    s->top = -1;
}

// Проверка на пустоту стека
bool isStackEmpty(Stack* s) {
    return s->top == -1;
}

// Добавление элемента в стек
void push(Stack* s, Token token) {
    if (s->top >= MAX_EXPRESSION_LENGTH - 1) {
        fprintf(stderr, "Stack overflow\n");
        exit(EXIT_FAILURE);
    }
    s->data[++(s->top)] = token;
}

// Извлечение элемента из стека
Token pop(Stack* s) {
    if (isStackEmpty(s)) {
        fprintf(stderr, "Stack underflow\n");
        exit(EXIT_FAILURE);
    }
    return s->data[(s->top)--];
}

// Просмотр верхнего элемента стека
Token peek(Stack* s) {
    if (isStackEmpty(s)) {
        fprintf(stderr, "Stack is empty\n");
        exit(EXIT_FAILURE);
    }
    return s->data[s->top];
}

// Инициализация очереди
void initQueue(Queue* q) {
    q->front = q->rear = -1;
}

// Добавление элемента в очередь
void enqueue(Queue* q, Token token) {
    if (q->rear >= MAX_EXPRESSION_LENGTH - 1) {
        fprintf(stderr, "Queue overflow\n");
        exit(EXIT_FAILURE);
    }
    if (q->front == -1) q->front = 0;
    q->data[++(q->rear)] = token;
}

// Проверка на пустоту очереди
bool isQueueEmpty(Queue* q) {
    return q->front == -1 || q->front > q->rear;
}

// Извлечение элемента из очереди
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

// Проверка, является ли строка функцией
bool isFunction(const char* s) {
    return strcmp(s, "sin") == 0 || strcmp(s, "cos") == 0 ||
        strcmp(s, "tg") == 0 || strcmp(s, "ln") == 0 ||
        strcmp(s, "sqrt") == 0;
}

// Проверка, является ли символ оператором
bool isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
}

// Получение приоритета оператора
int getPrecedence(char op) {
    switch (op) {
    case '+': case '-': return 2;
    case '*': case '/': return 3;
    case '^': return 4;
    default: return 0;
    }
}

// Разбор входного выражения на токены
int tokenize(const char* expr, Token* tokens) {
    int i = 0, tokenCount = 0;
    while (expr[i] != '\0') {
        // Пропускаем пробелы
        if (isspace(expr[i])) {
            i++;
            continue;
        }

        // Обработка чисел
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

        // Обработка функций
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

        // Обработка операторов
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

        // Обработка скобок
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

// Алгоритм Шунтинга для преобразования в обратную польскую запись
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
            pop(&operatorStack); // Удаляем левую скобку
            if (!isStackEmpty(&operatorStack) &&
                operatorStack.data[operatorStack.top].type == FUNCTION) {
                enqueue(output, pop(&operatorStack));
            }
        }
    }

    // Переносим оставшиеся операторы в выходную очередь
    while (!isStackEmpty(&operatorStack)) {
        if (operatorStack.data[operatorStack.top].type == LEFT_PAREN) {
            fprintf(stderr, "Mismatched parentheses\n");
            exit(EXIT_FAILURE);
        }
        enqueue(output, pop(&operatorStack));
    }
}

// Вычисление выражения в обратной польской записи
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

    printf("Введите математическое выражение: ");
    fgets(expr, sizeof(expr), stdin);
    expr[strcspn(expr, "\r\n")] = '\0';

    int tokenCount = tokenize(expr, tokens);
    initQueue(&output);
    shuntingYard(tokens, tokenCount, &output);
    double result = evaluateRPN(&output);
    printf("Результат: %g\n", result);

    return 0;
}