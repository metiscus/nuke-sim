CXXFLAGS=-gdwarf-4 -Wall -Wextra -pedantic -O0 -MD -Iimgui -I.
LDFLAGS=-lpthread `pkg-config --static --libs glfw3` -lGL -lboost_system -lboost_filesystem
default: libbase.a example_test

libBase_SRC=\
	assert_macros.cpp\
	callstack.cpp\
	debugger.cpp\
	fpe_ctrl.cpp\
	scheduler.cpp\
	logging.cpp\
	remotery.cpp\
	window.cpp\
	input.cpp\
	imgui/imgui.cpp\
	resourcemanager.cpp\
	imageresource.cpp\
	private/stb_image.cpp\


remotery.o: remotery.cpp
	$(CXX) -c -gdwarf-4 -Wall -Wextra -O0 -MD -o remotery.o remotery.cpp

libBase_OBJ=$(libBase_SRC:.cpp=.o)

libbase.a: $(libBase_SRC) $(libBase_OBJ)
	ar -rcs libbase.a $(libBase_OBJ)

example_SRC=\
	example/stb/stb_image.cpp\
	example/stb/stb_truetype.cpp\
	example/renderer_gl.cpp\
	example/test.cpp\

example_OBJ=$(example_SRC:.cpp=.o)

example_test: libbase.a $(example_OBJ) $(example_SRC)
	$(CXX) $(CXXFLAGS) -Iexample -o example_test $(example_OBJ) -L. -lbase $(LDFLAGS)

clean:
	-rm -f $(libBase_OBJ) $(libBase_OBJ:.o=.d) libbase.a
	-rm -f $(example_OBJ) $(example_OBJ:.o=.d) example_test

-include $(libBase_OBJ:.o=.d)