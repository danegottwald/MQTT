pip install paho-mqtt
    python mqtt library

paho mqtt python documentation
    https://www.eclipse.org/paho/index.php?page=clients/python/docs/index.php

g++

To run client
    python3 wtec_client.py


To run server
    ./wtec_server [--ip] [--port]

g++ -std=c++14 -Iinclude -c -o obj/server.o server.cpp
g++ -std=c++14 -Iinclude -o wtec_server obj/wtec_server.o obj/server.o -pthread
wtec_server successfully built

g++ -std=c++14 -Iinclude -c -o obj/wtec_server.o wtec_server.cpp
g++ -std=c++14 -Iinclude -c -o obj/server.o server.cpp
g++ -std=c++14 -Iinclude -o wtec_server obj/wtec_server.o obj/server.o -pthread
wtec_server successfully built