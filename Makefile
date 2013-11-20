.PHONY: clean
include library.mk
INCLUDE_PATH += /opt/cuda/include 
CFLAGS += -fopenmp 
LFLAGS += $(LIB)
STUDENT_ID = 0250187
all: compile
compile:
	g++ -I $(INCLUDE_PATH) $(CFLAGS) -c *.cc
	g++ -o reduction *.o $(LFLAGS)
	/opt/m2s42/bin/m2c --amd --amd-device Pitcairn addReduce.cl
compile32:
	g++ -m32 -I $(INCLUDE_PATH) $(CFLAGS) -c *.cc
	g++ -m32 -o reduction *.o $(LFLAGS)
	/opt/m2s42/bin/m2c --amd --amd-device Pitcairn addReduce.cl
compile_srun:
	g++ -g -I $(INCLUDE_PATH) -DSEPARATED_RUN $(CFLAGS) -c *.cc
	g++ -o reduction *.o $(LFLAGS)
	/opt/m2s42/bin/m2c --amd --amd-device Pitcairn addReduce.cl
compile_nv:
	g++ -g -I $(INCLUDE_PATH) -DNV  $(CFLAGS) -c *.cc
	g++ -o reduction *.o $(LFLAGS)
compile_nv_srun:
	g++ -g -I $(INCLUDE_PATH) -DNV -DSEPARATED_RUN  $(CFLAGS) -c *.cc
	g++ -o reduction *.o $(LFLAGS)
gen_handin: 
	mkdir handin/$(STUDENT_ID)
	cp addReduce.cl defines.hh reduction.cc reduction.hh library.mk handin/$(STUDENT_ID)
	cd handin; tar czf $(STUDENT_ID).tar.gz $(STUDENT_ID)
handin_test: gen_handin
	wget --no-check-certificate -O - 'https://sites.google.com/a/g2.nctu.edu.tw/course2013fall_computerarchitecture/announcements/lab1addreductionwithopencl/reduction.tar.bz2' | tar xjvf - -C handin
	wget --no-check-certificate -O handin/reduction/Makefile 'https://sites.google.com/a/g2.nctu.edu.tw/course2013fall_computerarchitecture/announcements/lab1faq/Makefile'
	wget --no-check-certificate -O handin/reduction/utils.cc 'https://sites.google.com/a/g2.nctu.edu.tw/course2013fall_computerarchitecture/announcements/lab1faq/utils.cc'
	cp handin/$(STUDENT_ID)/* handin/reduction/
	cd handin/reduction; make
clean:
	rm -rf *.o *.bin reduction handin/*
