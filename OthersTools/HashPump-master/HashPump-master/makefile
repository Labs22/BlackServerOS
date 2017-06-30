CXX?=g++
CXXFLAGS+=-c -Wall
INSTALLLOCATION=/usr/bin/

all: hashpump

hashpump: main.o Extender.o MD4ex.o MD5ex.o SHA1.o SHA256.o SHA512ex.o CRC32ex.o
	$(CXX) main.o Extender.o MD4ex.o MD5ex.o SHA1.o SHA256.o SHA512ex.o CRC32ex.o -lcrypto -o hashpump

main.o:
	$(CXX) $(CXXFLAGS) main.cpp

Extender.o:
	$(CXX) $(CXXFLAGS) Extender.cpp

CRC32ex.o:
	$(CXX) $(CXXFLAGS) CRC32ex.cpp

MD4ex.o:
	$(CXX) $(CXXFLAGS) MD4ex.cpp

MD5ex.o:
	$(CXX) $(CXXFLAGS) MD5ex.cpp

SHA1.o:
	$(CXX) $(CXXFLAGS) SHA1.cpp

SHA256.o:
	$(CXX) $(CXXFLAGS) SHA256.cpp

SHA512ex.o:
	$(CXX) $(CXXFLAGS) SHA512ex.cpp

clean:
	rm *.o hashpump

install: hashpump
	cp hashpump $(INSTALLLOCATION)
