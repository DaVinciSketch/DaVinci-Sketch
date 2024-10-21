#include "../src/DaVinci/DaVinci.h"
#include "../src/common_func.h"
#include <iostream>
#include <chrono>
#define HEAVY_MEM (150 * 1024)
#define TOT_MEM_IN_BYTES (200 * 1024)
#define BUCKET_NUM (HEAVY_MEM / 64)
static constexpr int bucket_num = BUCKET_NUM;

double vector_mean(vector<double> &v){
    double sum = 0;
    for(auto &i : v){
        sum += i;
    }
    return sum / v.size();
}

int main()
{
    printf("Start accuracy measurement of tower_fermat: TOTAL_MEMORY %dKB, FERMAT_BUCKET %d\n", TOT_MEM, ELE_BUCKET);
    uint32_t totnum_packet = ReadTwoWindows();
    std::ofstream outFile("outputs/alltests/davinci_all_tests.csv");
    outFile << "TotalMem, HeavyNum, TowerMem, FermatMem, FlowSizeARE, HHF1, HCF1, CardRE, DistWMRE, EntrRE, UnionARE, DiffARE, InnerPARE, TotalTime\n";

    int array_num = 3;
    int entry_num = (TOT_MEM_IN_BYTES - HEAVY_MEM);
    int _fermatcount = 2; //Use Count version with id+ and cnt +-
    bool _fing = false;

    double aveare = 0.0, aveaae = 0.0, ave_HH = 0.0, ave_HC = 0.0, ave_card_RE = 0.0;
    double ave_HH_are = 0.0;
    double ave_WMRD = 0.0, ave_entr_RE = 0.0;
    unordered_map<uint32_t, uint32_t> true_freqs[2];

    vector<double> hhresult(6, 0);
    vector<double> Cardresult(6, 0);
    vector<double> flowsizeresult(6, 0);
    for(int mem = 25; mem <= 700; mem += 25){
        double alpha = (double)mem / 500.0;
        double hh_pre = 0;
        int totaltime = 10;
        ave_card_RE = 0;
        vector<double> flowsizeprec; //v
        vector<double> hhf1; //v
        vector<double> hcf1; //v
        vector<double> cardre; //v
        vector<double> distwmre; //v
        vector<double> entrre; // v
        vector<double> unionare; // v
        vector<double> diffare; // v
        vector<double> innerpare; //
        vector<double> totaltimevec;
        int singletest_total_mem = alpha*500*1024;
        int singletest_fermat_mem = alpha*24000;//160000;
        int singletest_heavy_bucket_num = alpha*3100;
        int singletest_tower_mem = alpha*512000 - singletest_fermat_mem - singletest_heavy_bucket_num*64;
        for (int times = 0; times < totaltime; times++)//TIMES; times++)
        {
            int init_seed = prime_seeds[times*8];
            auto start = std::chrono::high_resolution_clock::now();
            std::cout << "times: " << times << std::endl;
            cout << "singletest_fermat_mem: " << singletest_fermat_mem << " singletest_heavy_bucket_num: " << singletest_heavy_bucket_num << " singletest_tower_mem: " << singletest_tower_mem << endl;
            unique_ptr<DaVinci<bucket_num>> davinci0 = make_unique<DaVinci<bucket_num>>(singletest_total_mem, singletest_fermat_mem, singletest_heavy_bucket_num, singletest_tower_mem, 3, false, init_seed);
            unique_ptr<DaVinci<bucket_num>> davinci1 = make_unique<DaVinci<bucket_num>>(singletest_total_mem, singletest_fermat_mem, singletest_heavy_bucket_num, singletest_tower_mem, 3, false, init_seed);
            true_freqs[0].clear();
            true_freqs[1].clear();

            vector<int> true_dist(1);
            int traceindex = 0;
            int num_pkt = (int)traces[traceindex].size();
            int numpkt0 = traces[0].size();
            int numpkt1 = traces[1].size();
            for (int i = 0; i < numpkt0; ++i)
            {
                ++true_freqs[0][*((uint32_t *)(traces[0][i].key))];
                davinci0->insert((const char *)(traces[0][i].key), 1);
            }
            for (int i = 0; i < numpkt1; ++i)
            {
                ++true_freqs[1][*((uint32_t *)(traces[1][i].key))];
                davinci1->insert((const char *)(traces[1][i].key), 1);
            }
            //union
            unique_ptr<DaVinci<bucket_num>> davinci_union_result = make_unique<DaVinci<bucket_num>>(singletest_total_mem, singletest_fermat_mem, singletest_heavy_bucket_num, singletest_tower_mem, 3, false, init_seed);
            Union<bucket_num>(*davinci0, *davinci1, *davinci_union_result, 37);
            
            unique_ptr<DaVinci<bucket_num>> davinci_diff_result = make_unique<DaVinci<bucket_num>>(singletest_total_mem, singletest_fermat_mem, singletest_heavy_bucket_num, singletest_tower_mem, 3, false, init_seed);
            Difference<bucket_num>(*davinci0, *davinci1, *davinci_diff_result, 37);

            long double innerpresult = InnerProduct<bucket_num>(*davinci0, *davinci1, 37); // will decode davinci0 and davinci1

            auto start_dist = std::chrono::high_resolution_clock::now();
            vector<double> dist_result(10, 0);
            davinci0->get_distribution(dist_result, 0); // need decode
            auto end_dist = std::chrono::high_resolution_clock::now();

            double entropy_result = davinci0->get_entropy(dist_result); // need distribution

            int cardinality_result = davinci0->get_cardinality(); // need decode

            set<uint32_t> davinci_hh;
            davinci0->get_heavy_hitters(davinci_hh);

            auto end = std::chrono::high_resolution_clock::now();

            // Start calculating flow size precision
            set<uint32_t> HH_true;
            set<uint32_t>::iterator itr;
            double flowSizeARE = 0.0;
            for(auto it = true_freqs[0].begin(); it != true_freqs[0].end(); ++it)
            {
                int estimated = (int)davinci0->query((const char *)&(it->first), 1);
                int actual = (int)it->second;
                double dist = abs(actual - estimated);
                flowSizeARE += dist * 1.0 / actual;
                if(actual > HH_THRESHOLD){
                    HH_true.insert(it->first);
                }
            }
            flowSizeARE /= true_freqs[0].size();
            flowsizeprec.push_back(flowSizeARE);

            // Start calculating HH F1
            double HH_precision = 0;
            double HH_are = 0;
            int HH_PR = 0;
            int HH_PR_denom = 0;
            int HH_RR = 0;
            int HH_RR_denom = 0;
            for (itr = HH_true.begin(); itr != HH_true.end(); ++itr)
            {
                HH_PR_denom += 1;
                HH_PR += (davinci_hh.find(*itr) != davinci_hh.end());
            }
            for (itr = davinci_hh.begin(); itr != davinci_hh.end(); ++itr)
            {
                HH_RR_denom += 1;
                HH_RR += (HH_true.find(*itr) != HH_true.end());
            }
            HH_precision = (2 * (double(HH_PR) / double(HH_PR_denom)) * (double(HH_RR) / double(HH_RR_denom))) / ((double(HH_PR) / double(HH_PR_denom)) + (double(HH_RR) / double(HH_RR_denom)));
            hhf1.push_back(HH_precision);


            // Start calculating HC F1
            davinci1->get_all_results();
            if(!(davinci0->have_got_all_result)||!(davinci1->have_got_all_result)){
                printf("Error: not all results are got\n");
                return 0;
            }else{
                printf("get all results ok\n");
            }
            double HC_precision = 0;
            int HC_PR = 0;
            int HC_PR_denom = 0;
            int HC_RR = 0;
            int HC_RR_denom = 0;
            set<uint32_t> est_hc;
            set<uint32_t> real_hc;
            for (auto f : davinci1->allResult)
            {
                if ((int)davinci1->query((const char *)&f.first) -
                        (int)davinci0->query((const char *)&f.first) >
                    HC_THRESHOLD)
                    est_hc.insert(f.first);
            }
            cout << endl;
            for (auto f : davinci0->allResult)
            {
                if ((int)davinci0->query((const char *)&f.first) -
                        (int)davinci1->query((const char *)&f.first) >
                    HC_THRESHOLD)
                    est_hc.insert(f.first);
            }
            cout << endl;
            for (auto f : true_freqs[1])
            {
                if (!true_freqs[0].count(f.first))
                {
                    if (f.second > HC_THRESHOLD)
                        real_hc.insert(f.first);
                }
                else if (int(f.second - true_freqs[0][f.first]) > HC_THRESHOLD)
                    real_hc.insert(f.first);
            }
            for (auto f : true_freqs[0])
            {
                if (!true_freqs[1].count(f.first))
                {
                    if (f.second > HC_THRESHOLD)
                        real_hc.insert(f.first);
                }
                else if (int(f.second - true_freqs[1][f.first]) > HC_THRESHOLD)
                    real_hc.insert(f.first);
            }
            for (itr = real_hc.begin(); itr != real_hc.end(); ++itr)
            {
                HC_PR_denom += 1;
                HC_PR += (est_hc.find(*itr) != est_hc.end());
            }
            for (itr = est_hc.begin(); itr != est_hc.end(); ++itr)
            {
                HC_RR_denom += 1;
                HC_RR += (real_hc.find(*itr) != real_hc.end());
            }
            HC_precision = (2 * (double(HC_PR) / double(HC_PR_denom)) * (double(HC_RR) / double(HC_RR_denom))) / ((double(HC_PR) / double(HC_PR_denom)) + (double(HC_RR) / double(HC_RR_denom)));
            hcf1.push_back(HC_precision);

            // Start calciulating cardinality RE
            double cardER = abs(cardinality_result - int(true_freqs[0].size())) / double(true_freqs[0].size());
            cardre.push_back(cardER);

            // Start calculating distribution WMRD and entropy RE
            vector<int> real_distribution(10, 0);
            for (auto it = true_freqs[0].begin(); it != true_freqs[0].end(); ++it){
                if (real_distribution.size() < it->second + 1)
                    real_distribution.resize(it->second + 1);
                real_distribution[it->second]++;
            }
            double WMRD = 0;
            double WMRD_nom = 0;
            double WMRD_denom = 0;
            double entr_true = 0;
            double tot_true = 0;
            double entropy_true = 0;
            printf("get ok\n");
            fflush(stdout);
            if (real_distribution.size() > dist_result.size())
                dist_result.resize(real_distribution.size());
            for (int i = 1; i < real_distribution.size(); ++i)
            {
                if (real_distribution[i] == 0)
                {
                    continue;
                }
                WMRD_nom += std::abs(real_distribution[i] - dist_result[i]);
                WMRD_denom += double(real_distribution[i] + dist_result[i]) / 2;
                tot_true += i * real_distribution[i];
                entr_true += i * real_distribution[i] * log2(i);
            }
            WMRD = WMRD_nom / WMRD_denom;
            distwmre.push_back(WMRD);
            entropy_true = -entr_true / tot_true + log2(tot_true);
            double entropy_err = std::abs(entropy_result - entropy_true) / entropy_true;
            entrre.push_back(entropy_err);
            
            //Start calculating Union ARE
            unordered_map<uint32_t, int32_t> union_real_result;
            unordered_map<uint32_t, int32_t> diff_real_result;
            for (const auto &elem : true_freqs[0])
            {
                if (true_freqs[1].find(elem.first) == true_freqs[1].end())
                {
                    union_real_result[elem.first] = elem.second;
                }
                else
                {
                    union_real_result[elem.first] = elem.second + true_freqs[1][elem.first];
                }

                if (true_freqs[1].find(elem.first) == true_freqs[1].end())
                {
                    diff_real_result[elem.first] = elem.second;
                }
                else
                {
                    diff_real_result[elem.first] = elem.second - true_freqs[1][elem.first];
                }

            }
            for (const auto &elem : true_freqs[1])
            {
                if (true_freqs[0].find(elem.first) == true_freqs[0].end())
                {
                    union_real_result[elem.first] = elem.second;
                }

                if (true_freqs[0].find(elem.first) == true_freqs[0].end())
                {
                    diff_real_result[elem.first] = -elem.second;
                }
            }
            double unionARE = 0.0;
            int count = 0;
            for (const auto& elem : union_real_result) {
                int trueFreq = elem.second;
                if(trueFreq == 0) continue;
                int estimatedFreq = davinci_union_result->query((char*)&(elem.first), true);
                double are = fabs((double)(trueFreq - estimatedFreq) / trueFreq);
                unionARE += are;
                count++;
            }
            unionARE /= count;
            unionare.push_back(unionARE);

            double diffARE = 0.0;
            count = 0;
            for (const auto& elem : diff_real_result) {
                int trueFreq = elem.second;
                if(trueFreq == 0) continue;
                int estimatedFreq = davinci_diff_result->query((char*)&(elem.first), true);
                double are = fabs((double)(trueFreq - estimatedFreq) / trueFreq);
                diffARE += are;
                count++;
            }
            diffARE /= count;
            diffare.push_back(diffARE);

            // Start calculating Inner Product ARE
            long double real_innerproduct = 0;
            for (auto it = true_freqs[0].begin(); it != true_freqs[0].end(); ++it)
            {
                real_innerproduct += (long double)it->second * (long double)true_freqs[1][it->first];
            }
            long double innerpRE = std::abs(innerpresult - real_innerproduct) / real_innerproduct;
            innerpare.push_back(innerpRE);

            // Time Interval
            std::chrono::duration<double> diff = end - start;
            std::chrono::duration<double> dist_dur = end_dist - start_dist;
            totaltimevec.push_back(diff.count());
        }
        outFile << singletest_total_mem << ", " << singletest_heavy_bucket_num << ", " << singletest_tower_mem << ", " << singletest_fermat_mem << ", " 
        << vector_mean(flowsizeprec) << ", " << vector_mean(hhf1) << ", " << vector_mean(hcf1) << ", " << vector_mean(cardre) << ", " << vector_mean(distwmre) << ", " << vector_mean(entrre) << ", " << vector_mean(unionare) << ", " << vector_mean(diffare) << ", " << vector_mean(innerpare) << ", " << vector_mean(totaltimevec) << "\n";
        for(int i=0; i < 10; i ++){
            cout << "flowsizeprec[" << i << "]: " << flowsizeprec[i] << ",";
            cout << "hhf1[" << i << "]: " << hhf1[i] << ",";
            cout << "hcf1[" << i << "]: " << hcf1[i] << ",";
            cout << "cardre[" << i << "]: " << cardre[i] << ",";
            cout << "distwmre[" << i << "]: " << distwmre[i] << ",";
            cout << "entrre[" << i << "]: " << entrre[i] << ",";
            cout << "unionare[" << i << "]: " << unionare[i] << ",";
            cout << "diffare[" << i << "]: " << diffare[i] << ",";
            cout << "innerpare[" << i << "]: " << innerpare[i] << ",";
            cout << "totaltimevec[" << i << "]: " << totaltimevec[i] << endl;
        }
        flowsizeprec.clear();
        hhf1.clear();
        hcf1.clear();
        cardre.clear();
        distwmre.clear();
        entrre.clear();
        unionare.clear();
        diffare.clear();
        innerpare.clear();
        totaltimevec.clear();

    }
    outFile.close();

}