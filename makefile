CXXFLAGS=-gdwarf-4 -Wall -Wextra -pedantic -O0 -MD

default: libbase.a


SRC=\
	assert.cpp\
	callstack.cpp\
	debugger.cpp\
	fpe_ctrl.cpp\
	sched.cpp\
	remotery.cpp

OBJ=$(SRC:.cpp=.o)

libbase.a: $(SRC) $(OBJ)


clean:
	-rm -f $(OBJ) $(OBJ:.o=.d) libbase.a

-include $(OBJ:.o=.d)