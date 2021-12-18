
#include <iostream>

#include "server.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << argv[0] << " [port]" << std::endl;
        return -1;
    }

    // Literal text form to uint16_t
    std::uint16_t port = boost::lexical_cast<std::uint16_t>(argv[1]);

    mqtt_server server(port);
    server.listen();

    //mqtt::setup_log();

    return 0;
}

// ideas for sum of last 3 minutes of data
// 1. create main thread that checks every three minutes and computes 
//      average, server listen on another thread that locks when inc's
// 2. 

// Note unrelated: YouTube plugin that allows you to bundle subscribed
// channels into folders, (i.e. vlogs, gaming, etc)

// MQTT (MQ Telemetry Transport, MQ refers to IBM's series of products)
//      Uses Publish/Subscribe model
//          Publishers publish messages
//          Subscribers receive messages if they have subscribed to the 
//              topic/channel
//      Publishers and subscribers do not directly communicate with 
//          each other, but to a central server/broker that mediates
//          in between.
//      The server/broker filters the incoming messages and distrubes
//          them to the subscribers accordingly.
//      Publishers and subscribers are also referred to as 'clients'

