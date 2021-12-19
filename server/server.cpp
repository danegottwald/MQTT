
#include "server.hpp"

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

#include <pthread.h>

inline void close_proc(std::set<sptr_client_endpoint>& cons, 
                       mi_sub_con& subs, 
                       sptr_client_endpoint const& con) {
    cons.erase(con);
    
    auto& idx = subs.get<tag_con>();
    auto r = idx.equal_range(con);
    idx.erase(r.first, r.second);
}

// Constructor
mqtt_server::mqtt_server(uint16_t port = 1883) {
    // Construct object in-place with parameters (TCP/IPv4 : port)
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

// Set the wqtt_server to a listen state
void mqtt_server::listen() {
    instance->listen();

    // Create 3 minute timer
    auto interval = boost::asio::chrono::seconds(3);
    boost::asio::steady_timer timer(io_context, interval);

    boost::function<void(const boost::system::error_code&)> loop;
    loop = [&, &data = atomic_data.value()](const boost::system::error_code&) {
        std::cout << boost::this_thread::get_id() << " tick" << std::endl;
        std::cout << "clients: " << data.clients << std::endl;
        std::cout << "sum    : " << data.count_sum << std::endl;
        std::cout << "average: " 
                  << ((data.clients) ? (data.count_sum / data.clients) : 0)
                  << std::endl;

        // Set new expiration time
        timer.expires_at(timer.expires_at() + interval);

        // Reset data between interval
        data.clients = data.count_sum = 0;

        // Refresh timer
        timer.async_wait(loop);
    };

    timer.async_wait(boost::bind(loop, boost::asio::placeholders::error));

    // Blocks here for event loops
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

    int abc = 1;

    // Set accept handler for server 'instance'
    instance->set_accept_handler(
        [this](sptr_client_endpoint sptr_ep) {
            auto& ep = *sptr_ep;

            // typedef std::remove_reference_t<decltype(ep)>::packet_id_t
            //     packet_id_t;
            std::cout << "debug: accept" << std::endl;

            // Start session with connected client
            //ep.start_session(std::move(sptr_ep));
            ep.start_session(sptr_ep);

            // ERROR
            // CLOSE
            // CONNECT
            // DISCONNECT
            // PUBLISH
            // SUBSCRIBE
            // UNSUBSCRIBE

            // ERROR
            ep.set_error_handler([&](mqtt::error_code ec){
                // handle error
            });

            // CONNECT
            ep.set_connect_handler(
                [&ep](mqtt::buffer client_id,
                    mqtt::optional<mqtt::buffer> username,
                    mqtt::optional<mqtt::buffer> password,
                    mqtt::optional<mqtt::will>,
                    bool clean_session,
                    std::uint16_t keep_alive) {
                        // Acknowledge connection, accept
                        std::cout << "[client] connect" << std::endl;
                        ep.connack(false, mqtt::connect_return_code::accepted);
                        return true;
                }
            );

            // DISCONNECT
            ep.set_disconnect_handler(
                [&ep]() {
                    std::cout << "[client] disconnect" << std::endl;
                    ep.disconnect(mqtt::v5::disconnect_reason_code::normal_disconnection);
                }
            );

            typedef std::remove_reference_t<decltype(ep)>::packet_id_t
                packet_id_t;

            // PUBLISH
            ep.set_publish_handler(
                [&data = atomic_data.value()](mqtt::optional<packet_id_t> packet_id,
                    mqtt::publish_options pubopts,
                    mqtt::buffer topic_name,
                    mqtt::buffer contents) {
                        std::cout << "[client] publish" << std::endl;
                        // Increment client publish request
                        data.clients++;
                        // Sum client publish count data (lexical_cast str to int)
                        data.count_sum += boost::lexical_cast<boost::uint64_t>(contents);
                        std::cout << "[server] topic_name: " << topic_name << std::endl;
                        std::cout << "[server] contents: " << contents << std::endl;
                        return true;
                }
            );
        }
    );
}

