CXXFLAGS=-gdwarf-4 -Wall -Wextra -pedantic -O0 -MD
LDFLAGS=-lpthread `pkg-config --static --libs glfw3` -lGL
default: libbase.a example_test


libBase_SRC=\
	assert.cpp\
	callstack.cpp\
	debugger.cpp\
	fpe_ctrl.cpp\
	scheduler.cpp\
	logging.cpp\
	remotery.cpp\
	window.cpp\
	input.cpp\
	imgui/imgui.cpp\

libBase_OBJ=$(libBase_SRC:.cpp=.o)

libbase.a: $(libBase_SRC) $(libBase_OBJ)
	ar -rcs libbase.a $(libBase_OBJ)

example_test: libbase.a example/test.cpp
	$(CXX) $(CXXFLAGS) -I. -o example_test example/test.cpp -L. -lbase $(LDFLAGS)

clean:
	-rm -f $(libBase_OBJ) $(libBase_OBJ:.o=.d) libbase.a

-include $(libBase_OBJ:.o=.d)