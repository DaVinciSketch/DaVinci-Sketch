#include "./src/Fermat/Fermat_tower.h"
#include "./src/common_func.h"
std::ofstream outfile1("query_diff.csv");
int main()
{
    //printf("Start accuracy measurement of tower_fermat: TOTAL_MEMORY %dKB,\n", TOT_MEM);
    // uint32_t totnum_packet = ReadTraces();
    uint32_t totnum_packet = myReadTraces();
    //uint32_t totnum_packet = ReadNTraces(10);


    Fermat_tower *sketches[2];
	unordered_map<uint32_t, uint32_t> true_freqs[2];
    sketches[0] = new Fermat_tower();
    sketches[1] = new Fermat_tower();
    true_freqs[0].clear();
    true_freqs[1].clear();

    int traceindex = 0;
    int num_pkt = (int)traces[traceindex].size();
    //printf("num_pkt: %d\n", num_pkt);
    for (int i = 0; i < num_pkt; ++i)
    {
        if(i%2==0){
            ++true_freqs[0][*((uint32_t *)(traces[traceindex][i].key))];
            sketches[0]->insert((char *)(traces[traceindex][i].key), 1);
        }
        else{
            ++true_freqs[1][*((uint32_t *)(traces[traceindex][i].key))];
            sketches[1]->insert((char *)(traces[traceindex][i].key), 1);
        }
    }
	//printf("Insertion finished\n");
    //printf("Sizes: %d, %d\n", true_freqs[0].size(), true_freqs[1].size());
	sketches[0]->diff(*sketches[1]);
	// flcsketch = &sketch_result;
	//flcsketch->decode();
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
            real_result[elem.first] = elem.second;
        }
    }
	double totalARE = 0.0;
    int count = 0;
    for (auto& elem : real_result) {
        int trueFreq = elem.second;
        if(trueFreq == 0) continue;
        if (trueFreq < 0)
            trueFreq = trueFreq * -1;

        // uint8_t key[4] = {0};                   // srcIP-flowkey
        // uint32_t temp_first = htonl(elem.first); // convert uint32_t -> uint8_t * 4 array
        // for (int i = 0; i < 4; ++i)
        // {
        //     key[i] = ((uint8_t *)&temp_first)[3 - i];
        // }

        int estimatedFreq = sketches[1]->query((char *)&(elem.first));
        outfile1 << (elem.first) << "," << estimatedFreq << "," << trueFreq << endl;
        double are = fabs((double)(trueFreq - estimatedFreq) / trueFreq);
        // cout << are << endl;
        totalARE += are;
        count++;
    }
    double averageARE = totalARE / count;

	//cout << "averageARE: " << averageARE  << endl;
    cout << "fermat," << averageARE << "," << TOT_MEM << endl;
	delete sketches[0];
    delete sketches[1];
}