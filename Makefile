CXX_RELEASE_FLAGS = -std=c++11 -O2
CXX_LINK_LIBS = -lboost_thread -lboost_system -lboost_program_options -lpthread
CXX_FLAGS = $(CXX_RELEASE_FLAGS)
OBJECTS = opoznienia.o clienticmp.o clienttcp.o clientudp.o datacontainer.o delaychecker.o
OBJECTS += serverdns.o serverudp.o tcpconnection.o telnetserver.o utils.o worker.o
HDRS = dns_header.h dns_question.h dns_response.h icmp_header.h ipv4_header.h

.PHONY: all clean

all: opoznienia

opoznienia: $(OBJECTS)
		g++ $(CXX_FLAGS) -o opoznienia $(OBJECTS) $(CXX_LINK_LIBS)

opoznienia.o: opoznienia.cpp dns_header.h dns_question.h dns_response.h $(HDRS)
		g++ $(CXX_FLAGS) -c opoznienia.cpp
		
clienticmp.o: clienticmp.cpp clienticmp.h icmp_header.h ipv4_header.h $(HDRS)
		g++ $(CXX_FLAGS) -c clienticmp.cpp
		
clienttcp.o: clienttcp.cpp clienttcp.h $(HDRS)
		g++ $(CXX_FLAGS) -c clienttcp.cpp

clientudp.o: clientudp.cpp clientudp.h $(HDRS)
		g++ $(CXX_FLAGS) -c clientudp.cpp
	
datacontainer.o: datacontainer.cpp datacontainer.h $(HDRS)
		g++ $(CXX_FLAGS) -c datacontainer.cpp
		
delaychecker.o: delaychecker.cpp delaychecker.h $(HDRS)
		g++ $(CXX_FLAGS) -c delaychecker.cpp

serverdns.o: serverdns.cpp serverdns.h $(HDRS)
		g++ $(CXX_FLAGS) -c serverdns.cpp

serverudp.o: serverudp.cpp serverudp.h $(HDRS)
		g++ $(CXX_FLAGS) -c serverudp.cpp
		
tcpconnection.o: tcpconnection.cpp tcpconnection.h $(HDRS)
		g++ $(CXX_FLAGS) -c tcpconnection.cpp
		
telnetserver.o: telnetserver.cpp telnetserver.h $(HDRS)
		g++ $(CXX_FLAGS) -c telnetserver.cpp
		
utils.o: utils.cpp utils.h $(HDRS)
		g++ $(CXX_FLAGS) -c utils.cpp
	
worker.o: worker.cpp worker.h $(HDRS)
		g++ $(CXX_FLAGS) -c worker.cpp

clean:
	rm opoznienia
	rm *.o
