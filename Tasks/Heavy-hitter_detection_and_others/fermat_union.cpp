// #include "./src/Fermat/Fermat_tower.h"
#include "./src/Fermat/fermat.h"
#include "./src/common_func.h"
#include <chrono>
//std::ofstream outfile1("query_union.csv");
int main()
{
    printf("Start accuracy measurement of tower_fermat: TOTAL_MEMORY %dKB,\n", TOT_MEM);
    // uint32_t totnum_packet = ReadTraces();
    uint32_t totnum_packet = myReadTraces();
    //uint32_t totnum_packet = ReadNTraces(10);
    //uint32_t totnum_packet = ReadTwoWindows();
    // double t = 0.0;


    Fermat *sketches[2];
	unordered_map<uint32_t, uint32_t> true_freqs[2];
    // sketches[0] = new Fermat_tower();
    // sketches[1] = new Fermat_tower();
    sketches[0] = new Fermat(TOT_MEM * 1024, 0, INIT);
    sketches[1] = new Fermat(TOT_MEM * 1024, 0, INIT);
    cout << INIT << " " << INIT << endl;
    true_freqs[0].clear();
    true_freqs[1].clear();

    int traceindex = 0;
    int num_pkt = (int)traces[traceindex].size();
    printf("num_pkt: %d\n", num_pkt);

    // int num_pkt1 = (int)traces[0].size();
    // int num_pkt2 = (int)traces[1].size();
    // printf("num_pkt1: %d\n", num_pkt1);
    // printf("num_pkt2: %d\n", num_pkt2);
    // auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_pkt; ++i)
    {
        if(i%2==0){
            ++true_freqs[0][*((uint32_t *)(traces[traceindex][i].key))];
            sketches[0]->Insert(*(uint32_t *)(traces[traceindex][i].key), 1);
        }
        else{
            ++true_freqs[1][*((uint32_t *)(traces[traceindex][i].key))];
            sketches[1]->Insert(*(uint32_t *)(traces[traceindex][i].key), 1);
        }
    }
    //auto start = std::chrono::high_resolution_clock::now();
    // for (int i = 0; i < num_pkt1; ++i)
    // {
    //     ++true_freqs[0][*((uint32_t *)(traces[0][i].key))];
    //     //sketches[0]->insert((char *)(traces[0][i].key), 1);
    //     sketches[0]->Insert(*(uint32_t *)(traces[0][i].key), 1);
    // }
    // for (int i = 0; i < num_pkt2; ++i)
    // {
    //     ++true_freqs[1][*((uint32_t *)(traces[1][i].key))];
    //     //sketches[1]->insert((char *)(traces[1][i].key), 1);
    //     sketches[1]->Insert(*(uint32_t *)(traces[1][i].key), 1);
    // }
	//printf("Insertion finished\n");
    printf("Sizes: %d, %d\n", true_freqs[0].size(), true_freqs[1].size());
	sketches[0]->Union(*sketches[1]);
    unordered_map<uint32_t, int> result;
    sketches[1]->Decode(result);

	// auto end = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double> diff = end - start;
    // t = diff.count();

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

        // uint8_t key[4] = {0};                   // srcIP-flowkey
        // uint32_t temp_first = htonl(elem.first); // convert uint32_t -> uint8_t * 4 array
        // for (int i = 0; i < 4; ++i)
        // {
        //     key[i] = ((uint8_t *)&temp_first)[3 - i];
        // }

        int estimatedFreq = result[elem.first];//sketches[1]->query((char *)&(elem.first));
        //outfile1 << (elem.first) << "," << estimatedFreq << endl;
        double are = (double)(trueFreq - estimatedFreq) / trueFreq;
        double aae = (double)(trueFreq - estimatedFreq);
        // cout << are << endl;
        totalARE += are;
        totalAAE += aae;
        count++;
    }
    double averageARE = totalARE / count;
    double averageAAE = totalAAE / count;

	//cout << "averageARE: " << averageARE  << endl;
    cout << "fermat," << averageARE << "," << averageAAE << "," << TOT_MEM << endl;
    //cout << TOT_MEM << "," << averageARE << "," << t << endl;
	// delete sketches[0];
    // delete sketches[1];
}