// This file contains declarations for the main Engine class. You will
// need to add declarations to this file as you develop your Engine.

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <chrono>
#include <memory>

#include "io.hpp"
#include "atomic_map.hpp"
#include "order_book.hpp"
#include "order.hpp"

struct Engine
{
public:
	Engine();
	void accept(ClientConnection conn);
	std::shared_ptr<OrderBook> GetOrderBook(instrument_id_t instrument);

private:
	void connection_thread(ClientConnection conn);
	AtomicMap<instrument_id_t, std::shared_ptr<OrderBook>> instruments;
};

inline std::chrono::microseconds::rep getCurrentTimestamp() noexcept
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

#endif
