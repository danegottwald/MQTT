
#include "mqtt_server.hpp"

#include <iostream>

typedef mqtt::server<>::endpoint_t              client_endpoint;
typedef std::shared_ptr<client_endpoint>        sptr_client_endpoint;

namespace wtec {

// Constructor
mqtt_server::mqtt_server(uint16_t port = 1883) {
    mqtt::setup_log();

    // Construct object in-place with parameters (TCP/IPv4 : port)
    instance.emplace(
        boost::asio::ip::tcp::endpoint(
            boost::asio::ip::tcp::v4(),
            port
        ),
        io_context
    );

    // Set all relevant handlers
    set_handlers();
}

// Destructor
mqtt_server::~mqtt_server() {
    io_context.stop();
    instance->close();
}

// Set the wqtt_server to a listen state
void mqtt_server::listen() {
    // Initialize timer
    timer.emplace(io_context, 180);

    // Set up timer callback
    auto loop = 
    [&, &data = atomic_data.value()](const boost::system::error_code&) {
        std::cout << "[" << timer->get_local_time() << "] Tick: "
                  << timer->get_count() << std::endl;
        std::cout << "\tClients: " << data.clients << std::endl;
        std::cout << "\tSum    : " << data.count_sum << std::endl;
        std::cout << "\tAverage: " 
                << ((data.clients) ? (data.count_sum / float(data.clients)) : 0)
                << std::endl;

        // Refresh interval using the last time
        timer->refresh_interval();

        // Reset data between interval
        data.clients = data.count_sum = 0;

        // Refresh timer
        timer->async_wait(timer->get_func());
    };
    timer->async_wait(loop);

    // Set MQTT server to listen for connection
    instance->listen();

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

    // Set accept handler for server 'instance'
    instance->set_accept_handler(
        [this](sptr_client_endpoint sptr_ep) {
            auto& ep = *sptr_ep;
            //std::cout << "[server]: accept client" << std::endl;

            // Start session with connected client
            ep.start_session(sptr_ep);

            // ERROR
            // PRESEND, before any response back to client
            // CONNECT, connect to client
            // PUBLISH, client send publish msg
            // SUBSCRIBE, client send subscribe msg
            // UNSUBSCRIBE, client send unsubscribe msg
            // DISCONNECT, disconnect from client
            // CLOSE, close client

            // ERROR
            ep.set_error_handler(
                [&ep](mqtt::error_code ec){
                    std::cout << ec.message() << std::endl;
                    ep.disconnect(mqtt::v5::disconnect_reason_code::unspecified_error);
            });

            // PRESEND
            ep.set_pre_send_handler([]() {
                //std::cout << "[server] pre send" << std::endl;
            });

            // CONNECT
            ep.set_connect_handler(
                [&ep](mqtt::buffer client_id,
                    mqtt::optional<mqtt::buffer> username,
                    mqtt::optional<mqtt::buffer> password,
                    mqtt::optional<mqtt::will> will,
                    bool clean_session,
                    std::uint16_t keep_alive) {
                        //std::cout << "[server] send connect" << std::endl;
                        // Acknowledge connection with client
                        ep.connack(false, mqtt::connect_return_code::accepted);
                        return true;
                }
            );

            typedef std::remove_reference_t<decltype(ep)>::packet_id_t
                packet_id_t;

            // PUBLISH
            ep.set_publish_handler(
                [&data = atomic_data.value()](
                    mqtt::optional<packet_id_t> packet_id,
                    mqtt::publish_options pubopts,
                    mqtt::buffer topic_name,
                    mqtt::buffer contents) {
                        //std::cout << "[client] publish " << contents << std::endl;
                        //std::cout << "[client] topic_name: " << topic_name << std::endl;
                        //std::cout << "[client] contents: " << contents << std::endl;
                        // Increment client publishes and sum count data
                        data.clients++;
                        data.count_sum += boost::lexical_cast<boost::uint64_t>(contents);
                        return true;
                }
            );

            // SUBSCRIBE
            ep.set_subscribe_handler(
                [] (packet_id_t packet_id, 
                    std::vector<mqtt::subscribe_entry> entries) {
                        //std::cout << "[client] subscribe" << std::endl;
                        return true;
                }
            );

            // UNSUBSCRIBE
            ep.set_unsubscribe_handler(
                [] (packet_id_t packet_id,
                    std::vector<mqtt::unsubscribe_entry> entries) {
                        //std::cout << "[client] unsubscribe" << std::endl;
                        return true;
                }
            );

            // DISCONNECT
            ep.set_disconnect_handler(
                [&ep]() {
                    //std::cout << "[server] send disconnect" << std::endl;
                    ep.disconnect(mqtt::v5::disconnect_reason_code::normal_disconnection);
                }
            );

            // CLOSE
            ep.set_close_handler(
                [this]() {
                    //timer.cancel();

                    //std::cout << "[server] close client" << std::endl;
                }
            );
        }
    );
}

}

