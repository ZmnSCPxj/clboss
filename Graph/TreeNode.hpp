#ifndef GRAPH_TREENODE_HPP
#define GRAPH_TREENODE_HPP

#include<vector>

namespace Graph {

/** struct Graph::TreeNode<a>
 *
 * @brief generic n-ary tree node, with
 * associated data on the nodes.
 */
template<typename a>
struct TreeNode {
	a data;
	TreeNode const* parent;
	std::vector<TreeNode const*> children;
};

}

#endif /* !defined(GRAPH_TREENODE_HPP) */
