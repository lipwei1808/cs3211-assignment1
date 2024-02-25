// This file contains declarations for the main Engine class. You will
// need to add declarations to this file as you develop your Engine.

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <chrono>

#include "order_book.hpp"
#include "io.hpp"

struct Engine
{
public:
	Engine();
	void accept(ClientConnection conn);

private:
	void connection_thread(ClientConnection conn);
	OrderBook order_book;
};

inline std::chrono::microseconds::rep getCurrentTimestamp() noexcept
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

#endif
