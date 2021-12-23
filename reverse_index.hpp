#pragma once

#include <algorithm>
#include <ranges>
#include <unordered_map>

template <typename T>
class ReverseDAGIndex final {
private:
    std::unordered_multimap<T*, T*> node_parents;

public:
    [[nodiscard]] int get_parent_count(T& node) const {
        return node_parents.count(&node);
    }

    class NodeParentIterator final {
        using iter_t = typename decltype(node_parents)::const_iterator;

    private:
        const iter_t begin_iter;
        const iter_t end_iter;

    public:
        explicit NodeParentIterator(std::pair<iter_t, iter_t> range) : begin_iter(range.first), end_iter(range.second) {}
        [[nodiscard]] iter_t begin() const { return begin_iter; }
        [[nodiscard]] iter_t end() const { return end_iter; }
    };
    static_assert(std::ranges::range<NodeParentIterator>);

    NodeParentIterator get_parents(T& node) const {
        return NodeParentIterator(node_parents.equal_range(&node));
    }

    void add(T& input, T& parent) {
        // Verify the link does not already exist.
        const auto& parents = get_parents(input);
        if (std::ranges::count_if(parents.begin(), parents.end(), [&parent](const auto& link) {
            return link.second == &parent;
        }) != 0) {
            throw std::logic_error("cannot add link: link already exists");
        }

        node_parents.insert(typename decltype(node_parents)::value_type(&input, &parent));
    }

    void remove(T& input, T& parent) {
        auto input_parents_range = node_parents.equal_range(&input);
        auto parent_link = std::find_if(
                input_parents_range.first,
                input_parents_range.second,
                [&parent](auto& input_parent) { return input_parent.second == &parent; }
        );
        if (parent_link == input_parents_range.second) {
            throw std::logic_error("cannot remove parent link: not found");
        }
        node_parents.erase(parent_link);
    }

    void replace_input(T& node, T& new_node) {
        if (get_parent_count(new_node) != 0) {
            throw std::logic_error("cannot replace input: new node already has parents");
        }

        for (const auto [_, parent] : get_parents(node)) {
            add(new_node, *parent);
        }
        node_parents.erase(&node);
    }
};
