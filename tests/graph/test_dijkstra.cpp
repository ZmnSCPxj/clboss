#undef NDEBUG
#include"Graph/Dijkstra.hpp"
#include"Ln/NodeId.hpp"
#include<assert.h>

namespace {

auto const A = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000000");
auto const B = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000001");
auto const C = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000002");

template<typename a>
void test_copy(Graph::Dijkstra<a> const& dijkstra) {
	auto copy = dijkstra;
	if (dijkstra.current()) {
		assert(copy.current());
		assert(*copy.current() == *dijkstra.current());
	} else {
		assert(!copy.current());
	}
}

}

int main() {
	auto dijkstra = Graph::Dijkstra<Ln::NodeId>(A);
	assert(dijkstra.current());
	assert(*dijkstra.current() == A);

	test_copy(dijkstra);

	{
		/* We can finalize at any time, at the cost of
		 * creating a copy of the shortest-path tree.  */
		auto tmp = Graph::Dijkstra<Ln::NodeId>(dijkstra).finalize();
		assert(tmp.size() == 1);
		assert(tmp.find(A) != tmp.end());
		assert(tmp[A]->data.second == 0);
	}

	/*
	 * A --5-- C
	 *  \     /
	 *   1   1
	 *    \ /
	 *     B
	 */
	dijkstra.neighbor(B, 1);
	dijkstra.neighbor(C, 5);
	dijkstra.end_neighbors();

	{
		auto tmp = Graph::Dijkstra<Ln::NodeId>(dijkstra).finalize();
		assert(tmp.size() == 3);
		assert(tmp.find(A) != tmp.end());
		assert(tmp.find(B) != tmp.end());
		assert(tmp.find(C) != tmp.end());
		assert(tmp[A]->data.second == 0);
		assert(tmp[B]->data.second == 1);
		assert(tmp[C]->data.second == 5);
		assert(!tmp[A]->parent);
		assert(*tmp[B]->parent->data.first == A);
		assert(*tmp[C]->parent->data.first == A);
	}

	/* B is nearer, so it should come first.  */
	assert(dijkstra.current());
	assert(*dijkstra.current() == B);
	/* Give its neighbors.  */
	dijkstra.neighbor(A, 1);
	dijkstra.neighbor(C, 1);
	dijkstra.end_neighbors();

	{
		auto tmp = Graph::Dijkstra<Ln::NodeId>(dijkstra).finalize();
		assert(tmp.size() == 3);
		assert(tmp.find(A) != tmp.end());
		assert(tmp.find(B) != tmp.end());
		assert(tmp.find(C) != tmp.end());
		assert(tmp[A]->data.second == 0);
		assert(tmp[B]->data.second == 1);
		/* C should now be reached by route A->B->C.  */
		assert(tmp[C]->data.second == 2);
		assert(!tmp[A]->parent);
		assert(*tmp[B]->parent->data.first == A);
		assert(*tmp[C]->parent->data.first == B);
	}

	/* C should now be what is queried.  */
	assert(dijkstra.current());
	assert(*dijkstra.current() == C);
	/* Give its neighbors.  */
	dijkstra.neighbor(A, 5);
	dijkstra.neighbor(B, 1);
	dijkstra.end_neighbors();

	/* Same result as above.  */
	{
		auto tmp = Graph::Dijkstra<Ln::NodeId>(dijkstra).finalize();
		assert(tmp.size() == 3);
		assert(tmp.find(A) != tmp.end());
		assert(tmp.find(B) != tmp.end());
		assert(tmp.find(C) != tmp.end());
		assert(tmp[A]->data.second == 0);
		assert(tmp[B]->data.second == 1);
		assert(tmp[C]->data.second == 2);
		assert(!tmp[A]->parent);
		assert(*tmp[B]->parent->data.first == A);
		assert(*tmp[C]->parent->data.first == B);
	}

	/* Dijkstra should have completed.  */
	assert(!dijkstra.current());

	return 0;
}
