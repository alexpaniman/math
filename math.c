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

void push_back_operator(struct stack* stack, enum operator operator) {
    struct expression* expr = malloc(sizeof *expr);

    expr->type = OPERATOR;
    expr->operator = operator;

    push_back(stack, expr);
}

void push_back_value(struct stack* stack, long long value) {
    struct expression* expr = malloc(sizeof *expr);

    expr->type = VALUE;
    expr->operator = value;

    push_back(stack, expr);
}

long long parse_value(int* index, char* text) {
    bool was_set = false;
    long long start = 0;

    while (true) {
        short digit = text[*index] - '0';

        if (digit <= 9 && digit >= 0) {
            start *= 10;
            start += digit;
            ++ *index;

            was_set = true;
        } else return was_set ? start : -1;
    }
}

void check_sqrt(int* index, char* text) {
    for (int j = 0; j < 4; ++ j, ++ *index)
        if (text[*index] != "sqrt"[j]) {
            printf("error: expected '%c' in sqrt instead of '%c'",
                   "sqrt"[j], text[*index]);
            exit(1);
        }

    -- *index;
}

struct stack* tokenize(char* text) {
    struct stack* stack = malloc(sizeof *stack);

    bool should_be_unary = false;
    char symbol; long long value;

    for (int i = 0; (symbol = text[i]) != '\0'; ++ i) {
        if (symbol == ' ')
            continue;

        long long number = parse_value(&i, text);
        if (number > 0) {
            push_back_value(stack, number);
            -- i;
            should_be_unary = false;
            continue;
        }

        switch(symbol) {
            case 's':
                check_sqrt(&i, text);
                push_back_operator(stack, SQRT);
                continue;

            case '(':
                push_back_operator(stack, LEFT_BRACKET);
                continue;

            case ')':
                push_back_operator(stack, RIGHT_BRACKET);
                should_be_unary = false;
                continue;   
        }

        if (should_be_unary)
            switch(symbol) {
                case '+':
                    push_back_operator(stack, UNARY_PLUS);
                    break;

                case '-':
                    push_back_operator(stack, UNARY_MINUS);
                    break;

                default:
                    printf("error: '%c' is not an unary operator!", symbol);
                    exit(1);
            }

        else {
            should_be_unary = true;

            switch(symbol) {
                case '-':
                    push_back_operator(stack, MINUS);
                    break;
    
                case '+':
                    push_back_operator(stack, PLUS);
                    break;
    
                case '/':
                    push_back_operator(stack, DIVIDE);
                    break;
    
                case '*':
                    push_back_operator(stack, MULTIPLY);
                    break;
    
                case '^':
                    push_back_operator(stack, POWER);
                    break;

                default:
                    printf("error: unexpected symbol '%c'", symbol);
                    exit(1);
            }
        }
    }

    return stack;
}

void print_expression(struct stack* stack) {
    struct expression* current = stack->top;

    while (current != NULL) {

        switch(current->type) {
            case OPERATOR:
                switch(current->operator) {
                    case UNARY_MINUS:
                        printf("[unary -] ");
                        break;
                    case UNARY_PLUS:
                        printf("[unary +] ");
                        break;
                    case PLUS:
                        printf("[+] ");
                        break;
                    case MINUS:
                        printf("[-] ");
                        break;
                    case DIVIDE:
                        printf("[/] ");
                        break;
                    case MULTIPLY:
                        printf("[*] ");
                        break;
                    case POWER:
                        printf("[^] ");
                        break;
                    case SQRT:
                        printf("[sqrt] ");
                        break;
                    case LEFT_BRACKET:
                        printf("[(] ");
                        break;
                    case RIGHT_BRACKET:
                        printf("[)] ");
                        break;
                }
                break;

            case VALUE:
                printf("[%lld] ", current->value);
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

    { POWER, 2 },

    { SQRT       , 3 },
    { UNARY_PLUS , 3 },
    { UNARY_MINUS, 3 }
};

short get_priority(enum operator operator) {
    for (size_t i = 0; i < sizeof(priority) / sizeof(*priority); ++ i) {
        struct priority_map map = priority[i];

        if (map.operator == operator)
            return map.priority;
    }
   
    return -1;
}

bool is_unary_operator(enum operator operator) {
    switch(operator) {
        case SQRT:
        case UNARY_MINUS:
        case UNARY_PLUS:
            return true;

        default:
            return false;
    }
}

struct stack* to_rpn(struct stack* stack) {
    struct stack* rpn_expr_stack = malloc(sizeof *rpn_expr_stack);
    struct stack* operator_stack = malloc(sizeof *operator_stack);

    struct expression* rpn_stack_bottom = rpn_expr_stack->top;

    struct expression* current = stack->top;

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

                if (is_unary_operator(current->operator)) {
                    push(operator_stack, current);
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
//  char* expression = "sqrt - sqrt +++ sqrt  - -10";
    char* expression = "2 +++++ - 3 * 4 - 1^(3^2 + 4 * (3 + 2)) / 2";
//  char* expression = "1 ++ 2";

    printf("------------------------------------------------------\n");
    printf("expression: %s\n", expression);
   
    struct stack* tokenized = tokenize(expression);
    printf("tokenized: "); print_expression(tokenized); printf("\n");

    struct stack* rpn = to_rpn(tokenized);
    printf("rpn:       "); print_expression(rpn); printf("\n");
    printf("------------------------------------------------------\n");
}
