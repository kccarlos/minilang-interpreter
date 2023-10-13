CXX_SRCS = src/cpputil.cpp src/lexer.cpp src/parser2.cpp \
	src/main.cpp src/ast.cpp src/node_base.cpp src/node.cpp src/treeprint.cpp \
	src/location.cpp src/exceptions.cpp \
	src/interp.cpp src/value.cpp src/environment.cpp src/valrep.cpp src/function.cpp src/array.cpp src/string.cpp

CXX_OBJS = $(CXX_SRCS:%.cpp=%.o)

CXX = g++
CXXFLAGS = -g -Wall -std=c++17

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

all : minilang

minilang : $(CXX_OBJS)
	$(CXX) -o $@ $(CXX_OBJS)

clean :
	rm -f src/*.o minilang depend.mak

depend :
	$(CXX) $(CXXFLAGS) -M $(CXX_SRCS) > depend.mak

depend.mak :
	touch $@

include depend.mak
