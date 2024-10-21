#include "../src/ElasticP4/Elastic_P4.h"
#include "../src/Lossradar/lossradar.h"
#include "../src/others/Choose_Ske.h"
#include "../src/common_func.h"
#include <iostream>
#include <chrono>

#define TEST_FCM

int Heavy_Thes=500;
double sketch_mem = 40;
int lossradar_mem = 5000;
int elastic_mem = TOT_MEM;

LossRadar* new_lossradar() {
    return new LossRadar(3, lossradar_mem * 1024, INIT); 
}

int main()
{
    std::ofstream outFile("outputs/memory_access.csv", std::ios::app);
    // outFile << "Ela MEM" << ", " << "Ela MA" << ", " << "Ela AMA" << ", " 
    //        << "Los MEM" << ", " << "Los MA" << ", " << "Los AMA" << ", " 
    //        << "join MEM" << ", "<< "Join MA" << ", " << "Join AMA" << "\n";

    int d = 3;
    int CHOOSE = 0;
	int w = sketch_mem * 1024;
    uint32_t totnum_packet = ReadTwoWindows();
    
    ElasticSketch *elastic = new ElasticSketch();
    LossRadar *lossradar = new_lossradar();
    Sketch *join = Choose_Sketch(w, d, INIT, CHOOSE);

    int elastic_trace_mac = 0;
    uint32_t lossradar_trace_mac = 0;
    int join_trace_mac = 0;
    
    int numpkt = traces[0].size();
    for (int i = 0; i < numpkt; ++i)
    {
        #ifdef TEST_FCM
        elastic->insert((uint8_t *)(traces[0][i].key), 1);
        elastic_trace_mac += elastic->memory_access_counter;
        #endif
        if ((i % 3 == 0)||(i % 3 == 1)){
            lossradar->Insert_range_data(*((uint32_t *)(traces[0][i].key)), 1);
        }
        lossradar_trace_mac += lossradar->memory_access_counter;
        join->Insert((const char *)(traces[0][i].key));
        join_trace_mac += join->getMemoryAccessCounter();
    }
    double ave_elastic_trace_mac = (double)elastic_trace_mac / numpkt;
    double ave_lossradar_trace_mac = (double)lossradar_trace_mac / numpkt;
    double ave_join_trace_mac = (double)join_trace_mac / numpkt;
    outFile << elastic_mem << ", " << elastic_trace_mac << ", " << ave_elastic_trace_mac << ", " 
            << lossradar_mem << ", " << lossradar_trace_mac << ", " << ave_lossradar_trace_mac << ", "
            << sketch_mem << ", " << join_trace_mac << ", " << ave_join_trace_mac << "\n";
    outFile.close();
}