#pragma once

#include "abstract_lqp_node.hpp"

class StoredTableNode final : public AbstractLeafNode {
private:
    std::string name;
public:
    explicit StoredTableNode(std::string name)
            : AbstractLeafNode(LQPNodeType::StoredTable)
            , name(std::move(name)) {}
};

class PredicateNode final : public AbstractSingleInputNode {
private:
    std::string predicate;
public:
    explicit PredicateNode(std::string predicate, const AbstractLQPNode& input)
            : AbstractSingleInputNode(LQPNodeType::Predicate, input)
            , predicate(std::move(predicate)) {}
};

class JoinNode final : public AbstractLQPNode {
private:
    LQPNodeRef left_input;
    LQPNodeRef right_input;
public:
    explicit JoinNode(const AbstractLQPNode& left_input, const AbstractLQPNode& right_input)
            : AbstractLQPNode(LQPNodeType::Join)
            , left_input(left_input.get_node_ref())
            , right_input(right_input.get_node_ref()) {}

    [[nodiscard]] LQPNodeVector get_inputs() const override { return {
                std::ref(left_input.get_node()),
                std::ref(right_input.get_node())
        }; };

    void replace_input(const AbstractLQPNode &old_input, const AbstractLQPNode &new_input) override {
        if (&old_input == &left_input.get_node()) {
            left_input = new_input.get_node_ref();
            return;
        }
        if (&old_input == &right_input.get_node()) {
            right_input = new_input.get_node_ref();
            return;
        }
        throw std::logic_error("cannot replace input: input not found");
    }
};
