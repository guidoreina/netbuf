CC=g++
CXXFLAGS=-O3 -std=c++11 -Wall -pedantic -D_GNU_SOURCE -I. -fPIC
#CXXFLAGS+=-DDEBUG
LDFLAGS=-shared

MAKEDEPEND=${CC} -MM
LIBRARY=libnetbuf.so

OBJS = net/buffer.o net/buffers.o net/socket.o net/ssl/socket.o \
       net/ssl/library.o net/sender.o

DEPS:= ${OBJS:%.o=%.d}

all: $(LIBRARY)

${LIBRARY}: ${OBJS}
	${CC} ${OBJS} ${LIBS} -o $@ ${LDFLAGS}

clean:
	rm -f ${LIBRARY} ${OBJS} ${DEPS}

${OBJS} ${DEPS} ${LIBRARY} : Makefile

.PHONY : all clean

%.d : %.cpp
	${MAKEDEPEND} ${CXXFLAGS} $< -MT ${@:%.d=%.o} > $@

%.o : %.cpp
	${CC} ${CXXFLAGS} -c -o $@ $<

-include ${DEPS}
