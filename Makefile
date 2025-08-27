CC = g++
CFLAGS = --std=c++17 -Wall -Werror -pedantic -g
LIB = -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system -lboost_unit_test_framework
SOURCE = main.cpp
OBJECTS = 
PROGRAM = ps7

.PHONY: all clean lint

all: $(PROGRAM)

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c $< $(INCLUDEDIR)

$(PROGRAM): main.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBDIR) $(LIB) 

$(PROGRAM).a : $(OBJECTS)
	ar rcs $@ $^ 

clean:
	rm *.o $(PROGRAM) $(PROGRAM).a 

lint:
	cpplint *.cpp *.hpp
