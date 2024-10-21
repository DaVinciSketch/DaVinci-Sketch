#include "../src/FCMelastic/FCMelastic.h"
#include "../src/Fermat/fermat.h"
#include "../src/others/Choose_Ske.h"
#include "../src/common_func.h"
#include <iostream>
#include <chrono>

#define TEST_FCM

int Heavy_Thes=500;
int fcm_mem = TOT_MEM;
int fermat_mem = 990;
double sketch_mem = 40;
int seed = INIT;

Fermat* new_fermat() {
    return new Fermat(fermat_mem * 1024, 0, seed); 
}

int main()
{
    std::ofstream outFile("outputs/memory_access.csv", std::ios::app);
    // outFile << "Fcm MEM" << ", " << "Fcm MA" << ", "
    //         << "Fer MEM" << ", " << "Fer MA" << ", "
    //         << "join MEM" << ", " << "Join MA" << ", "
    //         << "TOT MEM" << ", " << "AVE MA" << "\n";

    int d = 3;
    int CHOOSE = 0;
	int w = sketch_mem * 1024;
    uint32_t totnum_packet = ReadTwoWindows();
    
    FCMPlus *fcm = new FCMPlus();
    Fermat *fermat = new_fermat();
    Sketch *join = Choose_Sketch(w, d, INIT, CHOOSE);

    int fcm_trace_mac = 0;
    uint32_t fermat_trace_mac = 0;
    int join_trace_mac = 0;
    
    int numpkt = traces[0].size();
    for (int i = 0; i < numpkt; ++i)
    {
        #ifdef TEST_FCM
        fcm->insert((uint8_t *)(traces[0][i].key), 1);
        fcm_trace_mac += fcm->memory_access_counter;
        #endif
        fermat->Insert(*(uint32_t *)(traces[0][i].key), 1);
        fermat_trace_mac += fermat->memory_access_counter;
        join->Insert((const char *)(traces[0][i].key));
        join_trace_mac += join->getMemoryAccessCounter();
    }
    double total_mem = fcm_mem + fermat_mem + sketch_mem;
    double ave_trace_mac = (double)(fcm_trace_mac + fermat_trace_mac + join_trace_mac) / numpkt;
    outFile << fcm_mem << ", " << fcm_trace_mac << ", "
            << fermat_mem << ", " << fermat_trace_mac << ", "
            << sketch_mem << ", " << join_trace_mac << ", "
            << total_mem << ", " << ave_trace_mac << "\n";
    outFile.close();
}