CXX=g++
CXXFLAGS=-std=c++11 -I../include

TARGET: message_insert_test.cpp.out

%.cpp.out: %.cpp
	$(CXX) $< -o $@ $(CXXFLAGS)

clean:
	rm -f $(TARGET)
