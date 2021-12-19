#!/usr/bin/python3

from urllib import parse
import paho.mqtt.client as mqtt
import random
import time
import argparse

import signal
import sys


# Handle SIGINt properly
def signal_handler(sig, frame):
    print("Client terminated with Ctrl+C")
    sys.exit(0)


signal.signal(signal.SIGINT, signal_handler)
parser = argparse.ArgumentParser()
parser.add_argument('-v', action='store_true')
flags = parser.parse_args()

if False:
    # The callback for when the client receives a CONNACK response from the server.
    def on_connect(client, userdata, flags, rc):
        print("Connected with result code " + str(rc))

        # Subscribing in on_connect() means that if we lose the connection and
        # reconnect then subscriptions will be renewed.
        client.subscribe("$SYS/#")

    # The callback for when a PUBLISH message is received from the server.
    def on_message(client, userdata, msg):
        print(msg.topic + " " + str(msg.payload))

    # Create the MQTT Client and set callbacks
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect("mqtt.eclipseprojects.io", 1883, 60)

    # Blocking call that processes network traffic, dispatches callbacks and
    # handles reconnecting.
    # Other loop*() functions are available that give a threaded interface and a
    # manual interface.
    client.loop_forever()

# mqtt_client Class


class mqtt_client:
    '''mqtt Client wrapper'''

    def __init__(self, on_connect, on_message):
        self.client = mqtt.Client()
        self.client.on_connect = on_connect
        self.client.on_message = on_message

        self.keep_alive = 60

    def set_log(self, mode):
        if (isinstance(mode, bool)):
            self.client.on_log = self.__log_callback if (mode) else None

    def connect(self, host="localhost", port=1883, keep_alive=60):
        '''
        Connect the client to the provided broker.
        '''

        # Set up client
        self.keep_alive = keep_alive

        # Attempt to connection
        try:
            self.client.connect(host, port, keep_alive)
        except Exception as e:
            print("mqtt_client.connect: " + str(e) +
                  " (" + host + " : " + str(port) + ")")
            exit(e.args[0])

        # Process connection event
        self.loop()

    def subscribe(self, topic):
        # topic may be either a single string or a list of strings
        return self.client.subscribe(topic)

    def unsubscribe(self, topic):
        return self.client.unsubscribe(topic)

    def publish(self, topic, payload=None, qos=0, retain=False):
        '''
        Send a message from client to the connected broker.
        '''
        self.client.publish(topic, payload, qos, retain)

    def loop(self, timeout=60):
        '''Process network events'''

        # As per the loop() documention, timeout must not exceed the
        # client's keepalive value or regular disconnects may occur
        if (timeout > self.keep_alive):
            timeout = self.keep_alive

        self.client.loop(timeout)

    def __log_callback(self, client, userdata, level, buf):
        print(buf)


def on_connect(client, userdata, flags, rc):
    if (rc):
        print("Connection failed with code: " + str(rc))
    else:
        print("Successfully connected to " +
              client._host + ":" + str(client._port))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    # client.subscribe("$SYS/#")

# The callback for when a PUBLISH message is received from the server.


def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))


# Create a client instance
client = mqtt_client(on_connect, on_message)

# Connect to a broker
client.connect("127.0.1.1")
client.set_log(True if flags.v else False)

while True:
    sleep_time = random.randint(1, 30)
    sleep_time = 2
    rand = random.randint(1, 9)
    print("sleep: " + str(sleep_time))
    time.sleep(sleep_time)
    client.publish("counter", rand)
    print("publish: " + str(rand))
    client.loop()
