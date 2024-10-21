#include <chrono>
#include <iostream>
#include "../src/common_func.h"
#include "../src/DaVinci/DaVinci.h"

#define HEAVY_MEM (150 * 1024)
#define BUCKET_NUM (HEAVY_MEM / 64)
static constexpr int bucket_num = BUCKET_NUM;

int main()
{
    uint32_t totnum_packet = ReadTwoWindows();
    
    std::ofstream outFile("outputs/memory_access.csv", std::ios::app);
    outFile << "MEM" << ", " << "Dav MA" << ", " << "Dav AMA\n";
    
    int mem_values[] = {25, 50, 75, 100, 200, 300, 400, 500, 600}; // 列出所有需要的数值
    int num_values = sizeof(mem_values) / sizeof(mem_values[0]);    // 计算数组大小
    for (int idx = 0; idx < num_values; ++idx){
        int mem = mem_values[idx];
        double alpha = (double)mem / 500.0;
        int singletest_total_mem = alpha*500*1024;
        int singletest_fermat_mem = alpha*24000;//160000;
        int singletest_heavy_bucket_num = alpha*3100;
        int singletest_tower_mem = alpha*512000 - singletest_fermat_mem - singletest_heavy_bucket_num*64;
        int init_seed = prime_seeds[8];
        
        unique_ptr<DaVinci<bucket_num>> davinci = make_unique<DaVinci<bucket_num>>(singletest_total_mem, singletest_fermat_mem, singletest_heavy_bucket_num, singletest_tower_mem, 3, false, init_seed);

        int numpkt = traces[0].size();
        int davinci_trace_mac = 0; // memory access counter
        for (int i = 0; i < numpkt; ++i)
        {
            davinci->insert((const char *)(traces[0][i].key), 1);
            davinci_trace_mac += davinci->memory_access_counter;
        }
        double ave_davinci_trace_mac = (double)davinci_trace_mac / numpkt;
        outFile << mem << ", " << davinci_trace_mac << ", " << ave_davinci_trace_mac << "\n"; 
    }
    outFile.close();
}