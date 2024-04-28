#pragma once
#include "fermat.h"
#include "tower.h"
#include <map>
#include "../Common/EMFSD1.h"
#include "../common_func.h"
using namespace std;

class Fermat_tower
{
public:
    int tot_memory;
    int tot_packets;
    int fermatEleMem;
    int towerfilterMem;
    Fermat *fermatEle;
    TowerSketch *towerfilter;
    unordered_map<uint32_t, int> Eleresult;
    unordered_map<uint32_t, int> EleFermatInserted;
    EMFSD1 *em = NULL;

public:
    Fermat_tower(int _tot_memory = TOT_MEMORY, int _fermatEleMem = ELE_BUCKET * (8 + 4 * USE_FING),
                 int _towerfilterMem = TOT_MEMORY - (ELE_BUCKET) * (8 + 4 * USE_FING),
                 bool usefing = USE_FING, uint32_t _init = INIT) : tot_memory(_tot_memory), fermatEleMem(_fermatEleMem), towerfilterMem(_towerfilterMem)
    {
        tot_packets = 0;
        fermatEle = new Fermat(fermatEleMem, usefing, _init);
        int w_d = towerfilterMem / 8;
        towerfilter = new TowerSketch(w_d);
    }
    void insert(const char *key, int f = 1)
    {
        if(!towerfilter->insert(key)){
            fermatEle->Insert(*(uint32_t*) key, f);
            EleFermatInserted[*(uint32_t*) key] ++;
        }
        tot_packets++;
    }
    void get_distribution(vector<double> &ed)
    {
        em = new EMFSD1;
        uint32_t *countercpy;
        countercpy = new uint32_t[towerfilter->line[1].width];
        for (int i = 0; i < this->towerfilter->line[1].width; i++)
        {
            countercpy[i] = towerfilter->line[1].index(i);
        }
        em->set_counters(this->towerfilter->line[1].width, countercpy, 65535);
        for (int i = 0; i < FERMAT_EM_ITER; i++)
        {
            printf("%d_epoch\n", i);
            em->next_epoch();
        }
        ed.resize(em->ns.size());
        for (int i = 1; i < em->ns.size(); i++)
        {
            ed[i] = em->ns[i];
        }
        ed.resize(2000000);
        for(auto i : Eleresult){
            if(i.second + towerfilter->threshold > 65535){
                ed[i.second+towerfilter->threshold]++;
                if(ed[65535])
                    ed[65535]-=1;
            }
        }
        int sum = 0;
        for (int i = 1; i < 2000000; i++)
        {
            sum += i * ed[i];
        }
    }
    int get_cardinality()
    {
        int used = 0, total = 0;
        return towerfilter->get_cardinality();
    }
    double get_entropy()
    {
        int tot = 0;
        double entr = 0;
        towerfilter->get_entropy(tot, entr);

        set<uint32_t> checked_items;
        for(auto i : Eleresult){
            checked_items.insert(i.first);
        }
        for(auto it = checked_items.begin();it != checked_items.end();++it){
            uint32_t sum_val = 0;
            uint32_t heavy_val = 0;
            sum_val += Eleresult[*it];
            uint32_t key = *it;

            uint32_t light_val = towerfilter->query8bit((const char*)&key);
            if(light_val != 0){
                sum_val += light_val;
                tot -= light_val;
                entr -= light_val * log2(light_val);
            }
            if(sum_val){
                tot += sum_val;
                entr += sum_val * log2(sum_val);
            }
            
        }
        return -entr / tot + log2(tot);
    }
    void decode()
    {
        printf("Before decoding, Eleresult: %lu\n", Eleresult.size());
        if (fermatEle->Decode(Eleresult))
            printf("ele decode ok\n");
        printf("Eleresult: %lu\n", Eleresult.size());
        printf("Ele Fermat Inserted: %lu\n", EleFermatInserted.size());
        cout << "Decoded Rate: " << Eleresult.size()/EleFermatInserted.size() <<endl;
        

    }
    int query(const char *key)
    {
        if (Eleresult.count(*(uint32_t *)key))
        {
            return towerfilter->threshold + Eleresult[*(uint32_t *)key];
        }
        return towerfilter->query(key);
    }
    double get_entropy(vector<double> &distribution)
    {
        double entropy = 0.0;
        double tot = 0.0;
        double entr = 0.0;
        for (int i = 1; i < distribution.size(); i++)
        {
            if(distribution[i] < 1.0)
                continue;
            tot += i * (int)distribution[i];
            entr += i * distribution[i] * log2(i);
        }
        //tot = tot_packets;
        entropy = -entr / tot + log2(tot);
        return entropy;
    }
    void get_heavy_hitters(set<uint32_t> &hh)
    {
        for (auto i : Eleresult)
        {
            if (query((const char *)&i.first) >= HH_THRESHOLD)
                hh.insert(i.first);
        }
    }
    void diff(Fermat_tower &sketch2)
    {
        std::ofstream outfile1("./outputs/diff_f0.csv");
        std::ofstream outfile2("./outputs/diff_f1.csv");
        std::ofstream outfile3("./outputs/diff_f.csv");
        int array_num = fermatEle->getArrayNum();
        int entry_num = fermatEle->getEntryNum();

        Fermat::IdCountArrays Arrays1 = fermatEle->getArrays();  // 获取 id 和 count 两个数组
        uint32_t** idArray1 = Arrays1.idArray;
        uint32_t** countArray1 = Arrays1.countArray;

        Fermat::IdCountArrays Arrays2 = sketch2.fermatEle->getArrays();

        outfile1 << "id,count" << endl;
        outfile2 << "id,count" << endl;
        outfile3 << "id,count" << endl;
        cout << "PRIME_ID: " << PRIME_ID << endl;

        for (int i = 0; i < array_num; i++)
        {
            for (int j = 0; j < entry_num; j++)
            {
                outfile1 << idArray1[i][j] << "," << countArray1[i][j] << endl;
                outfile2 << Arrays2.idArray[i][j] << "," << Arrays2.countArray[i][j] << endl;
                Arrays2.idArray[i][j] = (PRIME_ID + idArray1[i][j] - Arrays2.idArray[i][j]) % PRIME_ID;
                Arrays2.countArray[i][j] = countArray1[i][j] - Arrays2.countArray[i][j];
                //sketch2.fermatEle->Insert(Arrays2.idArray[i][j], Arrays2.countArray[i][j]);
                EleFermatInserted[Arrays2.idArray[i][j]] ++;
                tot_packets++;
            }
        }
        sketch2.fermatEle->setArray(Arrays2);
        Fermat::IdCountArrays Arrays3 = sketch2.fermatEle->getArrays();
        for (int i = 0; i < array_num; i++)
        {
            for (int j = 0; j < entry_num; j++)
            {
                outfile3 << Arrays3.idArray[i][j] << "," << Arrays3.countArray[i][j] << endl;
            }
        }
    }
    void Union(Fermat_tower &sketch2)
    {     
        // std::ofstream outfile1("us0.csv");
        // std::ofstream outfile2("us1.csv");
        // std::ofstream outfile3("u.csv");
        int array_num = fermatEle->getArrayNum();
        int entry_num = fermatEle->getEntryNum();

        Fermat::IdCountArrays Arrays1 = fermatEle->getArrays();  // 获取 id 和 count 两个数组
        uint32_t** idArray1 = Arrays1.idArray;
        uint32_t** countArray1 = Arrays1.countArray;

        Fermat::IdCountArrays Arrays2 = sketch2.fermatEle->getArrays();  // 获取 id 和 count 两个数组

        // outfile1 << "id,count" << endl;
        // outfile2 << "id,count" << endl;
        // outfile3 << "id,count" << endl;
        // cout << "PRIME_ID: " << PRIME_ID << endl;

        for (int i = 0; i < array_num; i++)
        {
            for (int j = 0; j < entry_num; j++)
            {
                // outfile1 << idArray1[i][j] << "," << countArray1[i][j] << endl;
                // outfile2 << Arrays2.idArray[i][j] << "," << Arrays2.countArray[i][j] << endl;
                Arrays2.idArray[i][j] = (idArray1[i][j] + Arrays2.idArray[i][j]) % PRIME_ID;
                Arrays2.countArray[i][j] += countArray1[i][j];
            }
        }
        sketch2.fermatEle->setArray(Arrays2);
        // Fermat::IdCountArrays Arrays3 = sketch2.fermatEle->getArrays();
        // for (int i = 0; i < array_num; i++)
        // {
        //     for (int j = 0; j < entry_num; j++)
        //     {
        //         outfile3 << Arrays3.idArray[i][j] << "," << Arrays3.countArray[i][j] << endl;
        //     }
        // }
    }
};