#include<stdio.h>
#include<stdlib.h>

void print_usage(char* name) {
    printf(
        "Usage: %s [~PRECISION] [EXPRESSION]\n"
        "Calculate the expression.\n\n"
        "Example: %s 2 + 2 * 2\n\n"
        "If no expression is specified read from standard input instead.\n",

        name, name
    );
}

typedef struct {
    union {
        int value;
        int operation;
    };

    int type;
} expression_t;

struct stack {
    struct stack* next;
    expression_t expression;
};

typedef struct stack stack_t;

stack_t empty_stack(void) {
    stack_t stack;

    stack.next = NULL;
    stack.expression.type = -1;

    return stack;
}

stack_t* push(stack_t* stack, expression_t item) {
    stack_t* new_stack = (stack_t*) malloc(sizeof(stack_t));

    new_stack->next = stack;
    new_stack->expression = item;

    return new_stack;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        exit(1);
    }
    
    stack_t stack = empty_stack();
}
