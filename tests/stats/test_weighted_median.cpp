#undef NDEBUG
#include"Ln/Amount.hpp"
#include"Stats/WeightedMedian.hpp"
#include<assert.h>

int main() {
	auto flag = false;
	try {
		auto md = Stats::WeightedMedian<double, double>();
		auto result = std::move(md).finalize();
		(void) result;
		flag = false;
	} catch (Stats::NoSamples const&) {
		flag = true;
	}
	assert(flag);

	{
		auto md = Stats::WeightedMedian<double, double>();
		md.add(1.0, 500.0);
		auto result = std::move(md).finalize();
		assert(result == 1.0);
	}

	/* Check normal median.  */
	{
		auto md = Stats::WeightedMedian<double, double>();
		md.add(1.0, 500.0);
		md.add(2.0, 500.0);
		md.add(3.0, 500.0);
		auto result = std::move(md).finalize();
		assert(result == 2.0);
	}

	/* Check weighting.  */
	{
		auto md = Stats::WeightedMedian<double, double>();
		md.add(1.0, 500.0);
		md.add(2.0, 1.0);
		md.add(3.0, 1.0);
		auto result = std::move(md).finalize();
		assert(result == 1.0);
	}
	{
		auto md = Stats::WeightedMedian<double, double>();
		md.add(1.0, 1.0);
		md.add(2.0, 1.0);
		md.add(3.0, 3.0);
		auto result = std::move(md).finalize();
		assert(result == 3.0);
	}

	/* Check different types.  */
	{
		auto md = Stats::WeightedMedian<double, Ln::Amount>();
		md.add(4.0, Ln::Amount::sat(1));
		md.add(3.0, Ln::Amount::sat(3000));
		md.add(5.0, Ln::Amount::sat(1));
		md.add(1.0, Ln::Amount::sat(1));
		auto result = std::move(md).finalize();
		assert(result == 3.0);
	}

	return 0;
}
