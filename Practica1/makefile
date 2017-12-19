.SUFFIXES:
.PHONY: pc
.PRECIOUS: %_exe

compilador:=g++

pc: prodcons_exe
	./$<

fu: fumadores_exe
	./$<

%_exe: %.cpp
		$(compilador) -std=c++11 -I. -o $@ $< Semaphore.cpp -lpthread

clean:
	rm -f *_exe
