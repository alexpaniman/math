#include<iostream>
#include<stack>

enum class ExpressionType {
    OPERATOR, VALUE
};

enum class Operator {
    PLUS, MINUS, POWER, DIVIDE,
    LEFT_BRACKET, RIGHT_BRACKET;
}

struct Expression {
    ExpressionType type;
    union {
        int value;
        Operator op;
    }
}

void push_expressions(stack<Expression> stack, char* expression) {
    char symbol;
    for (int i = 0; (symbol = expression[i]) != '\0'; ++ i) {
        switch(symbol) {
            case '-':
                break;
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 2)
        exit(1);


}
