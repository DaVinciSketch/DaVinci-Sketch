CPPTYPE = -std=c++17 
DISTRIBUTION = ./elasticradar_distribution.cpp

all: 
	@if [ ! -d ./build ]; then mkdir -p ./build; fi; 
	make countheap cmsketch coco cusketch elasticp4 fcm fermat hashpipe mrac univmon

countheap: ./countheap.cpp
	g++ -O2 -o ./build/countheap.out ./countheap.cpp
cmsketch: ./cmsketch.cpp
	g++ -O2 -o ./build/cmsketch.out ./cmsketch.cpp
coco: ./coco.cpp
	g++ -O2 -o ./build/coco.out ./coco.cpp
cusketch: ./cusketch.cpp
	g++ -O2 -o ./build/cusketch.out ./cusketch.cpp
elasticp4: ./elasticp4.cpp
	g++ -O2 -o ./build/elasticp4.out ./elasticp4.cpp
elasticradar: ./elasticradar.cpp ./src/ElasticRadar/HeavyPart.cpp ./src/ElasticRadar/common/BOBHash32.cpp
	g++ -O2 -o ./build/elasticradar.out ./elasticradar.cpp ./src/ElasticRadar/HeavyPart.cpp $(CPPTYPE) ./src/ElasticRadar/common/BOBHash32.cpp
elasticradar_difftest: ./elasticradar_difftest.cpp ./src/ElasticRadar/HeavyPart.cpp ./src/ElasticRadar/common/BOBHash32.cpp
	g++ -O2 -o ./build/elasticradar_difftest.out ./elasticradar_difftest.cpp ./src/ElasticRadar/HeavyPart.cpp ./src/ElasticRadar/common/BOBHash32.cpp $(CPPTYPE)
elasticradar_uniontest: ./elasticradar_uniontest.cpp ./src/ElasticRadar/HeavyPart.cpp ./src/ElasticRadar/common/BOBHash32.cpp
	g++ -O2 -o ./build/elasticradar_uniontest.out ./elasticradar_uniontest.cpp ./src/ElasticRadar/HeavyPart.cpp $(CPPTYPE)
elasticradar_innerproducttest: ./elasticradar_innerproducttest.cpp ./src/ElasticRadar/HeavyPart.cpp ./src/ElasticRadar/common/BOBHash32.cpp
	g++ -O2 -o ./build/elasticradar_innerproducttest.out ./elasticradar_innerproducttest.cpp ./src/ElasticRadar/HeavyPart.cpp ./src/ElasticRadar/common/BOBHash32.cpp $(CPPTYPE)
elasticradar_distribution: ./elasticradar_distribution.cpp ./src/ElasticRadar/HeavyPart.cpp ./src/ElasticRadar/common/BOBHash32.cpp
	g++ -O2 -o ./build/elasticradar_distribution.out ./elasticradar_distribution.cpp ./src/ElasticRadar/HeavyPart.cpp ./src/ElasticRadar/common/BOBHash32.cpp $(CPPTYPE)
ER_all_tests_dist: ./ER_all_tests_dist.cpp ./src/ElasticRadar/HeavyPart.cpp ./src/ElasticSketch/ElasticSketch.cpp ./src/ElasticSketch/LightPart.cpp ./src/ElasticRadar/common/BOBHash32.cpp
	g++ -O2 -o ./build/ER_all_tests_dist.out ./ER_all_tests_dist.cpp ./src/ElasticRadar/HeavyPart.cpp ./src/ElasticSketch/ElasticSketch.cpp ./src/ElasticSketch/LightPart.cpp ./src/ElasticRadar/common/BOBHash32.cpp $(CPPTYPE)
ER_all_tests_diff: ./ER_all_tests_diff.cpp ./src/ElasticRadar/HeavyPart.cpp ./src/ElasticSketch/ElasticSketch.cpp ./src/ElasticSketch/LightPart.cpp ./src/ElasticRadar/common/BOBHash32.cpp
	g++ -O2 -o ./build/ER_all_tests_diff.out ./ER_all_tests_diff.cpp ./src/ElasticRadar/HeavyPart.cpp ./src/ElasticSketch/ElasticSketch.cpp ./src/ElasticSketch/LightPart.cpp ./src/ElasticRadar/common/BOBHash32.cpp $(CPPTYPE)
ER_all_tests_union: ./ER_all_tests_union.cpp ./src/ElasticRadar/HeavyPart.cpp ./src/ElasticSketch/ElasticSketch.cpp ./src/ElasticSketch/LightPart.cpp ./src/ElasticRadar/common/BOBHash32.cpp
	g++ -O2 -o ./build/ER_all_tests_union.out ./ER_all_tests_union.cpp ./src/ElasticRadar/HeavyPart.cpp ./src/ElasticSketch/ElasticSketch.cpp ./src/ElasticSketch/LightPart.cpp ./src/ElasticRadar/common/BOBHash32.cpp $(CPPTYPE)
ER_all_tests_innerp: ER_all_tests_innerp.cpp ./src/ElasticRadar/HeavyPart.cpp ./src/ElasticSketch/ElasticSketch.cpp ./src/ElasticSketch/LightPart.cpp ./src/ElasticRadar/common/BOBHash32.cpp
	g++ -O2 -o ./build/ER_all_tests_innerp.out ER_all_tests_innerp.cpp ./src/ElasticRadar/HeavyPart.cpp ./src/ElasticSketch/ElasticSketch.cpp ./src/ElasticSketch/LightPart.cpp ./src/ElasticRadar/common/BOBHash32.cpp $(CPPTYPE)
