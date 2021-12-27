#pragma once

#include <memory>
#include <queue>

#include "abstract_lqp_node.hpp"
#include "reverse_index.hpp"

class LQP {
    // TODO
    // - mutate itself
    // - integrity checks
    // - attach itself?
    // - split itself? / LQP view
    // - topological-sort destructor
private:
    using NodePtr = AbstractLQPNode *;
    using CNodePtr = const AbstractLQPNode *;
    /// Owns the nodes and provides an indexed access for removal.
    std::unordered_map<CNodePtr, std::unique_ptr<AbstractLQPNode>> nodes;
    ReverseDAGIndex<AbstractLQPNode> node_parents;

//     LQPNodeRef root;
    NodePtr root = nullptr;

    AbstractLQPNode& get_mutable(const AbstractLQPNode& node) {
        return const_cast<AbstractLQPNode&>(node);
    }

public:
    ~LQP() {
        // TODO topological sort to handle diamond schemas

        // Remove nodes sequentially starting with root and continuing through their inputs.
        if (root == nullptr) { std::cerr << "node root not set" << std::endl; std::terminate(); }
        std::queue<CNodePtr> removal_queue({ root });

        while (!removal_queue.empty()) {
            auto el = removal_queue.front();
            removal_queue.pop();

            for (auto input : el->get_inputs()) {
                removal_queue.push(&input.get());
            }

            remove_node(*el);
        }
    }

    // TODO don't allow the LQP to NOT have a root - create an LQPBuilder
    void set_root(const AbstractLQPNode& node) {
        // const_cast is allowed, because we have mutable access through `nodes`.
        root = const_cast<AbstractLQPNode *>(&node);
    }

    const AbstractLQPNode& get_root() {
        if (root == nullptr) throw std::logic_error("LQP root not set");
        return *root;
    }

    template <typename T, typename... Args>
    [[nodiscard]] const T& make_node(Args&&... args) {
        static_assert(std::derived_from<T, AbstractLQPNode>);

        // Create node.
        auto node = std::make_unique<T>(std::forward<Args>(args)...);

        // Store the node and the parent relation.
        auto node_ptr = node.get();
        for (auto& input : node->get_inputs()) {
            node_parents.add(input, *node);
        }
        nodes[node_ptr] = std::move(node);
        return *node_ptr;
    }

    void remove_node(const AbstractLQPNode& node) {
        // Assert invariants.
        if (node.get_ref_count() != 0) { throw std::logic_error("cannot remove node: non-zero reference count"); }
        if (node_parents.get_parent_count(node)) { throw std::logic_error("cannot remove node: parent links exist"); }

        // Remove inputs' parent links to this node.
        for (auto& input : node.get_inputs()) {
            node_parents.remove(input, node);
        }

        // Remove node.
        if (nodes.erase(&node) == 0) { throw std::logic_error("cannot remove node: not found in LQP"); }
    }

    /// Substitutes a node for a new single-input node that has the old node as its input.
    template <typename T, typename... Args>
    const T& wrap_node_with(const AbstractLQPNode& node, Args&&... args) {
        static_assert(std::derived_from<T, AbstractSingleInputNode>);

        // Create a new node with the wrapped node as input.
        auto& new_node = make_node<T>(std::forward<Args>(args)..., node);

        // Replace the old node with the new node for each parent.
        for (const auto& [_, parent_ptr] : node_parents.get_parents(node)) {
            if (parent_ptr == &new_node) continue;
            get_mutable(*parent_ptr).replace_input(node, new_node);
        }

        // Update parent node index.
        node_parents.remove(node, new_node);
        node_parents.replace_input(node, new_node);
        node_parents.add(node, new_node);

        return new_node;
    }

    void bypass_node(const AbstractSingleInputNode& node) {
        for (const auto& [_, parent] : node_parents.get_parents(node)) {
            get_mutable(*parent).replace_input(node, node.get_input());
        }

        // TODO extract common logic with remove_node
        if (node.get_ref_count() != 0) { throw std::logic_error("cannot remove node: non-zero reference count"); }
        if (nodes.erase(&node) == 0) { throw std::logic_error("cannot remove node: not found in LQP"); }

        node_parents.remove(node.get_input(), node);
        node_parents.replace_input(node, node.get_input());
    }

    template<typename State> using Visitor = std::function<bool(const AbstractLQPNode&, State&)>;

    /// State is passed by value to the visit function and by reference to the visitor.
    /// When visitor modifies the state, the children receive a copy of that modified state.
    template<typename State>
    void visit(const AbstractLQPNode& node, const Visitor<State>& visitor, State state) {
        auto visit_inputs = visitor(node, state);
        if (!visit_inputs) return;
        for (auto& input : node.get_inputs()) {
            visit(input, visitor, state);
        }
    }
};

void print_lqp(LQP& lqp) {
    static auto node_name = [](const AbstractLQPNode& node) {
        switch(node.type) {
            case LQPNodeType::Join: return "Join";
            case LQPNodeType::Predicate: return "Predicate";
            case LQPNodeType::Projection: return "Projection";
            case LQPNodeType::StoredTable: return "StoredTable";
        }
    };
    lqp.visit<int>(lqp.get_root(), [](const AbstractLQPNode& node, int& indent) {
        std::cout << std::string(indent, ' ') << node_name(node) << std::endl;
        indent += 2;
        return true;
    }, 0);
}
