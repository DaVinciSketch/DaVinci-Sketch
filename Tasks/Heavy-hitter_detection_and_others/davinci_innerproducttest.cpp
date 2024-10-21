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
    int entry_num = (TOT_MEM_IN_BYTES - HEAVY_MEM)*10;
    int _fermatcount = 2; //Use Count version with id+ and cnt +-
    bool _fing = false;

    DaVinci<bucket_num> *sketches[2];


    double aveare = 0.0, aveaae = 0.0, ave_HH = 0.0, ave_HC = 0.0, ave_card_RE = 0.0;
    double ave_HH_are = 0.0;
    double ave_WMRD = 0.0, ave_entr_RE = 0.0;
    unordered_map<uint32_t, uint32_t> true_freqs[2];

    uint32_t init_seed = INIT;
    sketches[0] = new DaVinci<bucket_num>();
    sketches[1] = new DaVinci<bucket_num>();

    
    true_freqs[0].clear();
    true_freqs[1].clear();


    vector<int> true_dist(1);
    int traceindex = 0;

    int num_pkt = (int)traces[traceindex].size();
    printf("num_pkt: %d\n", num_pkt);
    int num1, num2 = 0;
    sketches[0]->write2file("Before_insert.txt");
    for (int i = 0; i < num_pkt; ++i)
    {
        if(i%2==0){
            ++true_freqs[0][*((uint32_t *)(traces[traceindex][i].key))];
            num1++;
            sketches[0]->insert((const char *)(traces[traceindex][i].key), 1);
        }
        else{
            ++true_freqs[1][*((uint32_t *)(traces[traceindex][i].key))];
            num2++;
            sketches[1]->insert((const char *)(traces[traceindex][i].key), 1);
        }
    }
    sketches[0]->write2file("After_insert.txt");
    for (int i = 0; i < bucket_num; i++)
    {
        for(int j = 0; j < MAX_VALID_COUNTER; ++j){
            if(sketches[0]->heavy_part->buckets[i].key[j] == 50331651)
            cout << sketches[0]->heavy_part->buckets[i].key[j] << ": " << sketches[0]->heavy_part->buckets[i].val[j] << endl;
        }
    }
    printf("Insertion finished\n");
    printf("Sizes: %d, %d\n", true_freqs[0].size(), true_freqs[1].size());


    long double calculated_innerproduct = InnerProduct<bucket_num>(*sketches[0], *sketches[1], true);


    long double real_innerproduct = 0;
    for (auto it = true_freqs[0].begin(); it != true_freqs[0].end(); ++it)
    {
        real_innerproduct += (long double)it->second * (long double)true_freqs[1][it->first];
    }
    printf("Real inner product: %Lf\n", real_innerproduct);
    printf("Calculated inner product: %Lf\n", calculated_innerproduct);
    printf("Error rate: %Lf\n", (real_innerproduct - calculated_innerproduct) / real_innerproduct);
    
    /*-*-*-* End of packet insertion *-*-*-*/


    delete sketches[0];
    delete sketches[1];

}