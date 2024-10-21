#include "./src/CMSketch/CM.h"
#include "./src/common_func.h"
#define HEAVY_MEM (150 * 1024)
#define TOT_MEM_IN_BYTES (600 * 1024)
#define BUCKET_NUM (HEAVY_MEM / 64)
static constexpr int bucket_num = BUCKET_NUM;
int main()
{
    printf("Start accuracy measurement of tower_fermat: TOTAL_MEMORY %dKB, FERMAT_BUCKET %d\n", TOT_MEM, ELE_BUCKET);
    uint32_t totnum_packet = myReadTraces();

    CMSketch *sketches[2];
    CMSketch *cm = NULL;
	unordered_map<uint32_t, uint32_t> true_freqs[2];
    sketches[0] = new CMSketch(CM_BYTES);
    sketches[1] = new CMSketch(CM_BYTES);
    cm = new CMSketch(CM_BYTES);
    true_freqs[0].clear();
    true_freqs[1].clear();

    int traceindex = 0;
    int num_pkt = (int)traces[traceindex].size();
    printf("num_pkt: %d\n", num_pkt);
    for (int i = 0; i < num_pkt; ++i)
    {
        if(i%2==0){
            ++true_freqs[0][*((uint32_t *)(traces[traceindex][i].key))];
            sketches[0]->insert((uint8_t*)(traces[traceindex][i].key), 1);
        }
        else{
            ++true_freqs[1][*((uint32_t *)(traces[traceindex][i].key))];
            sketches[1]->insert((uint8_t*)(traces[traceindex][i].key), 1);
        }
    }
	printf("Insertion finished\n");
    printf("Sizes: %d, %d\n", true_freqs[0].size(), true_freqs[1].size());
	sketches[0]->Union(*sketches[1]);
	unordered_map<uint32_t, int32_t> real_result;
    for (const auto &elem : true_freqs[0])
    {
        if (true_freqs[1].find(elem.first) == true_freqs[1].end())
        {
            real_result[elem.first] = elem.second;
        }
        else
        {
            real_result[elem.first] = elem.second + true_freqs[1][elem.first];
        }
    }
    for (const auto &elem : true_freqs[1])
    {
        if (true_freqs[0].find(elem.first) == true_freqs[0].end())
        {
            real_result[elem.first] = elem.second;
        }
    }
	double totalARE = 0.0;
    double totalAAE = 0.0;
    int count = 0;
    for (auto& elem : real_result) {
        int trueFreq = elem.second;
        if(trueFreq == 0) continue;
        int estimatedFreq = sketches[1]->query((uint8_t*)&(elem.first));
        double are = fabs((double)(trueFreq - estimatedFreq) / trueFreq);
        double aae = fabs((double)(trueFreq - estimatedFreq));
        totalARE += are;
        totalAAE += aae;
        count++;
    }
    double averageARE = totalARE / count;
    double averageAAE = totalAAE / count;

    cout << "cm," << averageARE << "," << averageAAE << "," << CM_BYTES << endl;
	delete sketches[0];
    delete sketches[1];
	delete cm;
}