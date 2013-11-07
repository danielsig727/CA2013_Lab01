.PHONY: clean
all: compile
compile:
	g++ -c *.cc
	g++ -o reduction *.o -lOpenCL
	/opt/m2s42/bin/m2c --amd --amd-device Pitcairn addReduce.cl
clean:
	rm *.o *.bin reduction
