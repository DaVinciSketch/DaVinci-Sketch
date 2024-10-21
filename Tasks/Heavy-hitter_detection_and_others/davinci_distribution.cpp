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
    
    
    int totalMem[] = {100*1024, 200*1024, 300*1024, 400*1024, 500*1024, 600*1024};
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
    int singletest_fermat_mem = 24000;//160000;
    int singletest_heavy_bucket_num = 3100;
    int singletest_tower_mem = 512000 - singletest_fermat_mem - singletest_heavy_bucket_num*64;
    
    std::ofstream outFile("./outputs/finalresults/davinci_dist_final_result.csv", std::ios::app);
    outFile << "index, totalMem, fermatMem, heavyBucketNum, towerMem, WMRD\n";
    for (int fermat_mem_base = 64000; fermat_mem_base <= 500 * 1024; fermat_mem_base += 10000)
    {
        for (int heavy_bucket_num_base = 1000; heavy_bucket_num_base * 64 <= 500 * 1024 - fermat_mem_base; heavy_bucket_num_base += 100)
        {
            for (int index_ = 0; index_ < 6; index_++)
            {
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
                printf("Insertion finished\n");
                
                
                davinci->decode(1);

                vector<double> dist(10, 0);

                cout << "Entering get_distribution" << endl;
                davinci->get_distribution(dist, 0);
                cout << "Exiting get_distribution" << endl;


                int minnum = std::min(dist.size(), real_distribution.size());
                long double are = 0;
                for (int i = 0; i < minnum; ++i)
                {
                    
                    if (real_distribution[i] != 0)
                        are += std::abs(dist[i] - real_distribution[i]) / real_distribution[i];
                    else
                        are += dist[i];
                }
                
                
                cout << index << "th array ARE: " << are / minnum << endl;

                // compute WMRD
                double WMRD = 0;
                double WMRD_nom = 0;
                double WMRD_denom = 0;
                printf("get ok\n");
                fflush(stdout);
                if (real_distribution.size() > dist.size())
                    dist.resize(real_distribution.size());
                for (int i = 1; i < real_distribution.size(); ++i)
                {
                    if (real_distribution[i] == 0)
                    {
                        continue;
                    }
                    
                    WMRD_nom += std::abs(real_distribution[i] - dist[i]);
                    WMRD_denom += double(real_distribution[i] + dist[i]) / 2;
                }
                WMRD = WMRD_nom / WMRD_denom;
                if(minwmrd > WMRD)
                {
                    minwmrd = WMRD;
                    minindex = index;
                }

                printf("WMRD : %3.5f\n", WMRD);
                fflush(stdout);
                outFile << (index++) << ", " << cur_total_mem << ", " << cur_fermat_mem << ", " << cur_heavy_bucket_num << ", " << cur_tower_mem << "," << WMRD << "\n";
                if(singletest){
                    return 0;
                }
            }
        }
    }
    outFile << "minwmrd, " << minwmrd << ", minindex, " << minindex << "\n";
    outFile.close();
}