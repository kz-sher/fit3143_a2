all:
	mpicc assg2.c -o assg2
run:
	mpirun -np 21 assg2