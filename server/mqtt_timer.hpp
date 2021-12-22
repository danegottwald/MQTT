
#include <boost/asio.hpp>

#include <boost/optional.hpp>
#include <boost/function.hpp>
#include <boost/bind/bind.hpp>

namespace wtec {

typedef boost::function<void(const boost::system::error_code&)> 
    func_ptr;

class mqtt_timer {
private:
    // Timer instance
    boost::optional<boost::asio::steady_timer> timer;

    // Interval relevance
    boost::uint32_t count = 0;
    boost::uint32_t interval_seconds;
    boost::asio::chrono::seconds interval;

    // Holds the function to call
    boost::function<void(const boost::system::error_code&)> func;

public:
    // Constructor/Destructor
    mqtt_timer(boost::asio::io_context &ioc, boost::uint32_t seconds);
    ~mqtt_timer();

    // Use to reuse last interval
    void refresh_interval();

    // Set new expire time, overwrites interval
    void expire_from_now(boost::uint32_t seconds);

    // Use to reuse last async_wait function
    void async_wait();

    // Supply new function to async_wait on
    void async_wait(const func_ptr &l);
    
    boost::posix_time::ptime get_local_time();
    inline boost::uint32_t get_count() { return count; }
    inline const func_ptr &get_func() { return func; }
};

}

