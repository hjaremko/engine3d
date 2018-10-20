CC = g++
CFLAGS = -std=c++14 -O2 -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Wmisleading-indentation -Wnull-dereference
LIBS = -lsfml-system -lsfml-window -lsfml-graphics
SFMLPATH = C:/SFML

all:
# $(CC) $(CFLAGS) -c main.cpp $< -I$(SFMLPATH)/include -L$(SFMLPATH)/lib $(LIBS) -o engine3d.exe
	$(CC) $(CFLAGS) main.cpp -I$(SFMLPATH)/include -L$(SFMLPATH)/lib $(LIBS) -o engine3d
# g++ main.cpp -o engine32

