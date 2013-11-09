.PHONY: clean
INCLUDE_PATH += /opt/cuda/include 
STUDENT_ID = 0250187
all: compile
compile:
	g++ -g -I $(INCLUDE_PATH) -c *.cc
	g++ -o reduction *.o -lOpenCL
	/opt/m2s42/bin/m2c --amd --amd-device Pitcairn addReduce.cl
compile_nv:
	g++ -g -I $(INCLUDE_PATH) -DNV -c *.cc
	g++ -o reduction *.o -lOpenCL
compile_nv_srun:
	g++ -g -I $(INCLUDE_PATH) -DNV -DSEPARATED_RUN -c *.cc
	g++ -o reduction *.o -lOpenCL
gen_handin: 
	mkdir handin/$(STUDENT_ID)
	cp addReduce.cl defines.hh reduction.cc reduction.hh handin/$(STUDENT_ID)
	tar czf handin/$(STUDENT_ID).tar.gz handin/$(STUDENT_ID)
handin_test: gen_handin
	wget -O - 'https://sites.google.com/a/g2.nctu.edu.tw/course2013fall_computerarchitecture/announcements/lab1addreductionwithopencl/reduction.tar.bz2' | tar xjvf - -C handin
	cp handin/$(STUDENT_ID)/* handin/reduction/
	cd hankin/reduction; make
clean:
	rm -rf *.o *.bin reduction handin/*
