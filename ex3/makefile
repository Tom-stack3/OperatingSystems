CXX=g++
RM=rm -f
LDFLAGS=-g -c -pthread -std=c++11

ex3.out: ex3.o BoundedQueue.o UnboundedQueue.o
	$(CXX) -g -pthread -std=c++11 -o ex3.out ex3.o BoundedQueue.o UnboundedQueue.o

ex3.o: ex3.cpp ex3.h
	$(CXX) $(LDFLAGS) ex3.cpp

BoundedQueue.o: BoundedQueue.cpp BoundedQueue.h
	$(CXX) $(LDFLAGS) BoundedQueue.cpp

UnboundedQueue.o: UnboundedQueue.cpp UnboundedQueue.h
	$(CXX) $(LDFLAGS) UnboundedQueue.cpp

clean:
	$(RM) ex3.out ex3.o BoundedQueue.o UnboundedQueue.o