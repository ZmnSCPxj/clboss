#undef NDEBUG
#include"Boss/random_engine.hpp"
#include"Stats/ReservoirSampler.hpp"
#include<assert.h>
#include<string>

int main() {
	{
		auto smp = Stats::ReservoirSampler<std::string>();
		smp.add("a", 1, Boss::random_engine);
		smp.add("b", 1, Boss::random_engine);
		smp.add("c", 1, Boss::random_engine);
		smp.add("d", 1, Boss::random_engine);

		assert(smp.get().size() == 1);
		auto sels = std::move(smp).finalize();
		assert(sels.size() == 1);
		assert( sels[0] == "a"
		     || sels[0] == "b"
		     || sels[0] == "c"
		     || sels[0] == "d"
		      );
	}

	{
		auto smp = Stats::ReservoirSampler<std::string>(3);
		smp.add("a", 1, Boss::random_engine);
		assert(smp.get().size() == 1);
		assert(smp.get()[0] == "a");
		smp.add("b", 1, Boss::random_engine);
		smp.add("c", 1, Boss::random_engine);
		smp.add("d", 1, Boss::random_engine);
		smp.add("e", 1, Boss::random_engine);
		smp.add("f", 1, Boss::random_engine);

		assert(smp.get().size() == 3);
	}

	return 0;
}
