#include "lqp.hpp"
#include "lqp_nodes.hpp"

int main() {
    // Step 1: create a simple LQP.
    //
    // [0] [Predicate]
    //  \_[1] [Join]
    //     \_ [3] [StoredTable]
    //     \_ [4] [StoredTable]

    // TODO: create an LQP builder to add a "root exists" invariant to LQP?
    LQP lqp;
    const auto& tbl_a_node = lqp.make_node<StoredTableNode>("tbl_a");
    lqp.set_root(lqp.make_node<PredicateNode>("some predicate",
        lqp.make_node<JoinNode>(
            tbl_a_node,
            lqp.make_node<StoredTableNode>("tbl_b")
        )
    ));
    print_lqp(lqp);

    // Step 2: apply predicate pushdown.
    const auto& new_pred = lqp.wrap_node_with<PredicateNode>(tbl_a_node, "some predicate lower down");
    print_lqp(lqp);
    lqp.bypass_node(new_pred);
    // TODO

    // Step 3: verify LQP.
    //
    // [0] [Join]
    //  \_[1] [Predicate]
    //  |  \_[2] [StoredTable]
    //  \_[3] [Predicate]
    //     \_[4] [StoredTable]
    print_lqp(lqp);
    // TODO

    return 0;
}
