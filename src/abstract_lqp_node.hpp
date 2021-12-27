#pragma once

#include <iostream>
#include <functional>

#include "fwd.hpp"
#include "utils.hpp"

enum class LQPNodeType {
    StoredTable,
    Projection,
    Predicate,
    Join
};

/// Only allows const access to the node.
/// Modifications must go through the owner of the node.
/// Reference counting is only used for consistency checks.
class LQPNodeRef final : public utils::ReferenceCounter {
    std::reference_wrapper<const AbstractLQPNode> node;
public:
    explicit LQPNodeRef(const AbstractLQPNode& node, int& ref_count)
            : utils::ReferenceCounter(ref_count)
            , node(node) {}

    [[nodiscard]] const AbstractLQPNode& get_node() const { return node; }
};

class LQPNodeRefManager {
private:
    mutable int ref_count = 0;
    AbstractLQPNode& node;
protected:
    ~LQPNodeRefManager() {
        if (ref_count != 0) {
            std::cerr << "dangling reference" << std::endl;
            std::terminate();
        }
    }
public:
    explicit LQPNodeRefManager(AbstractLQPNode& node) : node(node) {}
    [[nodiscard]] int get_ref_count() const { return ref_count; }
    [[nodiscard]] LQPNodeRef get_node_ref() const { return LQPNodeRef(node, ref_count); }
};

using LQPNodeVector = std::vector<std::reference_wrapper<const AbstractLQPNode>>;

class AbstractLQPNode : public LQPNodeRefManager {
protected:
    explicit AbstractLQPNode(const LQPNodeType type) : LQPNodeRefManager(*this), type(type) {}

public:
    virtual ~AbstractLQPNode() = default;

    const LQPNodeType type;

    [[nodiscard]] virtual LQPNodeVector get_inputs() const = 0;

    virtual void replace_input(const AbstractLQPNode& old_input, const AbstractLQPNode& new_input) = 0;
};

class AbstractLeafNode : public AbstractLQPNode {
protected:
    explicit AbstractLeafNode(const LQPNodeType& type) : AbstractLQPNode(type) {}
public:
    [[nodiscard]] LQPNodeVector get_inputs() const override { return {}; }

    void replace_input(const AbstractLQPNode &old_input, const AbstractLQPNode &new_input) override {
        throw std::logic_error("cannot replace input: node is a leaf");
    }
};

class AbstractSingleInputNode : public AbstractLQPNode {
protected:
    explicit AbstractSingleInputNode(const LQPNodeType& type, const AbstractLQPNode& input)
            : AbstractLQPNode(type)
            , input(input.get_node_ref()) {}

    LQPNodeRef input;
public:
    [[nodiscard]] std::reference_wrapper<const AbstractLQPNode> get_input() const { return std::ref(input.get_node()); }

    [[nodiscard]] LQPNodeVector get_inputs() const override { return { get_input() }; }

    void replace_input(const AbstractLQPNode &old_input, const AbstractLQPNode &new_input) override {
        if (&old_input != &input.get_node()) throw std::logic_error("cannot replace input: input not found");
        input = new_input.get_node_ref();
    }
};
