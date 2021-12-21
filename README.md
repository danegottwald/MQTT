# Client (client/)
Requires the following:
* [Mosquitto](https://mosquitto.org/download/)
```bash
sudo apt-get install mosquitto mosquitto-clients
```

To Run:
```bash
py client.py
```

# Server (server/)
Requires the following:
* [Boost C++](https://www.boost.org/users/download/)
```bash
sudo apt-get install libboost-all-dev
```

To Run:
```bash
# Run make
make

# Run application
./server [port]
```
