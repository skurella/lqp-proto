#pragma once

#include "abstract_expression.hpp"
#include "abstract_lqp_node.hpp"

class ColumnExpression final : public AbstractExpression {
private:
    LQPNodeRef lqp_node;
};
