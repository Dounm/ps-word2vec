TARGET = output/psw2v
CPPS = $(wildcard src/*.cpp)
OBJS = $(patsubst src/%.cpp, output/%.o, $(CPPS))
DLOG_PATH = ./dlog
PSLITE_PATH = ./ps-lite
BOOST_PATH = ../../Install/boost_1_63_0/


CXX = g++
CXXFLAGS = -g \
          -O3 \
          -pipe \
          -Wextra \
          -Wall \
          -Wno-parentheses \
          -Wno-literal-suffix \
          -Wno-unused-parameter \
          -Wno-unused-local-typedefs \
          -fPIC \
          -std=c++11
INCPATH = -I./include \
        -I$(PSLITE_PATH)/include \
        -I$(DLOG_PATH)/output/include \
        -I$(BOOST_PATH)/boost
LDFLAGS = $(PSLITE_PATH)/build/libps.a \
        -L$(PSLITE_PATH)/deps/lib \
        -lprotobuf-lite -lzmq \
        $(DLOG_PATH)/output/lib/libdlog.a \
        -pthread 


.PHONY: all clean debug

all: $(TARGET)
	@echo "make all done"

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

output/%.o: src/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCPATH) -c $< -o $@

clean:
	rm -f *.o
	rm -f psw2v
	rm -rf output

debug:
	$(CXX) -v
	@echo $(OBJS)