ER_all_tests_accuracy: ER_all_tests_accuracy.cpp ./src/ElasticRadar/HeavyPart.cpp ./src/ElasticSketch/ElasticSketch.cpp ./src/ElasticSketch/LightPart.cpp ./src/ElasticRadar/common/BOBHash32.cpp
	g++ -O2 -o ./build/ER_all_tests_accuracy.out ER_all_tests_accuracy.cpp ./src/ElasticRadar/HeavyPart.cpp ./src/ElasticSketch/ElasticSketch.cpp ./src/ElasticSketch/LightPart.cpp ./src/ElasticRadar/common/BOBHash32.cpp $(CPPTYPE)
fcm: ./fcm.cpp
	g++ -O2 -o ./build/fcm.out ./fcm.cpp -lpthread
fermat: ./fermat.cpp
	g++ -O2 -o ./build/fermat.out ./fermat.cpp
hashpipe: ./hashpipe.cpp
	g++ -O2 -o ./build/hashpipe.out ./hashpipe.cpp
mrac: ./mrac.cpp
	g++ -O3 -o ./build/mrac.out ./mrac.cpp
univmon: ./univmon.cpp
	g++ -O2 -o ./build/univmon.out ./univmon.cpp

test: ./test.cpp
	g++ -O2 -o ./build/test.out ./test.cpp

davinci: ./davinci.cpp ./src/DaVinci/HeavyPart.cpp ./src/DaVinci/common/BOBHash32.cpp
	g++ -O2 -o ./build/davinci.out ./davinci.cpp ./src/DaVinci/HeavyPart.cpp $(CPPTYPE) ./src/DaVinci/common/BOBHash32.cpp
davinci_all_tests: ./davinci_all_tests.cpp ./src/DaVinci/HeavyPart.cpp ./src/DaVinci/common/BOBHash32.cpp
	g++ -O3 -o ./build/davinci_all_tests.out ./davinci_all_tests.cpp ./src/DaVinci/HeavyPart.cpp $(CPPTYPE) ./src/DaVinci/common/BOBHash32.cpp
find_best_davinci: ./find_best_davinci.cpp ./src/DaVinci/HeavyPart.cpp ./src/DaVinci/common/BOBHash32.cpp
	g++ -g -pthread -O2 -o ./build/find_best_davinci.out ./find_best_davinci.cpp ./src/DaVinci/HeavyPart.cpp $(CPPTYPE) ./src/DaVinci/common/BOBHash32.cpp
find_best_davinci_hh: ./find_best_davinci_hh.cpp ./src/DaVinci/HeavyPart.cpp ./src/DaVinci/common/BOBHash32.cpp
	g++ -g -pthread -O2 -o ./build/find_best_davinci_hh.out ./find_best_davinci_hh.cpp ./src/DaVinci/HeavyPart.cpp $(CPPTYPE) ./src/DaVinci/common/BOBHash32.cpp
davinci_uniontest: ./davinci_uniontest.cpp ./src/DaVinci/HeavyPart.cpp ./src/DaVinci/common/BOBHash32.cpp
	g++ -O2 -o ./build/davinci_uniontest.out ./davinci_uniontest.cpp ./src/DaVinci/HeavyPart.cpp $(CPPTYPE) ./src/DaVinci/common/BOBHash32.cpp
davinci_difftest: ./davinci_difftest.cpp ./src/DaVinci/HeavyPart.cpp ./src/DaVinci/common/BOBHash32.cpp
	g++ -O2 -o ./build/davinci_difftest.out ./davinci_difftest.cpp ./src/DaVinci/HeavyPart.cpp $(CPPTYPE) ./src/DaVinci/common/BOBHash32.cpp
davinci_innerproducttest: ./davinci_innerproducttest.cpp ./src/DaVinci/HeavyPart.cpp ./src/DaVinci/common/BOBHash32.cpp
	g++ -g -O2 -o ./build/davinci_innerproducttest.out ./davinci_innerproducttest.cpp ./src/DaVinci/HeavyPart.cpp ./src/DaVinci/common/BOBHash32.cpp $(CPPTYPE)
davinci_distribution: ./davinci_distribution.cpp ./src/DaVinci/HeavyPart.cpp ./src/DaVinci/common/BOBHash32.cpp
	g++ -g -O2 -o ./build/davinci_distribution.out ./davinci_distribution.cpp ./src/DaVinci/HeavyPart.cpp ./src/DaVinci/common/BOBHash32.cpp $(CPPTYPE)
davinci_cardinality: ./davinci_cardinality.cpp ./src/DaVinci/HeavyPart.cpp ./src/DaVinci/common/BOBHash32.cpp
	g++ -g -O2 -o ./build/davinci_cardinality.out ./davinci_cardinality.cpp ./src/DaVinci/HeavyPart.cpp ./src/DaVinci/common/BOBHash32.cpp $(CPPTYPE)
davinci_entropy: ./davinci_entropy.cpp ./src/DaVinci/HeavyPart.cpp ./src/DaVinci/common/BOBHash32.cpp
	g++ -g -O2 -o ./build/davinci_entropy.out ./davinci_entropy.cpp ./src/DaVinci/HeavyPart.cpp ./src/DaVinci/common/BOBHash32.cpp $(CPPTYPE)
davinci_hc: ./davinci_hc.cpp ./src/DaVinci/HeavyPart.cpp ./src/DaVinci/common/BOBHash32.cpp
	g++ -g -O2 -o ./build/davinci_hc.out ./davinci_hc.cpp ./src/DaVinci/HeavyPart.cpp ./src/DaVinci/common/BOBHash32.cpp $(CPPTYPE)


clean:
	rm ./build/*.out 