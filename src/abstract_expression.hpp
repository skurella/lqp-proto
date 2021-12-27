#pragma once

enum class ExpressionType {
    Column,
    Constant,
    Sum
};

class AbstractExpression {
public:
    const ExpressionType type;
};
