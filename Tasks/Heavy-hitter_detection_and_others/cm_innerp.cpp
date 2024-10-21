#include "./src/CMSketch/CM.h"
#include "./src/common_func.h"
#include <cmath>
void ReadInNTraces(const char *trace_prefix, int n)
{
    if (n < 0 || n > 10) {
        printf("Invalid value of n. It should be between 0 and 10.\n");
        return;
    }

    int printednum = 0;
    for (int datafileCnt = 0; datafileCnt <= n; ++datafileCnt)
    {
        char datafileName[100];
        sprintf(datafileName, "%s%d.dat", trace_prefix, datafileCnt);
        FILE *fin = fopen(datafileName, "rb");

        if (fin == NULL) {
            printf("Failed to open file %s\n", datafileName);
            continue;
        }

        SRCIP_TUPLE tmp_five_tuple;
        traces[datafileCnt].clear();
        while (fread(&tmp_five_tuple, 1, 13, fin) == 13)
        {
            traces[datafileCnt].push_back(tmp_five_tuple);
        }
        fclose(fin);

        printf("Successfully read in %s, %ld packets\n", datafileName, traces[datafileCnt].size());
    }
    printf("\n");
}

int main()
{
    printf("Start accuracy measurement of tower_fermat: TOTAL_MEMORY %dKB, FERMAT_BUCKET %d\n", TOT_MEM, ELE_BUCKET);
    uint32_t totnum_packet = myReadTraces();

    CMSketch *sketches[2];
    CMSketch *cm = NULL;
    unordered_map<uint32_t, uint32_t> true_freqs[2];

    sketches[0] = new CMSketch(CM_BYTES);
    sketches[1] = new CMSketch(CM_BYTES);
    true_freqs[0].clear();
    true_freqs[1].clear();

    int traceindex = 0;
    int num_pkt = (int)traces[traceindex].size();
    printf("num_pkt: %d\n", num_pkt);

    for (int i = 0; i < num_pkt; ++i)
    {
        if(i%2==0){
            ++true_freqs[0][*((uint32_t *)(traces[traceindex][i].key))];
            sketches[0]->insert((uint8_t*)(traces[traceindex][i].key));
        }
        else{
            ++true_freqs[1][*((uint32_t *)(traces[traceindex][i].key))];
            sketches[1]->insert((uint8_t*)(traces[traceindex][i].key));
        }
    }
    printf("Insertion finished\n");
    printf("Sizes: %d, %d\n", true_freqs[0].size(), true_freqs[1].size());

    long double calculated_innerproduct = sketches[0]->InnerProduct(sketches[1]);

    //Real inner product
    long double real_innerproduct = 0;
    for (auto it = true_freqs[0].begin(); it != true_freqs[0].end(); ++it)
    {
        real_innerproduct += (long double)it->second * (long double)true_freqs[1][it->first];
    }
    cout << "cm," << (fabs(real_innerproduct - calculated_innerproduct)) / real_innerproduct << ",0," << TOT_MEM << endl;


    delete sketches[0];
    delete sketches[1];

}