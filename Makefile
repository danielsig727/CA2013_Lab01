.PHONY: clean
INCLUDE_PATH += /opt/cuda/include 
all: compile_nv
compile:
	g++ -g -I $(INCLUDE_PATH) -c *.cc
	g++ -o reduction *.o -lOpenCL
	/opt/m2s42/bin/m2c --amd --amd-device Pitcairn addReduce.cl
compile_nv:
	g++ -g -I $(INCLUDE_PATH) -DNV -c *.cc
	g++ -o reduction *.o -lOpenCL
run:
	./reduction
clean:
	rm *.o *.bin reduction
