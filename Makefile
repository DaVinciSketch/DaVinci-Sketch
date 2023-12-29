GCC = g++
CFLAGS = -O2 -std=c++14
SSEFLAGS = -msse2 -mssse3 -msse4.1 -msse4.2 -mavx -march=native
INCLUDES = -I./common -I./ElasticSketch
COMMON_SOURCES = ./common/BOBHash32.cpp ./ElasticSketch/HeavyPart.cpp ./ElasticSketch/LightPart.cpp ./ElasticSketch/ElasticSketch.cpp
FILES = ER.out test.out #elastic.out
CPPTYPE = -std=c++17 

all: $(FILES) 

ER.out: ERtest.cpp $(COMMON_SOURCES)
	$(GCC) $(CFLAGS) $(SSEFLAGS) $(INCLUDES) -o ERtest.out ERtest.cpp $(COMMON_SOURCES) $(CPPTYPE)

# test.out: test.cpp $(COMMON_SOURCES)
# 	$(GCC) $(CFLAGS) $(SSEFLAGS) $(INCLUDES) -o test.out test.cpp $(COMMON_SOURCES)

# elastic.out: elastic.cpp $(COMMON_SOURCES)
# 	$(GCC) $(CFLAGS) $(SSEFLAGS) $(INCLUDES) -o elastic.out elastic.cpp $(COMMON_SOURCES)

clean:
	rm $(all) -f *~ *.o *.out
