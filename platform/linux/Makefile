all: idagetssids idasetwifi idasetwifitest

HexConverter.o: HexConverter.h
	g++ -std=c++14 -c -o HexConverter.o HexConverter.cpp

idasetwifi.o: idasetwifi.cpp
	g++ -std=c++14 -c -o idasetwifi.o idasetwifi.cpp

idagetssids: idagetssids.cpp
	g++ -std=c++14 -o idagetssids idagetssids.cpp
	./setuid.sh idagetssids

idasetwifi: idasetwifi.o HexConverter.o
	g++ -o idasetwifi -lm idasetwifi.o HexConverter.o
	./setuid.sh idasetwifi

idasetwifitest: idasetwifitest.cpp
	g++ -std=c++14 -o idasetwifitest idasetwifitest.cpp

clean:
	rm -f *.o idagetssids idasetwifi idasetwifitest
