MPICC?=mpicc

build/ioutils.o: src/ioutils.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

build/0-serial.o: src/0-serial.c build/ioutils.o
	$(CC) $(CFLAGS) $(CPPFLAGS) -lm -fopenmp -o $@ $^ 

build/1-openmp.o: src/1-openmp.c build/ioutils.o
	$(CC) $(CFLAGS) $(CPPFLAGS) -lm -fopenmp -o $@ $^

build/2-mpi.o: src/2-mpi.c build/ioutils.o
	$(CC) $(CFLAGS) $(CPPFLAGS) -lm -fopenmp -o $@ $^

build/2-mpi-local.o: src/2-mpi.c build/ioutils.o
	$(MPICC) $(CFLAGS) $(CPPFLAGS) -lm -fopenmp -o $@ $^

.PHONY: all mkoutput clean push fetch

mkoutput:
	@mkdir -p build

all: mkoutput build/1-openmp.o build/0-serial.o build/2-mpi.o

clean:
	@rm build/*

push:
	rsync -vhPra --chown=hengyil:hengyil \
		./src \
		./include \
		./makefile \
		./jobs \
		hengyil@dardel.pdc.kth.se:~/Private/project/

fetch:
	rsync -vhPra --chown=finp:users \
		hengyil@dardel.pdc.kth.se:~/Private/project/logs ./