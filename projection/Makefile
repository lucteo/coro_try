
CXX=g++
CXXFLAGS=-std=c++20

all: .out/projection.naive .out/projection.cxxcoro .out/projection.switchedresume .out/projection.retcon

.out/projection.naive: main.cpp Makefile
	$(CXX) $(CXXFLAGS) -o .out/projection.naive main.cpp -DUSE_NAIVE

.out/projection.cxxcoro: main.cpp Makefile
	$(CXX) $(CXXFLAGS) -o .out/projection.cxxcoro main.cpp -DUSE_CXX_CORO

.out/projection.switchedresume: main.cpp switchedresume.ll Makefile
	clang -S -emit-llvm -o .out/switchedresume.opt.ll switchedresume.ll
	clang -c -o .out/switchedresume.o switchedresume.ll -O2
	$(CXX) $(CXXFLAGS) -c -o .out/main.o main.cpp
	$(CXX) -o .out/projection.switchedresume .out/main.o .out/switchedresume.o

.out/projection.retcon: main.cpp retcon.ll Makefile
	clang -S -emit-llvm -o .out/retcon.opt.ll retcon.ll
	clang -c -o .out/retcon.o retcon.ll -O2
	$(CXX) $(CXXFLAGS) -c -o .out/main.o main.cpp
	$(CXX) -o .out/projection.retcon .out/main.o .out/retcon.o
