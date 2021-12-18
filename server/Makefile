CXX = g++
CXXFLAGS = -std=c++14 -I${INCLUDE_DIR}

TARGET = wtec_server

INCLUDE_DIR = include
OBJ_DIR = obj

SRCS = wtec_server.cpp server.cpp
OBJS = $(SRCS:%.cpp=${OBJ_DIR}/%.o)

LIBS = -lpthread

all: ${TARGET}
	@echo $(TARGET) successfully built

${TARGET}: ${shell mkdir -p ${OBJ_DIR}} ${OBJS}
	${CXX} ${CXXFLAGS} ${LIBS} -o $@ $^

${OBJ_DIR}/%.o: %.cpp
	${CXX} ${CXXFLAGS} -c $< -o $@

clean:
	rm -rf ${TARGET} ${OBJ_DIR}

.PHONY: all clean

