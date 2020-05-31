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
    struct expression* result = expr;

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

    return result;
}

void print_expression(struct expression* expr) {
    struct expression* current = expr;
    while (current != NULL) {
        switch(current->type) {
            case OPERATOR:
                switch(current->operator) {
                    case PLUS:
                        printf(" + ");
                        break;
                    case MINUS:
                        printf(" - ");
                        break;
                    case DIVIDE:
                        printf(" / ");
                        break;
                    case MULTIPLY:
                        printf(" * ");
                        break;
                    case POWER:
                        printf(" ^ ");
                        break;
                    case SQRT:
                        printf("sqrt ");
                        break;
                    case LEFT_BRACKET:
                        printf("(");
                        break;
                    case RIGHT_BRACKET:
                        printf(")");
                        break;
                }
                break;
            case VALUE:
                printf("%lld", current->value);
                break;
        }

        current = current->next;
    }
}

struct expression to_rpn(struct expression expr) {
    struct expression rpn_expr;

    // TODO: convert to RPN
    return rpn_expr;
}

int main(int argc, char** argv) {
    print_expression(tokenize("2000 + (sqrt (2 * 2)) * 78^2"));
}
