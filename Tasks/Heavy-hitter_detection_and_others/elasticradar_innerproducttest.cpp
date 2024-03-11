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

    FLCSketch<bucket_num> *sketches[2];
    FLCSketch<bucket_num> *flcsketch = NULL;

    // Fermat_tower *sketches[2];
    // Fermat_tower *fermat_tower = NULL;
    double aveare = 0.0, aveaae = 0.0, ave_HH = 0.0, ave_HC = 0.0, ave_card_RE = 0.0;
    double ave_HH_are = 0.0;
    double ave_WMRD = 0.0, ave_entr_RE = 0.0;
    unordered_map<uint32_t, uint32_t> true_freqs[2];

    uint32_t init_seed = INIT;
    sketches[0] = new FLCSketch<bucket_num>(BUCKET_NUM, array_num, entry_num, _fermatcount, _fing, init_seed);
    sketches[1] = new FLCSketch<bucket_num>(BUCKET_NUM, array_num, entry_num, _fermatcount, _fing, init_seed);
    true_freqs[0].clear();
    true_freqs[1].clear();

    // printf("[INFO] %3d-th trace starts to be processed..\n", times);
    // unordered_map<uint32_t, int32_t> &true_freq = true_freqs[iter_data];
    vector<int> true_dist(1);
    int traceindex = 0;
    // flcsketch= sketches[iter_data];
    int num_pkt = (int)traces[traceindex].size();
    printf("num_pkt: %d\n", num_pkt);
    int num1, num2 = 0;
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
    printf("Insertion finished\n");
    printf("Sizes: %d, %d\n", true_freqs[0].size(), true_freqs[1].size());

    // FLCSketch<bucket_num> sketch_result = Union<bucket_num>(*sketches[0], *sketches[1], init_seed);
    long double calculated_innerproduct = InnerProduct<bucket_num>(*sketches[0], *sketches[1]);

    //Real inner product
    long double real_innerproduct = 0;
    for (auto it = true_freqs[0].begin(); it != true_freqs[0].end(); ++it)
    {
        real_innerproduct += (long double)it->second * (long double)true_freqs[1][it->first];
    }
    printf("Real inner product: %Lf\n", real_innerproduct);
    printf("Calculated inner product: %Lf\n", calculated_innerproduct);
    printf("Error rate: %Lf\n", (real_innerproduct - calculated_innerproduct) / real_innerproduct);

    sketches[0]->write2file("innerp_sketch1.txt");
    sketches[1]->write2file("innerp_sketch2.txt");
    
    /*-*-*-* End of packet insertion *-*-*-*/


    delete sketches[0];
    delete sketches[1];

}