#include "./src/DaVinci/DaVinci.h"
#include "./src/common_func.h"
#define HEAVY_MEM (150 * 1024)
#define TOT_MEM_IN_BYTES (600 * 1024)
#define BUCKET_NUM (HEAVY_MEM / 64)
static constexpr int bucket_num = BUCKET_NUM;


int main()
{
    printf("Start accuracy measurement of tower_fermat: TOTAL_MEMORY %dKB, FERMAT_BUCKET %d\n", TOT_MEM, ELE_BUCKET);
    
    uint32_t totnum_packet = myReadTraces();

    int array_num = 3;
    int entry_num = (TOT_MEM_IN_BYTES - HEAVY_MEM) * 10;
    int _fermatcount = 2; // Use Count version with id+ and cnt +-
    bool _fing = false;

    double aveare = 0.0, aveaae = 0.0, ave_HH = 0.0, ave_HC = 0.0, ave_card_RE = 0.0;
    double ave_HH_are = 0.0;
    double ave_WMRD = 0.0, ave_entr_RE = 0.0;
    unordered_map<uint32_t, uint32_t> true_freq;

    uint32_t init_seed = 813;
    
    
    int totalMem[] = {200*1024, 300*1024, 400*1024, 500*1024, 600*1024};
    int num_pkt = (int)traces[0].size();
    for (int i = 0; i < num_pkt; ++i)
    {
        ++true_freq[*((uint32_t *)(traces[0][i].key))];
    }
    vector<int> real_distribution(10, 0);
    for (auto it = true_freq.begin(); it != true_freq.end(); ++it)
    {
        if (real_distribution.size() < it->second + 1)
            real_distribution.resize(it->second + 1);
        real_distribution[it->second]++;
    }
    
    
    long double minwmrd = 1000000;
    int minindex = -1;
    int index = 0;

    bool singletest = 0;
    int singletest_fermat_mem = 24000;
    int singletest_heavy_bucket_num = 3100;
    int singletest_tower_mem = 512000 - singletest_fermat_mem - singletest_heavy_bucket_num*64;
    
    std::ofstream outFile("./outputs/finalresults/davinci_entropy_final_result.csv", std::ios::app);
    outFile << "index, totalMem, fermatMem, heavyBucketNum, towerMem, RE\n";
    for (int fermat_mem_base = 10000; fermat_mem_base <= 500 * 1024; fermat_mem_base += 10000)
    {
        for (int heavy_bucket_num_base = 1000; heavy_bucket_num_base * 64 <= 500 * 1024 - fermat_mem_base; heavy_bucket_num_base += 100)
        {
            double totalsum = 0;
            for (int index_ = 0; index_ < 5; index_++)
            {
                cout << index_;
                int cur_total_mem = totalMem[index_]; 
                double cur_alpha = (double)cur_total_mem / (500*1024);
                int cur_fermat_mem = fermat_mem_base * cur_alpha;
                int cur_heavy_bucket_num = heavy_bucket_num_base * cur_alpha;
                int cur_tower_mem = cur_total_mem - cur_fermat_mem - cur_heavy_bucket_num*64;
                if(singletest){
                    cur_total_mem = 204800;
                    cur_fermat_mem = 8000;
                    cur_heavy_bucket_num = 2040;
                    cur_tower_mem = 66240;
                }

                if(cur_tower_mem < 1){
                    continue;
                }
                unique_ptr<DaVinci<bucket_num>> davinci = make_unique<DaVinci<bucket_num>>(cur_total_mem, cur_fermat_mem, cur_heavy_bucket_num, cur_tower_mem, 3, false, 813);
                
                
                int traceindex = 0;
                printf("num_pkt: %d\n", num_pkt);
                int num1, num2 = 0;
                for (int i = 0; i < num_pkt; ++i)
                {
                    
                    num1++;
                    davinci->insert((const char *)(traces[traceindex][i].key), 1);
                }
                davinci->decode(1);
                vector<double> dist(10, 0); //[array_num];
                davinci->get_distribution(dist, 0);
                double entropy_est = davinci->get_entropy(dist);

                double entr_true = 0;
                double tot_true = 0;
                double entropy_true = 0;
                for (int i = 0; i < real_distribution.size(); ++i)
                {
                    if (real_distribution[i] == 0)
                        continue;
                    tot_true += i * real_distribution[i];
                    entr_true += i * real_distribution[i] * log2(i);
                }
                entropy_true = -entr_true / tot_true + log2(tot_true);

                double entropy_err = std::abs(entropy_est - entropy_true) / entropy_true;
                totalsum += entropy_err;
                
                outFile << (index++) << ", " << cur_total_mem << ", " << cur_fermat_mem << ", " << cur_heavy_bucket_num << ", " << cur_tower_mem << "," << entropy_err << "\n";
                if(singletest){
                    return 0;
                }
            }
            if(totalsum < minwmrd){
                minwmrd = totalsum;
                minindex = index;
            }
        }
    }
    outFile << "minre, " << minwmrd << ", minindex, " << minindex << "\n";
    outFile.close();
}