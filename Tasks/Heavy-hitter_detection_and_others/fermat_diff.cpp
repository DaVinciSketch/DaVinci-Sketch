#include "./src/Fermat/fermat.h"
#include "./src/FCMelastic/FCMelastic.h"

#include "./src/common_func.h"

int main()
{
    
    uint32_t totnum_packet = myReadTraces();
    
    Fermat *sketches[2];
	unordered_map<uint32_t, uint32_t> true_freqs[2];

    sketches[0] = new Fermat(600 * 1024, false, INIT);
    sketches[1] = new Fermat(600 * 1024, false, INIT);

    true_freqs[0].clear();
    true_freqs[1].clear();

    int traceindex = 0;
    int num_pkt = (int)traces[traceindex].size();
    for (int i = 0; i < num_pkt; ++i)
    {   
        
        if (i % 3 == 0) {
            ++true_freqs[0][*((uint32_t *)(traces[traceindex][i].key))];
            sketches[0]->Insert(*(uint32_t *)(traces[traceindex][i].key), 1);
        }
        if (i % 3 == 1) {
            ++true_freqs[0][*((uint32_t *)(traces[traceindex][i].key))];
            sketches[0]->Insert(*(uint32_t *)(traces[traceindex][i].key), 1);

            ++true_freqs[1][*((uint32_t *)(traces[traceindex][i].key))];
            sketches[1]->Insert(*(uint32_t *)(traces[traceindex][i].key), 1);
        }
        if (i % 3 == 2) { 
            ++true_freqs[1][*((uint32_t *)(traces[traceindex][i].key))];
            sketches[1]->Insert(*(uint32_t *)(traces[traceindex][i].key), 1);
        }
    }

    printf("Sizes: %d, %d\n", true_freqs[0].size(), true_freqs[1].size());
    unordered_map<uint32_t, int> result;
    
	sketches[0]->diff(*sketches[1]);
    bool flag = sketches[1]->Decode(result);
    
	unordered_map<uint32_t, int32_t> real_result;
    for (const auto &elem : true_freqs[0])
    {
        if (true_freqs[1].find(elem.first) == true_freqs[1].end())
        {
            real_result[elem.first] = elem.second;
        }
        else
        {
            real_result[elem.first] = elem.second - true_freqs[1][elem.first];
        }
    }
    for (const auto &elem : true_freqs[1])
    {
        if (true_freqs[0].find(elem.first) == true_freqs[0].end())
        {
            real_result[elem.first] = -elem.second;
        }
    }
	double totalARE = 0.0;
    double totalAAE = 0.0;
    int count = 0;
    for (auto& elem : real_result) {
        int trueFreq = elem.second;
        if(trueFreq == 0) continue;

        int estimatedFreq = result[elem.first];
        double are = fabs((double)(trueFreq - estimatedFreq) / trueFreq);
        double aae = fabs((double)(trueFreq - estimatedFreq));
        totalARE += are;
        totalAAE += aae;
        count++;
    }
    printf("count = %d\n", count);
    double averageARE = totalARE / count;
    double averageAAE = totalAAE / count;

    cout << "fermat," << averageARE << "," << averageAAE << "," << TOT_MEM << endl;
}