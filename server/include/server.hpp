
#include <iostream>
#include <iomanip>
#include <set>

#include <mqtt_server_cpp.hpp>
#include <mqtt/setup_log.hpp>

#include <boost/atomic.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

class mqtt_server {
    private:
        // Server Instance (boost::optional to avoid construction)
        boost::optional<mqtt::server<>> instance;

        // IO Context contains state to run event loops
        boost::asio::io_context io_context;

        // Atomic struct (thread safe to avoid data race)
        // <# client publishes, total sum count>
        // https://www.boost.org/doc/libs/1_78_0/doc/html/atomic/interface.html
        struct count_data {
            boost::uint16_t clients;
            boost::uint64_t count_sum;
        };
        boost::atomic<count_data> atomic_data {};

        // Private Helper Functions
        void set_handlers();

    public:
        // Constructors
        mqtt_server(std::uint16_t port_num);

        // Set server to a listening state for connecting clients
        void listen();

        // Getters
        inline std::uint16_t port() { return instance->port(); }

        // Operator Overloading
        mqtt_server(const mqtt_server&) = delete;
        mqtt_server& operator=(const mqtt_server&) = delete;

};

