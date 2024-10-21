#include "../src/ElasticP4/Elastic_P4.h"
#include "../src/Lossradar/lossradar.h"
#include "../src/others/Choose_Ske.h"
#include "../src/common_func.h"
#include <iostream>
#include <chrono>

int lossradar_mem = 5;
double sketch_mem = 0.8;

LossRadar* new_lossradar(int init) {
    return new LossRadar(1, lossradar_mem * 1024, init); 
}

double vector_mean(vector<double> &v){
    double sum = 0;
    for(auto &i : v){
        sum += i;
    }
    return sum / v.size();
}

int Heavy_Thes=500;

int main()
{
    uint32_t totnum_packet = ReadTwoWindows();
    
    double aveare = 0.0, aveaae = 0.0, ave_HH = 0.0, ave_HC = 0.0, ave_card_RE = 0.0;
    double ave_HH_are = 0.0;
    double ave_WMRD = 0.0, ave_entr_RE = 0.0;
    
    unordered_map<uint32_t, uint32_t> true_freqs[2];

    vector<double> hhresult(6, 0);
    vector<double> Cardresult(6, 0);
    vector<double> flowsizeresult(6, 0);

    double hh_pre = 0;
    int totaltime = 20;
    ave_card_RE = 0;
    
    vector<double> flowsizeprec;
    vector<double> hhf1;
    vector<double> hcf1;
    vector<double> cardre;
    vector<double> distwmre;
    vector<double> entrre;
    vector<double> unionare;
    vector<double> unionaae;
    vector<double> diffare;
    vector<double> innerpare;
    vector<double> totaltimevec;
    int d = 3;
	int w = sketch_mem * 1024;
    int CHOOSE = 0;
    
    for (int times = 0; times < totaltime; times++)
    {
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << "times: " << times << std::endl;
        
        int seed = INIT;
        ElasticSketch *elastic0 = new ElasticSketch(seed),
                      *elastic1 = new ElasticSketch(seed),
                      *elastic2 = new ElasticSketch(seed);
        LossRadar *lossradar0 = new_lossradar(seed),
                  *lossradar1 = new_lossradar(seed);
        Sketch *join0 = Choose_Sketch(w, d, seed, CHOOSE),
            *join1 = Choose_Sketch(w, d, seed, CHOOSE);

        true_freqs[0].clear();
        true_freqs[1].clear();
        
        vector<int> true_dist(1);
        int numpkt0 = traces[0].size();
        int numpkt1 = traces[1].size();

        for (int i = 0; i < numpkt0; ++i)
        {
            ++true_freqs[0][*((uint32_t *)(traces[0][i].key))];
            elastic0->insert((uint8_t *)(traces[0][i].key), 1);
            lossradar0->Insert_range_data(*((uint32_t *)(traces[0][i].key)), 1);
            join0->Insert((const char *)(traces[0][i].key));
        }
        for (int i = 0; i < numpkt1; ++i)
        {
            ++true_freqs[1][*((uint32_t *)(traces[1][i].key))];
            elastic1->insert((uint8_t *)(traces[1][i].key), 1);
            lossradar1->Insert_range_data(*((uint32_t *)(traces[1][i].key)), 1);
            join1->Insert((const char *)(traces[1][i].key));
        }
        elastic2->Union(*elastic0, *elastic1, *elastic2);
        lossradar0->diff(*lossradar1);

        // innerproduct
        long double innerpresult = join0->Join(join1);
        // distribution
        auto start_dist = std::chrono::high_resolution_clock::now();
        vector<double> dist_result;
        elastic0->get_distribution(dist_result); // need decode
        auto end_dist = std::chrono::high_resolution_clock::now();
        // entropy
        double tot_est = 0;
        double entr_est = 0;
        for (int i = 1; i < dist_result.size(); ++i)
        {
            if (dist_result[i] == 0)
                continue;
            tot_est += i * dist_result[i];
            entr_est += i * dist_result[i] * log2(i);
        }
        double entropy_result = -entr_est / tot_est + log2(tot_est);
        // card
        int cardinality_result = elastic0->get_cardinality(); // need decode
        // heavy hitter/changer
        set<uint32_t> hh_est;
        elastic0->get_heavy_hitters(hh_est);

        auto end = std::chrono::high_resolution_clock::now();
        // Start calculating flow size precision
        set<uint32_t> HH_true;
        set<uint32_t>::iterator itr;
        double flowSizeARE = 0.0;
        for(auto it = true_freqs[0].begin(); it != true_freqs[0].end(); ++it)
        {
            uint8_t key[4] = {0};                   // srcIP-flowkey
            uint32_t temp_first = htonl(it->first); // convert uint32_t -> uint8_t * 4 array
            for (int i = 0; i < 4; ++i)
            {
                key[i] = ((uint8_t *)&temp_first)[3 - i];
            }
            int estimated = (int)elastic0->query(key);
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
            HH_PR += (hh_est.find(*itr) != hh_est.end());
        }
        for (itr = hh_est.begin(); itr != hh_est.end(); ++itr)
        {
            HH_RR_denom += 1;
            HH_RR += (HH_true.find(*itr) != HH_true.end());
        }
        HH_precision = (2 * (double(HH_PR) / double(HH_PR_denom)) * (double(HH_RR) / double(HH_RR_denom))) / ((double(HH_PR) / double(HH_PR_denom)) + (double(HH_RR) / double(HH_RR_denom)));
        hhf1.push_back(HH_precision);


        // Start calculating HC F1
        double HC_precision = 0;
        int HC_PR = 0;
        int HC_PR_denom = 0;
        int HC_RR = 0;
        int HC_RR_denom = 0;
        set<uint32_t> est_hc;
        set<uint32_t> real_hc;
        unordered_map<uint32_t, int> hc_candidates[2];
        elastic0->get_hc_candidates(hc_candidates[0]);
        elastic1->get_hc_candidates(hc_candidates[1]);
        for (auto f : hc_candidates[1])
        {
            if (int(f.second - elastic0->query((uint8_t *)&f.first)) > (int)HC_THRESHOLD)
                est_hc.insert(f.first);
        }
        for (auto f : hc_candidates[0])
        {
            if (int(f.second - elastic1->query((uint8_t *)&f.first)) > (int)HC_THRESHOLD)
                est_hc.insert(f.first);
        }
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
        double unionAAE = 0.0;
        int count = 0;
        unordered_map<uint32_t, int> union_result, diff_result;
 
        lossradar1->Decode(diff_result);


        for (auto& elem : union_real_result) {
            int trueFreq = elem.second;
            if(trueFreq == 0) continue;
            int estimatedFreq = elastic2->query((uint8_t*)&(elem.first));
            double are = fabs((double)(trueFreq - estimatedFreq) / trueFreq);
            double aae = fabs((double)(trueFreq - estimatedFreq));
            // cout << are << endl;
            unionARE += are;
            unionAAE += aae;
            count++;
        }
        double average_unionARE = unionARE / count;
        double average_unionAAE = unionAAE / count;
        unionARE /= count;
        unionare.push_back(unionARE);
        unionaae.push_back(unionAAE);

        double diffARE = 0.0;
        count = 0;
        for (const auto& elem : diff_real_result) {
            int trueFreq = elem.second;
            if(trueFreq == 0) continue;
            int estimatedFreq = diff_result[elem.first];
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
        
        long double innerpRE = 0;
        if (real_innerproduct != 0)
            innerpRE = std::abs(innerpresult - real_innerproduct) / real_innerproduct;
        else
            std::cout << "Real Inner Product is 0!!!!!" << endl;
        innerpare.push_back(innerpRE);

        // Time Interval
        std::chrono::duration<double> diff = end - start;
        
        std::chrono::duration<double> dist_dur = end_dist - start_dist;
        totaltimevec.push_back(diff.count());


        delete elastic0;
        delete elastic1;
        delete elastic2;
        delete lossradar0;
        delete lossradar1;
        delete join0;
        delete join1;
        // Output
        // outFile << "TotalMem, HeavyNum, TowerMem, FermatMem, FlowSizeARE, HHF1, HCF1, CardRE, DistWMRE, EntrRE, UnionARE, DiffARE, InnerPARE, TotalTime\n";
        // outFile << singletest_total_mem << ", " << singletest_heavy_bucket_num << ", " << singletest_tower_mem << ", " << singletest_fermat_mem << ", " << flowSizeARE << ", " << HH_precision << ", " << HC_precision << ", " << cardER << ", " << WMRD << ", " << entropy_err << ", " << unionARE << ", " << diffARE << ", " << innerpRE << ", " << diff.count() << "\n";
    }
    cout << vector_mean(flowsizeprec) << ", " << vector_mean(hhf1) << ", " << vector_mean(hcf1) << ", " << vector_mean(cardre) << ", " << vector_mean(distwmre) << ", " << vector_mean(entrre) << ", " << vector_mean(unionare) << ", " << vector_mean(diffare) << ", " << vector_mean(innerpare) << ", " << vector_mean(totaltimevec) << ", " << TOT_MEM << "\n";
    // flowSizeARE << ", " << HH_precision << ", " << HC_precision << ", " << cardER << ", " << WMRD << ", " << entropy_err << ", " << unionARE << ", " << diffARE << ", " << innerpRE << ", " << diff.count() << "\n";
    
    flowsizeprec.clear();
    hhf1.clear();
    hcf1.clear();
    cardre.clear();
    distwmre.clear();
    entrre.clear();
    unionare.clear();
    unionaae.clear();
    diffare.clear();
    innerpare.clear();
    totaltimevec.clear();

}