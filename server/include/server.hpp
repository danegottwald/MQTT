
#include <iostream>
#include <iomanip>
#include <set>

#include <mqtt_server_cpp.hpp>
#include <mqtt/setup_log.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

typedef mqtt::server<>::endpoint_t      con_t;
typedef std::shared_ptr<con_t>          con_sp_t;

struct sub_con {
    sub_con(mqtt::buffer topic, con_sp_t con, mqtt::qos qos_value)
        :topic(std::move(topic)), con(std::move(con)), qos_value(qos_value) {}
    mqtt::buffer topic;
    con_sp_t con;
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
            BOOST_MULTI_INDEX_MEMBER(sub_con, con_sp_t, con)
        >
    >
> mi_sub_con;

class mqtt_server {
    private:
        // Server Instance (boost::optional to avoid construction)
        boost::optional<mqtt::server<>> instance;

        // IO Context contains state to run event loops
        boost::asio::io_context io_context;

        std::uint16_t port;
        std::set<con_sp_t> connections;
        mi_sub_con subscribers;

        bool is_listening = false;

        //
        boost::tuple<std::uint16_t, std::uint64_t> data;
        std::uint64_t count = 0;

        void set_handlers();

    public:
        // Constructors
        mqtt_server(std::uint16_t port_num);

        // Set server to a listening state for connecting clients
        void listen();

        // Operator Overloading
        mqtt_server(const mqtt_server&) = delete;
        mqtt_server& operator=(const mqtt_server&) = delete;

};

