#include <iostream>
#include <utility>
#include <vector>
#include <functional>
#include <memory>
#include <queue>

#include "utils.hpp"

// Forward declarations
class AbstractExpression;
class AbstractLQPNode;

enum class ExpressionType {
    Column,
    Constant,
    Sum
};

class AbstractExpression {
public:
    const ExpressionType type;
};

enum class LQPNodeType {
    StoredTable,
    Projection,
    Predicate,
    Join
};

/// Reference counting is only used for assertions.
/// Only allows const access to the node.
/// Modifications must go through the owner of the node.
class LQPNodeRef final : public utils::ReferenceCounter<AbstractLQPNode> {
    std::reference_wrapper<const AbstractLQPNode> node;
public:
    explicit LQPNodeRef(const AbstractLQPNode& node, int& ref_count)
            : utils::ReferenceCounter<AbstractLQPNode>(ref_count)
            , node(node) {}

    [[nodiscard]] const AbstractLQPNode& get_node() const { return node; }
};

class LQPNodeRefManager {
private:
    int ref_count = 0;
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
    [[nodiscard]] LQPNodeRef get_node_ref() { return LQPNodeRef(node, ref_count); }
};

using LQPNodeVector = std::vector<std::reference_wrapper<const AbstractLQPNode>>;

class AbstractLQPNode : public LQPNodeRefManager {
protected:
    explicit AbstractLQPNode(const LQPNodeType type) : LQPNodeRefManager(*this), type(type) {}
public:
    virtual ~AbstractLQPNode() = default;

    const LQPNodeType type;
    [[nodiscard]] virtual LQPNodeVector get_inputs() const = 0;
};

class ColumnExpression final : public AbstractExpression {
private:
    LQPNodeRef lqp_node;
};

class AbstractLeafNode : public AbstractLQPNode {
protected:
    explicit AbstractLeafNode(const LQPNodeType& type) : AbstractLQPNode(type) {}
public:
    [[nodiscard]] LQPNodeVector get_inputs() const override { return {}; }
};

class AbstractSingleInputNode : public AbstractLQPNode {
protected:
    explicit AbstractSingleInputNode(const LQPNodeType& type, AbstractLQPNode& input)
        : AbstractLQPNode(type)
        , input(input.get_node_ref()) {}
    LQPNodeRef input;
public:
    [[nodiscard]] LQPNodeVector get_inputs() const override { return { std::ref(input.get_node()) }; }
};

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
    explicit PredicateNode(std::string predicate, AbstractLQPNode& input)
        : AbstractSingleInputNode(LQPNodeType::Predicate, input)
        , predicate(std::move(predicate)) {}
};

class JoinNode final : public AbstractLQPNode {
private:
    LQPNodeRef left_input;
    LQPNodeRef right_input;
public:
    explicit JoinNode(AbstractLQPNode& left_input, AbstractLQPNode& right_input)
        : AbstractLQPNode(LQPNodeType::Join)
        , left_input(left_input.get_node_ref())
        , right_input(right_input.get_node_ref()) {}

    [[nodiscard]] LQPNodeVector get_inputs() const override { return {
        std::ref(left_input.get_node()),
        std::ref(right_input.get_node())
    }; };
};

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
    std::unordered_multimap<CNodePtr, NodePtr> node_parents {};

//     LQPNodeRef root;
    NodePtr root = nullptr;

public:
    ~LQP() {
        // TODO topological sort to handle diamond schemas

        // Remove nodes sequentially starting with root and continuing through their inputs.
        if (root == nullptr) { throw std::runtime_error("node root not set"); }
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

    void set_root(const AbstractLQPNode& node) {
        // TODO don't allow the LQP to NOT have a root - create an LQPBuilder
        root = const_cast<AbstractLQPNode *>(&node);
    }

    template <typename T, typename... Args>
    [[nodiscard]] T& make_node(Args&&... args) {
        auto node = std::make_unique<T>(std::forward<Args>(args)...);
        auto node_ptr = node.get();
        nodes[node_ptr] = std::move(node);
        return *node_ptr;
    }

    void remove_node(const AbstractLQPNode& node) {
        if (node.get_ref_count() != 0) { throw std::logic_error("cannot remove node: non-zero reference count"); }
        if (nodes.erase(&node) == 0) { throw std::logic_error("cannot remove node: not found in LQP"); }
    }

    void replace_node(const AbstractLQPNode& old_node, AbstractLQPNode& new_node) {
        throw std::logic_error("not implemented");
    }
};

int main() {
    // Step 1: create a simple LQP.
    //
    // [0] [Predicate]
    //  \_[1] [Join]
    //     \_ [3] [StoredTable]
    //     \_ [4] [StoredTable]

    // TODO: create an LQP builder to add a "root exists" invariant to LQP?
    LQP lqp;
    lqp.set_root(lqp.make_node<PredicateNode>("some predicate",
        lqp.make_node<JoinNode>(
            lqp.make_node<StoredTableNode>("tbl_a"),
            lqp.make_node<StoredTableNode>("tbl_b")
        )
    ));

    // Step 2: apply predicate pushdown.
    // TODO

    // Step 3: verify LQP.
    //
    // [0] [Join]
    //  \_[1] [Predicate]
    //  |  \_[2] [StoredTable]
    //  \_[3] [Predicate]
    //     \_[4] [StoredTable]
    // TODO

    return 0;
}
