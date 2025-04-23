EXECS=mpi_hello_world
MPICC?=mpicc

all: ${EXECS}

mpi_hello_world: hello.c
	${MPICC} -o mpi_hello_world hello.c

clean:
	rm -f ${EXECS}