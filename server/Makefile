CXX = g++
CXXFLAGS = -std=c++14 -I${INCLUDE_DIR}

TARGET = server

INCLUDE_DIR = include
OBJ_DIR = obj

SRCS = server.cpp mqtt_server.cpp mqtt_timer.cpp
OBJS = $(SRCS:%.cpp=${OBJ_DIR}/%.o)

LIBS = -pthread

all: ${TARGET}
	@echo $(TARGET) successfully built

${TARGET}: ${shell mkdir -p ${OBJ_DIR}} ${OBJS}
	${CXX} ${CXXFLAGS} ${LIBS} -o $@ $^

${OBJ_DIR}/%.o: %.cpp
	${CXX} ${CXXFLAGS} -c $< -o $@

clean:
	rm -rf ${TARGET} ${OBJ_DIR}

.PHONY: all clean

