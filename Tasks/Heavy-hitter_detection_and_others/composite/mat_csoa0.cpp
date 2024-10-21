#include "../src/Hashpipe/hashpipe.h"
#include "../src/FCMelastic/FCMelastic.h"
#include "../src/ElasticP4/Elastic_P4.h"
#include "../src/Fermat/fermat.h"
#include "../src/others/Choose_Ske.h"
#include "../src/common_func.h"
#include <iostream>
#include <chrono>

int Heavy_Thes=500;
int seed = INIT;
int hash_mem = TOT_MEM;
int fcm_mem = TOT_MEM;
int elastic_mem = TOT_MEM;
int fermat_mem = 862;
double sketch_mem = 863;

int main()
{
    std::ofstream outFile("outputs/memory_access.csv", std::ios::app);
    //outFile << "Hash MEM" << ", " << "Hash MA" << ", "
    //        << "Fcm MEM" << ", " << "Fcm MA" << ", "
    //        << "Ela MEM" << ", " << "Ela MA" << ", "
    //        << "Fer MEM" << ", " << "Fer MA" << ", "
    //        << "Join MEM" << ", "<< "Join MA" << ", "
    //        << "TOT MEM" << ", " << "AVE MA" << "\n";

    int d = 3;
    int CHOOSE = 0;
	int w = sketch_mem * 1024;
    uint32_t totnum_packet = ReadTwoWindows();
    
    HashPipe *hashpipe = new HashPipe(TOT_MEM * 1024);
    FCMPlus *fcm = new FCMPlus();
    ElasticSketch *elastic = new ElasticSketch();
    Fermat *fermat = new Fermat(fermat_mem * 1024, 0, seed);
    Sketch *join = Choose_Sketch(w, d, INIT, CHOOSE);

    int hash_trace_mac = 0;
    int fcm_trace_mac = 0;
    int elastic_trace_mac = 0;
    uint32_t fermat_trace_mac = 0;
    int join_trace_mac = 0;
    
    int numpkt = traces[0].size();
    for (int i = 0; i < numpkt; ++i)
    {
        hashpipe->insert((uint8_t *)(traces[0][i].key));
        hash_trace_mac += hashpipe->memory_access_counter;
        
        fcm->insert((uint8_t *)(traces[0][i].key), 1);
        fcm_trace_mac += fcm->memory_access_counter;
        
        elastic->insert((uint8_t *)(traces[0][i].key), 1);
        elastic_trace_mac += elastic->memory_access_counter;
        
        fermat->Insert(*(uint32_t *)(traces[0][i].key), 1);
        fermat_trace_mac += fermat->memory_access_counter;
        
        join->Insert((const char *)(traces[0][i].key));
        join_trace_mac += join->getMemoryAccessCounter();
    }
    double total_mem = fcm_mem + fermat_mem + sketch_mem + 2 * TOT_MEM;
    double ave_trace_mac = (double)(hash_trace_mac + fcm_trace_mac + elastic_trace_mac + fermat_trace_mac + join_trace_mac) / numpkt;

    outFile << hash_mem << ", " << hash_trace_mac << ", "
            << fcm_mem << ", " << fcm_trace_mac << ", "
            << elastic_mem << ", " << elastic_trace_mac << ", "
            << fermat_mem << ", " << fermat_trace_mac << ", "
            << sketch_mem << ", " << join_trace_mac << ", " 
            << total_mem << ", " << ave_trace_mac << "\n";
    outFile.close();
}
