
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

typedef mqtt::server<>::endpoint_t client_endpoint;
typedef std::shared_ptr<client_endpoint> sptr_client_endpoint;

struct sub_con {
    sub_con(mqtt::buffer topic, sptr_client_endpoint con, mqtt::qos qos_value)
        :topic(std::move(topic)), con(std::move(con)), qos_value(qos_value) {}
    mqtt::buffer topic;
    sptr_client_endpoint con;
    mqtt::qos qos_value;
};

struct tag_topic {};
struct tag_con {};

typedef boost::multi_index::multi_index_container<
    sub_con,
    boost::multi_index::indexed_by<
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<tag_topic>,
            BOOST_MULTI_INDEX_MEMBER(sub_con, mqtt::buffer, topic)
        >,
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<tag_con>,
            BOOST_MULTI_INDEX_MEMBER(sub_con, sptr_client_endpoint, con)
        >
    >
> mi_sub_con;

class mqtt_server {
    private:
        // Server Instance (boost::optional to avoid construction)
        boost::optional<mqtt::server<>> instance;

        // IO Context contains state to run event loops
        boost::asio::io_context io_context;

        std::set<sptr_client_endpoint> connections;
        mi_sub_con subscribers;

        // Atomic struct (thread safe to avoid data race)
        // <# client publishes, total sum count>
        // https://www.boost.org/doc/libs/1_78_0/doc/html/atomic/interface.html
        struct client_data {
            boost::uint16_t clients;
            boost::uint64_t count_sum;
        };
        boost::atomic<client_data> atomic_data {};

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

