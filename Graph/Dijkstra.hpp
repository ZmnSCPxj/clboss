#ifndef GRAPH_DIJKSTRA_HPP
#define GRAPH_DIJKSTRA_HPP

#include"Graph/TreeNode.hpp"
#include"Util/make_unique.hpp"
#include<algorithm>
#include<functional>
#include<map>
#include<memory>
#include<queue>
#include<set>

namespace Graph {

/** class Graph::Dijkstra<Node, Cost>
 *
 * @brief an implementation of the Dijkstra
 * shortest-path algorithm.
 *
 * @desc this implementation uses a data
 * interface, i.e. it is mostly passive and
 * will just wait for data to be fed into it
 * by the client.
 *
 * This allows clients with non-standard
 * execution policies and incremental map
 * loading to use this class.
 */
template< typename Node
	, typename Cost = double
	, typename CmpN = std::less<Node>
	, typename CmpC = std::less<Cost>
	, typename AddC = std::plus<Cost>
	>
class Dijkstra {
public:
	typedef Graph::TreeNode<std::pair<Node const*, Cost>> TreeNode;
	typedef
	std::map< Node
		, std::unique_ptr<TreeNode>
		, CmpN
		> Result;

	Dijkstra() =delete;
	Dijkstra(Dijkstra&&) =default;
	Dijkstra( Dijkstra const& o
		) : cmp_c(o.cmp_c)
		  , add_c(o.add_c)
		  , q(o.q)
		  , treenodes()
		  , closed(o.closed)
		  {
		/* treenodes have to be copied.  */
		copy_treenodes(o.treenodes);
	}

	explicit
	Dijkstra( Node root
		, Cost root_cost = 0
		, CmpN cmp_n_ = CmpN()
		, CmpC cmp_c_ = CmpC()
		, AddC add_c_ = AddC()
		) : cmp_c(cmp_c_)
		  , add_c(std::move(add_c_))
		  , q(WrappedCmpC(cmp_c_))
		  , treenodes(cmp_n_)
		  , closed()
		  {
		initialize(std::move(root), std::move(root_cost));
	}

	/** Graph::Dijkstra<Node, Cost>::current
	 *
	 * @brief return the current node being considered
	 * by the algorithm.
	 *
	 * @desc the client should provide the neighbors
	 * of the current node via the `neighbor` member
	 * function.
	 *
	 * This function can return `nullptr`, meaning the
	 * run has ended and there are no more nodes
	 * reachable from the root node.
	 *
	 * If you are using this class as a pathfinding
	 * algorithm (as opposed to a shortest-path-tree
	 * algorithm) you can stop when the goal node (or
	 * one of the goal nodes) is returned by this
	 * function.
	 */
	Node const* current() const {
		if (q.empty())
			return nullptr;
		return q.top().first->data.first;
	}
	/** Graph::Dijsktra<Node, Cost>::neighbor
	 *
	 * @brief inform the algorithm that the `current()`
	 * node has a directly-reachable neighbor, that has
	 * a cost `c` to go to.
	 *
	 * @desc do not call this if `current()` returns
	 * `nullptr`.
	 * Calling this will not change what `current()`
	 * returns.
	 */
	void neighbor(Node n, Cost c) {
		/* for each neighbor v of u: */ /* v = n, u = current() */
		auto& u = *q.top().first;
		auto& u_cost = u.data.second;
		auto alt = add_c(u_cost, c);

		auto it = treenodes.find(n);
		if (it == treenodes.end()) {
			/* No TreeNode yet, initialize it to alt.  */
			auto& v = create_treenode(n);
			v.data.second = alt;
			reparent(v, u);
			q.push(std::make_pair(&v, alt));
		} else {
			auto& v = *it->second;
			if (cmp_c(alt, v.data.second)) {
				v.data.second = alt;
				reparent(v, u);
				q.push(std::make_pair(&v, alt));
			}
		}
	}
	/** Graph::Dijkstra<Node, Cost>::end_neighbors
	 *
	 * @brief inform the algorithm that the `current()`
	 * node has no more neighbors.
	 *
	 * @desc do not call this if `current()` returns
	 * `nullptr`.
	 * After this call, `current()` will change.
	 */
	void end_neighbors() {
		auto u = q.top().first;
		closed.insert(u);
		q.pop();
		/* Filter out those already in the closed set.  */
		while ( !q.empty()
		     && closed.find(q.top().first) != closed.end()
		      )
			q.pop();
	}

	/** Graph::Dijkstra<Node::Cost>::finalize
	 *
	 * @brief destruct this algorithm and extract its
	 * result.
	 *
	 * @desc after calling this function, the object is
	 * now in an unusable state.
	 */
	Result
	finalize()&& {
		return std::move(treenodes);
	}

private:
	class WrappedCmpC {
	private:
		CmpC cmp_c;
	public:
		explicit
		WrappedCmpC(CmpC cmp_c_) : cmp_c(cmp_c_) { }
		bool operator()( std::pair<TreeNode*, Cost> const& a
			       , std::pair<TreeNode*, Cost> const& b
			       ) const {
			/* We want the priority queue to return
			 * the *lowest* cost, but std::priority_queue
			 * is designed to return the *highest* score
			 * when given std::less.
			 * So flip the args here.
			 */
			return cmp_c(b.second, a.second);
		}
	};

	CmpC cmp_c;
	AddC add_c;
	std::priority_queue< std::pair<TreeNode*, Cost>
			   , std::vector<std::pair<TreeNode*, Cost>>
			   , WrappedCmpC
			   > q;
	Result treenodes;
	std::set<TreeNode*> closed;

	void initialize(Node root, Cost root_cost) {
		auto& tn = get_treenode(root);
		tn.data.second = root_cost;
		q.push(std::make_pair(&tn, root_cost));
	}
	TreeNode& get_treenode(Node n) {
		auto it = treenodes.find(n);
		if (it == treenodes.end())
			return create_treenode(std::move(n));
		return *it->second;
	}
	TreeNode& create_treenode(Node n) {
		auto tn = Util::make_unique<TreeNode>();
		auto itb = treenodes.insert(std::make_pair(
			std::move(n), std::move(tn)
		));
		auto it = itb.first;
		it->second->data.first = &it->first;
		return *it->second;
	}
	void reparent(TreeNode& node, TreeNode& parent) {
		if (node.parent) {
			/* Remove it from the existing parent children.  */
			auto& curparent = const_cast<TreeNode&>(*node.parent);
			auto& children = curparent.children;
			children.erase( std::remove( children.begin()
						   , children.end()
						   , &node
						   )
				      , children.end()
				      );
		}
		node.parent = &parent;
		parent.children.push_back(&node);
	}

	void copy_treenodes(Result const& src) {
		for (auto const& e : src) {
			auto const& n = e.first;
			auto const& tn = *e.second;
			auto& my_tn = get_treenode(n);
			if (tn.parent) {
				auto const& parent_n = *tn.parent->data.first;
				auto& my_parent_tn = get_treenode(parent_n);
				reparent(my_tn, my_parent_tn);
			}
			my_tn.data.second = tn.data.second;
		}
	}
};

}

#endif /* !defined(GRAPH_DIJKSTRA_HPP) */
