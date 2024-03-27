#include "./src/ElasticRadar/ERSketch.h"
#include "./src/common_func.h"
#define HEAVY_MEM (150 * 1024)
#define TOT_MEM_IN_BYTES (600 * 1024)
#define BUCKET_NUM (HEAVY_MEM / 64)
static constexpr int bucket_num = BUCKET_NUM;
// template class HeavyPart<2400>;
int main()
{
    printf("Start accuracy measurement of tower_fermat: TOTAL_MEMORY %dKB, FERMAT_BUCKET %d\n", TOT_MEM, ELE_BUCKET);
    // uint32_t totnum_packet = ReadTraces();
    uint32_t totnum_packet = myReadTraces();

    int array_num = 3;
    int entry_num = (TOT_MEM_IN_BYTES - HEAVY_MEM)*10;
    int _fermatcount = 2; //Use Count version with id+ and cnt +-
    bool _fing = false;

    FLCSketch<bucket_num> *sketch;

    double aveare = 0.0, aveaae = 0.0, ave_HH = 0.0, ave_HC = 0.0, ave_card_RE = 0.0;
    double ave_HH_are = 0.0;
    double ave_WMRD = 0.0, ave_entr_RE = 0.0;
    unordered_map<uint32_t, uint32_t> true_freq;

    uint32_t init_seed = INIT;
    sketch = new FLCSketch<bucket_num>(BUCKET_NUM, array_num, entry_num, _fermatcount, _fing, init_seed);
    true_freq.clear();

    vector<int> true_dist(1);
    int traceindex = 0;
    int num_pkt = (int)traces[traceindex].size();
    printf("num_pkt: %d\n", num_pkt);
    int num1, num2 = 0;
    for (int i = 0; i < num_pkt; ++i)
    {
        ++true_freq[*((uint32_t *)(traces[traceindex][i].key))];
        num1++;
        sketch->insert((const char *)(traces[traceindex][i].key), 1);
    }
    printf("Insertion finished\n");
    // printf("Sizes: %d, %d\n", true_freq.size(), true_freqs[1].size());

    
    vector<double> real_distribution(10, 0);
    for(auto it = true_freq.begin(); it != true_freq.end(); ++it)
    {
        if(real_distribution.size() < it->second + 1)
            real_distribution.resize(it->second + 1);
        real_distribution[it->second]++;
    }

    for(int index = 0;index < array_num; index++){
        vector<double> dist;//[array_num];


        sketch->get_distribution(dist, index);
        
        std::ofstream outFile("./outputs/dist_result_compare_array" + std::to_string(index) + ".csv");
        outFile << "Num, EstVal, RealVal" << std::endl;
        int minnum = std::min(dist.size(), real_distribution.size());
        long double are = 0;
        for(int i = 0; i < minnum; ++i)
        {
            outFile << i << ", " << dist[i] << ", " << real_distribution[i] << std::endl;
            if(real_distribution[i] != 0)     
                are += std::abs(dist[i] - real_distribution[i]) / real_distribution[i];
            else
                are += dist[i];
        }
        if(dist.size() > real_distribution.size())
        {
            for(int i = minnum; i < dist.size(); ++i)
            {
                outFile << i << ", " << dist[i] << ", 0" << std::endl;
            }
            // cout << "dist.size() > real_distribution.size() by " << dist.size() - real_distribution.size() << " elements" << endl;
        }
        else if(dist.size() < real_distribution.size())
        {
            for(int i = minnum; i < real_distribution.size(); ++i)
            {
                outFile << i << ", 0, " << real_distribution[i] << std::endl;
            }
            // cout << "dist.size() < real_distribution.size() by " << real_distribution.size() - dist.size() << " elements" << endl;
        }

        outFile.close();
        cout << index << "th array ARE: " << are / minnum << endl;
    }

    delete sketch;

}