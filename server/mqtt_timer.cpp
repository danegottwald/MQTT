
#include "mqtt_timer.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace wtec {

// Constructor
mqtt_timer::mqtt_timer(boost::asio::io_context &ioc, boost::uint32_t seconds)
    : interval(boost::asio::chrono::seconds(seconds)),
      interval_seconds(seconds) {
           timer.emplace(ioc, interval);
}

// Destructor
mqtt_timer::~mqtt_timer() {
    timer->cancel();
}

// Reuse interval
void mqtt_timer::refresh_interval() {
    timer->expires_after(interval);
}

// Set new expire time, overwrites interval
void mqtt_timer::expire_from_now(boost::uint32_t seconds) {
    interval_seconds = seconds;
    interval = boost::asio::chrono::seconds(seconds);
    timer->expires_after(interval);
}

// Reuse last func
void mqtt_timer::async_wait() {
    count++;
    timer->async_wait(func);
}

// Supply new function to async_wait on
void mqtt_timer::async_wait(const func_ptr &l) {
    count++;
    func = l;
    timer->async_wait(boost::bind(l, boost::asio::placeholders::error));
}

// Wrapper around getting local time
boost::posix_time::ptime mqtt_timer::get_local_time() {
    return boost::posix_time::second_clock::local_time();
}

}

