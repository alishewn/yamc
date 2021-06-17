
#ifndef YAMC_LATCH_HPP_
#define YAMC_LATCH_HPP_

#include <cassert>
#include <condition_variable>
#include <cstddef>
#include <limits>
#include <mutex>

/*
 * Latches in C++20 Standard Library
 *
 * - yamc::latch
 */
namespace yamc {

class latch {
	std::ptrdiff_t counter_;
	mutable std::condition_variable cv_;
	mutable std::mutex mtx_;

public:
	static constexpr ptrdiff_t(max)() noexcept { return (std::numeric_limits<ptrdiff_t>::max)(); }

	/*constexpr*/ explicit latch(std::ptrdiff_t expected) : counter_(expected) {
		assert(0 <= expected && expected < (max)());
	}

	~latch() = default;

	latch(const latch&) = delete;
	latch& operator=(const latch&) = delete;

	void count_down(std::ptrdiff_t update = 1) {
		std::lock_guard<decltype(mtx_)> lk(mtx_);
		assert(0 <= update && update <= counter_);
		counter_ -= update;
		if (counter_ == 0) { cv_.notify_all(); }
	}

	bool try_wait() const noexcept {
		std::lock_guard<decltype(mtx_)> lk(mtx_);
		// no spurious failure
		return (counter_ == 0);
	}

	void wait() const {
		std::unique_lock<decltype(mtx_)> lk(mtx_);
		while (counter_ != 0) { cv_.wait(lk); }
	}

	void arrive_and_wait(std::ptrdiff_t update = 1) {
		std::unique_lock<decltype(mtx_)> lk(mtx_);
		// equivalent to { count_down(update); wait(); }
		assert(0 <= update && update <= counter_);
		counter_ -= update;
		if (counter_ == 0) { cv_.notify_all(); }
		while (counter_ != 0) { cv_.wait(lk); }
	}
};

}    // namespace yamc

#endif
