
#include "server.hpp"

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

#include <pthread.h>

inline void close_proc(std::set<con_sp_t>& cons, mi_sub_con& subs, con_sp_t const& con) {
    cons.erase(con);
    
    auto& idx = subs.get<tag_con>();
    auto r = idx.equal_range(con);
    idx.erase(r.first, r.second);
}

// Constructor
mqtt_server::mqtt_server(uint16_t port_num = 1883) : port(port_num) {
    // Construct object in-place with parameters
    instance.emplace(
        boost::asio::ip::tcp::endpoint(
            boost::asio::ip::tcp::v4(),
            port
        ),
        io_context
    );

    // notable instance methods (methods for mqtt::server<>)
    // port() returns server port number
    // close() close server
    // ioc_accept() returns reference io context for acceptor
    // ioc_con() returns reference io context for connections
    // set_error_handler()
    // set_accept_handler()

    set_handlers();
}

void test(const boost::system::error_code& /*e*/,
          boost::asio::steady_timer* t, 
          std::uint64_t* count) {
    std::cout << boost::this_thread::get_id() << " tick" << std::endl;
    // Reschedule the timer for 1 second in the future:
    (*t).expires_at((*t).expires_at() + boost::asio::chrono::seconds(3));
    // Posts the timer event
    (*t).async_wait(boost::bind(test,
        boost::asio::placeholders::error, t, count));
}

// Set the wqtt_server to a listen state
void mqtt_server::listen() {
    is_listening = true;
    instance->listen();
    std::cout << "after listen" << std::endl;

    //boost::thread timer_thd(&timer);
    //timer_thd.join();

    // Create 3 minute timer
    boost::asio::steady_timer t(io_context, 
        boost::asio::chrono::seconds(3));

    // 
    t.async_wait(boost::bind(test,
        boost::asio::placeholders::error, &t, &count));

    // Blocks here for connections
    io_context.run();
}

// Set all the necessary callbacks that are required for the mqtt_server
void mqtt_server::set_handlers() {
    // Set error handler for server 'instance'
    instance->set_error_handler(
        [](mqtt::error_code ec) {
            std::cerr << "wqtt_server: [Errno " << ec.value() << "] " 
                      << ec.message() << std::endl;
        }
    );

    // Set accept handler for server 'instance'
    instance->set_accept_handler(
        [&c = count, &con = connections, &sub = subscribers](con_sp_t sp_ep) {
            auto& ep = *sp_ep;
            std::weak_ptr<con_t> wp(sp_ep);

            using packet_id_t = typename std::remove_reference_t<decltype(ep)>::packet_id_t;
            std::cout << "debug: accept" << std::endl;

            // Pass sp_ep to keep lifetime.
            // It makes sure wp.lock() never return nullptr in the handlers below
            // including close_handler and error_handler.
            ep.start_session(std::move(sp_ep));

            // CLOSE handler
            ep.set_close_handler(
                [&con, &sub, wp](){
                    std::cout << "[server] closed." << std::endl;
                    auto sp = wp.lock();
                    BOOST_ASSERT(sp);
                    close_proc(con, sub, sp);
                }
            );

            // ERROR handler
            ep.set_error_handler(
                [&con, &sub, wp](mqtt::error_code ec){
                    std::cout << "[server] error: " << ec.message() 
                        << std::endl;
                    auto sp = wp.lock();
                    BOOST_ASSERT(sp);
                    close_proc(con, sub, sp);
                }
            );

            // CONNECT handler
            ep.set_connect_handler(
                [&con, wp]
                (mqtt::buffer client_id,
                 mqtt::optional<mqtt::buffer> username,
                 mqtt::optional<mqtt::buffer> password,
                 mqtt::optional<mqtt::will>,
                 bool clean_session,
                 std::uint16_t keep_alive) {
                    using namespace mqtt::literals;
                    std::cout << "[server] client_id    : " << client_id << std::endl;
                    std::cout << "[server] username     : " << (username ? username.value() : "none"_mb) << std::endl;
                    std::cout << "[server] password     : " << (password ? password.value() : "none"_mb) << std::endl;
                    std::cout << "[server] clean_session: " << std::boolalpha << clean_session << std::endl;
                    std::cout << "[server] keep_alive   : " << keep_alive << std::endl;
                    auto sp = wp.lock();
                    BOOST_ASSERT(sp);
                    con.insert(sp);
                    sp->connack(false, mqtt::connect_return_code::accepted);
                    return true;
                }
            );

            // DISCONNECT handler
            ep.set_disconnect_handler(
                [&con, &sub, wp](){
                    std::cout << "[server] disconnect received." 
                        << std::endl;
                    auto sp = wp.lock();
                    BOOST_ASSERT(sp);
                    close_proc(con, sub, sp);
                }
            );

            // PUBLISH handler
            ep.set_publish_handler(
                [&c, &sub]
                (mqtt::optional<packet_id_t> packet_id,
                 mqtt::publish_options pubopts,
                 mqtt::buffer topic_name,
                 mqtt::buffer contents){
                    std::cout << "[server] publish received."
                              << " dup: "    << pubopts.get_dup()
                              << " qos: "    << pubopts.get_qos()
                              << " retain: " << pubopts.get_retain() << std::endl;
                    if (packet_id)
                        std::cout << "[server] packet_id: " << *packet_id << std::endl;
                    std::cout << "[server] topic_name: " << topic_name << std::endl;
                    std::cout << "[server] contents: " << contents << std::endl;
                    auto const& idx = sub.get<tag_topic>();
                    auto r = idx.equal_range(topic_name);
                    c++;
                    std::cout << c << std::endl;
                    for (; r.first != r.second; ++r.first) {
                        r.first->con->publish(
                            topic_name,
                            contents,
                            std::min(r.first->qos_value, pubopts.get_qos())
                        );
                    }
                    return true;
                }
            );

            // SUBSCRIBE handler
            ep.set_subscribe_handler(
                [&sub, wp]
                (packet_id_t packet_id,
                 std::vector<mqtt::subscribe_entry> entries) {
                    std::cout << "[server] subscribe received. packet_id: " << packet_id << std::endl;
                    std::vector<mqtt::suback_return_code> res;
                    res.reserve(entries.size());
                    auto sp = wp.lock();
                    BOOST_ASSERT(sp);
                    for (auto const& e : entries) {
                        std::cout << "[server] topic_filter: " << e.topic_filter  << " qos: " << e.subopts.get_qos() << std::endl;
                        res.emplace_back(mqtt::qos_to_suback_return_code(e.subopts.get_qos()));
                        sub.emplace(std::move(e.topic_filter), sp, e.subopts.get_qos());
                    }
                    sp->suback(packet_id, res);
                    return true;
                }
            );

            // UNSUBSCRIBE handler
            ep.set_unsubscribe_handler(
                [&sub, wp]
                (packet_id_t packet_id,
                 std::vector<mqtt::unsubscribe_entry> entries) {
                    std::cout << "[server] unsubscribe received. packet_id: " << packet_id << std::endl;
                    for (auto const& e : entries) {
                        sub.erase(e.topic_filter);
                    }
                    auto sp = wp.lock();
                    BOOST_ASSERT(sp);
                    sp->unsuback(packet_id);
                    return true;
                }
            );
        }
    );
}

