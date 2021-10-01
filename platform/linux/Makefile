all: idagetssids idasetwifi idasetwifitest

HexConverter.o: HexConverter.cpp
	g++ -std=c++14 -c -o HexConverter.o HexConverter.cpp

AsciiEncoder.o: AsciiEncoder.cpp
	g++ -std=c++14 -c -o AsciiEncoder.o AsciiEncoder.cpp

Pipe.o: Pipe.cpp
	g++ -std=c++14 -c -o Pipe.o Pipe.cpp

Process.o: Process.cpp
	g++ -std=c++14 -c -o Process.o Process.cpp

idasetwifi.o: idasetwifi.cpp
	g++ -std=c++14 -c -o idasetwifi.o idasetwifi.cpp

idagetssids.o: idagetssids.cpp
	g++ -std=c++14 -c -o idagetssids.o idagetssids.cpp

idasetwifitest.o: idasetwifitest.cpp
	g++ -std=c++14 -c -o idasetwifitest.o idasetwifitest.cpp

idasetwifi: idasetwifi.o AsciiEncoder.o HexConverter.o Pipe.o Process.o
	g++ -o idasetwifi -lm idasetwifi.o AsciiEncoder.o HexConverter.o Pipe.o Process.o
	./setuid.sh idasetwifi

idagetssids: idagetssids.o AsciiEncoder.o Pipe.o Process.o
	g++ -o idagetssids -lm idagetssids.o AsciiEncoder.o Pipe.o Process.o
	./setuid.sh idagetssids

idasetwifitest: idasetwifitest.o AsciiEncoder.o Pipe.o Process.o
	g++ -o idasetwifitest -lm idasetwifitest.o AsciiEncoder.o Pipe.o Process.o

clean:
	rm -f *.o idagetssids idasetwifi idasetwifitest

