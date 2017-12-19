compilador := mpicxx -std=c++11 -g -Wall

filosofos_camarero_ex:
		$(compilador) -o filosofos_camarero_ex filosofos_camarero_ex.cpp

run:
	mpirun -np 12 ./filosofos_camarero_ex
