#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>

void print_usage(char* name) {
    printf(
        "Usage: %s [~PRECISION] [EXPRESSION]\n"
        "Calculate the expression.\n\n"
        "Example: %s 2 + 2 * 2\n\n"
        "If no expression is specified read from standard input instead.\n",

        name, name
    );
}

enum operator {
    PLUS, MINUS,
    UNARY_PLUS, UNARY_MINUS,
    MULTIPLY, DIVIDE,
    POWER, SQRT,
    LEFT_BRACKET,
    RIGHT_BRACKET
};

enum expression_type {
    VALUE, OPERATOR
};

struct expression {
    struct expression* next;

    union {
        long long value;
        enum operator operator;
    };
    enum expression_type type;
};

struct stack {
    struct expression* top;
    struct expression* bottom;
};

void push(struct stack* stack, struct expression* expr) {
    expr->next = stack->top;
    stack->top = expr;

    if (stack->bottom == NULL) 
        stack->bottom = stack->top;
}

void push_back(struct stack* stack, struct expression* expr) {
    if (stack->bottom != NULL)
        stack->bottom->next = expr;
    stack->bottom = expr;

    if (stack->top == NULL)
        stack->top = stack->bottom;
}

struct expression* pop(struct stack* stack) {
    struct expression* top = stack->top;
    stack->top = top->next;
    return top;
}

struct expression* peek(struct stack* stack) {
    return stack->top;
}

long long parse_value(int* index, char* text) {
    long long start = 0;
    bool is_negative = false;

    switch(text[*index]) {
        case '-':
            is_negative = true;
        case '+':
            ++ *index;
        default:
            for (;; ++ *index) {
                short digit = text[*index] - '0';
                if (digit < 0 || digit > 9) {
                    -- *index;
                    break;
                }

                start *= 10;
                start += digit;
            }
    }

    return (is_negative ? -1 : 1) * start;
}

struct expression* tokenize(char* text) {
    struct expression* expr = malloc(sizeof *expr);
    struct expression* expr_top = expr;

    enum expression_type last_expr;
    char symbol; long long value;

    for (int i = 0; (symbol = text[i]) != '\0'; ++ i) {
        expr->type = OPERATOR;
        switch(symbol) {
            case ' ':
                continue;
            case '-':
                if (last_expr == VALUE) {
                    expr->operator = MINUS;
                } else {
                    expr->type = VALUE;
                    expr->value = parse_value(&i, text);
                    last_expr = VALUE;
                }
                break;
            case '+':
                if (last_expr == VALUE) {
                    expr->operator = PLUS;
                } else {
                    expr->type = VALUE;
                    expr->value = parse_value(&i, text);
                    last_expr = VALUE;
                }
                break;
            case '/':
                expr->operator = DIVIDE;
                last_expr = OPERATOR;
                break;
            case '*':
                expr->operator = MULTIPLY;
                last_expr = OPERATOR;
                break;
            case '^':
                expr->operator = POWER;
                last_expr = OPERATOR;
                break;
            case '(':
                expr->operator = LEFT_BRACKET;
                last_expr = OPERATOR;
                break;
            case ')':
                expr->operator = RIGHT_BRACKET;
                last_expr = OPERATOR;
                break;
            case 's':
                for (int j = 0; j < 4; ++ j, ++ i)
                    if (text[i] != "sqrt"[j]) {
                        printf("error: expected %c", "sqrt"[j]);
                        exit(1);
                    }

                expr->operator = SQRT;
                last_expr = OPERATOR;
                break;
            default:
                expr->type = VALUE;
                expr->value = parse_value(&i, text);
                last_expr = VALUE;
                break;
        }
        
        if (text[i + 1] != '\0') {
            struct expression* next_expr = malloc(sizeof *next_expr);
            expr->next = next_expr; expr = next_expr;
        }
    }

    return expr_top;
}

void print_expression(struct expression* expr) {
    struct expression* current = expr;
    while (current != NULL) {
        switch(current->type) {
            case OPERATOR:
                switch(current->operator) {
                    case UNARY_MINUS:
                        printf(" [unary -]");
                        break;
                    case UNARY_PLUS:
                        printf(" [unary +]");
                        break;
                    case PLUS:
                        printf(" [+] ");
                        break;
                    case MINUS:
                        printf(" [-] ");
                        break;
                    case DIVIDE:
                        printf(" [/] ");
                        break;
                    case MULTIPLY:
                        printf(" [*] ");
                        break;
                    case POWER:
                        printf(" [^] ");
                        break;
                    case SQRT:
                        printf(" [sqrt] ");
                        break;
                    case LEFT_BRACKET:
                        printf(" [(] ");
                        break;
                    case RIGHT_BRACKET:
                        printf(" [)] ");
                        break;
                }
                break;
            case VALUE:
                printf(" [%lld] ", current->value);
                break;
        }

        current = current->next;
    }
}

struct priority_map {
    enum operator operator;
    short priority;
};

struct priority_map const priority[] = { 
    { PLUS , 0 },
    { MINUS, 0 },

    { MULTIPLY, 1 },
    { DIVIDE  , 1 },

    { POWER, 2 }
};

short get_priority(enum operator operator) {
    for (size_t i = 0; i < sizeof(priority) / sizeof(*priority); ++ i) {
        struct priority_map map = priority[i];

        if (map.operator == operator)
            return map.priority;
    }
   
    return -1;
}

struct stack* to_rpn(struct expression* expr) {
    struct stack* rpn_expr_stack = malloc(sizeof *rpn_expr_stack);
    struct stack* operator_stack = malloc(sizeof *operator_stack);

    struct expression* rpn_stack_bottom = rpn_expr_stack->top;

    struct expression* current = expr;

    while(current != NULL) {
        struct expression* next = current->next;

        switch(current->type) {
            case VALUE:
                push_back(rpn_expr_stack, current);
                break;

            case OPERATOR: {
                if (current->operator == LEFT_BRACKET) {
                    push(operator_stack, current);
                    break;
                }

                if (current->operator == RIGHT_BRACKET) {
                    struct expression* current_expr;

                    while (true) {
                        if (peek(operator_stack) == NULL) {
                            printf("error: missing )");
                            exit(1);
                        }
                        
                        if (peek(operator_stack)->operator == LEFT_BRACKET) {
                            pop(operator_stack);
                            break;
                        }

                        push_back(rpn_expr_stack, pop(operator_stack));
                    }

                    break;
                }

                short curr_priority = get_priority(current->operator);
                short last_priority = peek(operator_stack) == NULL? -1 : get_priority(
                    peek(operator_stack)->operator
                );
                
                while (curr_priority <= last_priority) {
                    if (peek(operator_stack) == NULL)
                        break;
                    
                    push_back(rpn_expr_stack, pop(operator_stack));

                    last_priority = peek(operator_stack) == NULL? -1 : get_priority(
                        peek(operator_stack)->operator
                    );
                }

                push(operator_stack, current);
            }
        }

        current = next;
    }

    while (peek(operator_stack) != NULL)
        push_back(rpn_expr_stack, pop(operator_stack));

    return rpn_expr_stack;
}

int main(int argc, char** argv) {
    // TODO: make unary minus work
//    print_expression(tokenize("-2000 + (sqrt (2 * 2)) * 78^2"));
//    printf("\n");
    print_expression(to_rpn(tokenize("2^3/(5*5) + 10"))->top);
}
