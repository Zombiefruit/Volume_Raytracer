PROG = volren

OBJS = main.o window.o arcballWindow.o font.o volume.o gpuProgram.o linalg.o volWindow.o cube.o grid.o

INCS = 
LIBS = -Llib -lpthread -lGLEW -lglut -lGLU -lGL

CXXFLAGS = -O2 -Wno-write-strings -DLINUX $(INCS) -DWORKING_VERSION
CXX      = g++

all:	genpvm $(PROG)

genpvm:	genpvm.o
	$(CXX) $(CXXFLAGS) -o genpvm genpvm.o

$(PROG):	$(OBJS)
	$(CXX) $(CXXFLAGS) -o $(PROG) $(OBJS) $(LIBS)

.C.o:
	$(CXX) $(CXXFLAGS) -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<


clean:
	rm -f *.o *~ core volren genpvm Makefile.bak

depend:	
	makedepend -Y *.h *.cpp

# DO NOT DELETE

arcballWindow.o: headers.h linalg.h window.h
font.o: headers.h
gpuProgram.o: headers.h linalg.h
grid.o: gpuProgram.h headers.h linalg.h
main.o: volWindow.h headers.h arcballWindow.h linalg.h window.h volume.h
main.o: gpuProgram.h grid.h
volWindow.o: headers.h arcballWindow.h linalg.h window.h volume.h main.h
volWindow.o: volWindow.h gpuProgram.h grid.h
volume.o: headers.h main.h volWindow.h arcballWindow.h linalg.h window.h
volume.o: volume.h gpuProgram.h grid.h
window.o: headers.h linalg.h
arcballWindow.o: headers.h arcballWindow.h linalg.h window.h font.h
cube.o: headers.h cube.h main.h volWindow.h arcballWindow.h linalg.h window.h
cube.o: volume.h gpuProgram.h grid.h
font.o: headers.h
gpuProgram.o: gpuProgram.h headers.h linalg.h
grid.o: headers.h grid.h gpuProgram.h linalg.h
linalg.o: linalg.h
main.o: headers.h volWindow.h arcballWindow.h linalg.h window.h volume.h
main.o: main.h gpuProgram.h grid.h
volWindow.o: volWindow.h headers.h arcballWindow.h linalg.h window.h volume.h
volWindow.o: main.h gpuProgram.h grid.h
volume.o: headers.h volWindow.h arcballWindow.h linalg.h window.h volume.h
volume.o: main.h gpuProgram.h grid.h font.h cube.h
window.o: window.h headers.h linalg.h
