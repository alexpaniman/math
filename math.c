#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>

enum operator {
    PLUS, MINUS, UNARY_PLUS, POWER,
    UNARY_MINUS, MULTIPLY, DIVIDE,
    LEFT_BRACKET, RIGHT_BRACKET, SQRT
};

enum expression_type {
    VALUE, OPERATOR
};

struct expression {
    struct expression* next;

    union {
        long double value;
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

    expr->next = NULL;
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

void push_back_value(struct stack* stack, long double value) {
    struct expression* expr = malloc(sizeof *expr);

    expr->type = VALUE;
    expr->value = value;

    push_back(stack, expr);
}

long double parse_value(int* index, char* text) {
    bool was_set = false;
    long double before_dot = 0, after_dot = -1;

    while (true) {
        char symbol = text[*index];

        if (symbol == '.' || symbol == ',') {
            ++ *index;

            after_dot = 0;
            continue;
        }

        short digit = symbol - '0';

        if (digit <= 9 && digit >= 0) {
            if (after_dot >= 0) {
                after_dot *= 10;
                after_dot += digit;
            } else {
                before_dot *= 10;
                before_dot += digit;
            }

            ++ *index;
            was_set = true;
        } else {
            while (after_dot > 1)
                after_dot /= 10;

            long double result = before_dot +
                (after_dot < 0? 0 : after_dot);
            return was_set ? result : -1;
        }
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

struct stack* tokenize(struct stack* stack, char* text) {
    bool should_be_unary = true;
    char symbol; long double value;

    for (int i = 0; (symbol = text[i]) != '\0'; ++ i) {
        if (symbol == ' ')
            continue;

        long double number = parse_value(&i, text);
        if (number >= 0) {
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

    struct expression* current = stack->top;
    struct expression* next;

    do {
        next = current->next;

        if (current->type == VALUE) {
            push_back(rpn_expr_stack, current);
            continue;
        }

        enum operator operator = current->operator;

        if (operator == LEFT_BRACKET) {
            push(operator_stack, current);
            continue;
        }

        if (operator == RIGHT_BRACKET) {
            struct expression* current_expr;

            while (true) {
                if (peek(operator_stack) == NULL) {
                    printf("error: missing (");
                    exit(1);
                }
                
                if (peek(operator_stack)->operator == LEFT_BRACKET) {
                    pop(operator_stack);
                    break;
                }

                push_back(rpn_expr_stack, pop(operator_stack));
            }
            continue;
        }

        if (is_unary_operator(operator)) {
            push(operator_stack, current);
            continue;
        }
    
        short curr_priority = get_priority(operator);
        
        while (true) {
            short last_priority = peek(operator_stack) == NULL?
                -1 : get_priority(peek(operator_stack)->operator);
    
            if (curr_priority > last_priority)
                break;
            
            push_back(rpn_expr_stack, pop(operator_stack));
        }
    
        push(operator_stack, current);
    } while((current = next) != NULL);

    while (peek(operator_stack) != NULL)
        push_back(rpn_expr_stack, pop(operator_stack));

    free(operator_stack);

    return rpn_expr_stack;
}

#define perform_on_top(stack, operation) { \
    struct expression* expr_snd = pop(stack); \
    struct expression* expr_fst = peek(stack); \
\
    long double fst = expr_fst->value; \
    long double snd = expr_snd->value; \
\
    expr_fst->value = operation; \
\
    free(expr_snd); \
}

#define perform_unary_on_top(stack, operation) { \
    long double value = peek(stack)->value; \
    peek(stack)->value = operation; \
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
                printf("[%Lf] ", current->value);
                break;
        }

        current = current->next;
    }
}

long double calculate_rpn_expression(struct stack* stack) {
    struct expression* current = stack->top;
    struct stack calculation = { NULL, NULL }; 
    while(current != NULL) {
        struct expression* next = current->next;

        switch(current->type) {
            case VALUE:
                push(&calculation, current);
                break;

            case OPERATOR: switch(current->operator) {
                case PLUS:
                    perform_on_top(&calculation, fst + snd)
                    break;

                case MINUS:
                    perform_on_top(&calculation, fst - snd)
                    break;
                
                case DIVIDE:
                    perform_on_top(&calculation, fst / snd)
                    break;

                case MULTIPLY:
                    perform_on_top(&calculation, fst * snd)
                    break;
                
                case POWER: 
                    perform_on_top(&calculation, pow(fst, snd))
                    break;

                case SQRT:
                    perform_unary_on_top(&calculation, sqrt(value))
                    break;
                
                case UNARY_MINUS:
                    perform_unary_on_top(&calculation, - value)
                    break;

                case UNARY_PLUS:
                    break;

                default: exit(1);
            }
        }

        current = next;
    }

    struct expression* expr = pop(&calculation);
    long double result = expr->value;
    
    if (calculation.top != NULL) {
        printf("error: wrong expression");
        exit(1);
    }

    free(expr);
    return result;
}

void print_usage(char* name) {
    printf(
        "Usage: %s [:PRECISION] [EXPRESSION]\n"
        "Calculate the expression.\n\n"
        "Example: %s :2 '2 + 2 * 2'\n\n"
        "If no expression is specified read from standard input instead.\n",

        name, name
    );
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        exit(1);
    }

    int precision = -1, start = 1;
    if (argv[1][0] == ':') {
        int index = 1;

        precision = parse_value(&index, argv[1]);
        ++ start;
    }

    if (precision < 0)
        precision = 3;


    struct stack* stack = malloc(sizeof *stack);

    for (int i = start; i < argc; ++ i)
        tokenize(stack, argv[i]);
    
    struct stack* rpn = to_rpn(stack);
    long double result = calculate_rpn_expression(rpn);

    free(rpn);
    free(stack);

    char format[6];
    sprintf(format, "%%.%dLf", precision);

    printf(format, result);
}
