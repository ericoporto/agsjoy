CXX = g++
CXXFLAGS = -g -O2 -Wall -fPIC -I/usr/include/SDL2 -D_REENTRANT  
LD = g++
LDFLAGS = -L/usr/lib/x86_64-linux-gnu -lSDL2
DEFINES = 

all: libagsjoy.so

clean:
	rm -f libagsjoy.so *.o test

libagsjoy.so: agsjoy.o
	$(LD) $(LDFLAGS) -shared -o $@ $^

libagsjoy32.so: agsjoy32.o
	$(LD) $(LDFLAGS) -shared -m32 -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEFINES) -o $@ -c $^

%32.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEFINES) -m32 -o agsjoy32.o -c $^
