#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>
#include <thread>
#include <cmath>
#include <sstream>
#include <map>
#include <stdlib.h>
#include <mutex>
#include <atomic>
#include <algorithm>
using std::cout;
using std::endl;
void print(std::string Info){
    cout<<Info<<endl;
}

inline std::chrono::high_resolution_clock::time_point get_current_time_fenced()
{
    std::atomic_thread_fence(std::memory_order_seq_cst);
    auto res_time = std::chrono::high_resolution_clock::now();
    std::atomic_thread_fence(std::memory_order_seq_cst);
    return res_time;
}

template<class D>
inline long long to_us(const D& d)
{
    return std::chrono::duration_cast<std::chrono::microseconds>(d).count();
}


std::vector<std::string> SplitStringToVector(std::istream& file){
    std::vector<std::string> VectorOfResult;
    std::string word;
    while( file >> word )
        VectorOfResult.push_back(word);
    return VectorOfResult;
}

std::map <std::string,long long> Histograma(const std::vector<std::string>& Data){
    std::map <std::string,long long>WordMap;
    for(auto& w: Data){
            ++WordMap[w]; }
    return WordMap;
}

void HistogramaThreads(const std::vector<std::string>& Data,long long start,long long end,std::mutex& m,std::map <std::string,long long> &Result){
    std::map <std::string,long long> localMap;
    for(int i = start; i < end;i++){
        ++localMap[Data[i]];//////////////

    }
    std::lock_guard<std::mutex> lg(m);
    for (std::map<std::string,long long>::iterator it=localMap.begin(); it!=localMap.end(); it++){
        Result[it->first] = it->second;
    }
}


std::vector<std::string> VectorOfData(std::string NameOfFile){
    //NameOfFile --> string  of file name
    //It gets Vector of words from file.
    //Stop Main if "Error opening file"(with code 1)
    std::ifstream conf_file(NameOfFile);//"Conf.txt"
    if(!conf_file.is_open())
    { std::cerr << "Error opening file " << std::endl;
        exit(1); }
    std::vector<std::string> VectorOfConfiguration = SplitStringToVector(conf_file);
    return VectorOfConfiguration;
}

void WriteHistogramaToFile(std::vector<std::pair<std::string, long long>> &VectorOfWords, const std::string& NameOfFile) {
    //std::ofstream fs(NameOfFile, std::ios_base::app | std::ios_base::out);
    std::ofstream fs(NameOfFile);
    for (auto &wordPair : VectorOfWords) {
        fs << wordPair.first << " : " << wordPair.second << std::endl;
    }
}


std::vector<std::pair<std::string, long long>> SortVectorOfWordsByHistogram(const std::map<std::string, long long> &WordMap){
    std::vector<std::pair<std::string, long long>> mapVec;
    for (auto &mapPair : WordMap) {
        mapVec.push_back(mapPair);
    }
    std::sort(mapVec.begin(), mapVec.end(),
              [](std::pair<std::string, long long> &a, std::pair<std::string, long long> &b) {
                  return a.second > b.second;
              });

    return mapVec;
}




std::vector<std::pair<std::string, long long>> SortVectorOfWordsByAlpa(const std::map<std::string, long long> &WordMap){
    std::vector<std::pair<std::string, long long>> mapVec;
    for (auto &mapPair : WordMap) {
        mapVec.push_back(mapPair);
    }
    std::sort(mapVec.begin(), mapVec.end(),
              [](std::pair<std::string, long long> &a, std::pair<std::string, long long> &b) {
                  return a.first < b.first;
              });

    return mapVec;
}






int main(int argc, char *argv[]){
    std::vector<std::string> VectorOfConfiguration = VectorOfData("Conf.txt");
    int Nthreads = 0;
    std::string FileForWritingSortingByNumber;
    std::string FileForWritingSortingByAlphabet;
    std::string DataFile;
    bool key = true;
    for(int i = 0; i < VectorOfConfiguration.size();i++) {
        if (i + 2 < VectorOfConfiguration.size() && (VectorOfConfiguration[i] == "NThreds" || VectorOfConfiguration[i] == "FileForWritingSortingByNumber" || VectorOfConfiguration[i] == "FileForWritingSortingByAlphabet" || VectorOfConfiguration[i] == "FileData" ) && VectorOfConfiguration[i + 1] == "=="  )
        {
            key = false;
            if(VectorOfConfiguration[i] == "NThreds"){
                Nthreads = atoi(VectorOfConfiguration[i + 2].c_str());
            }
            else if(VectorOfConfiguration[i] == "FileForWritingSortingByNumber"){
                FileForWritingSortingByNumber = VectorOfConfiguration[i + 2];
            }
            else if(VectorOfConfiguration[i] == "FileForWritingSortingByAlphabet"){
                FileForWritingSortingByAlphabet = VectorOfConfiguration[i + 2];
            }
            else if(VectorOfConfiguration[i] == "FileData"){
                DataFile = VectorOfConfiguration[i + 2];
            }
        }
        else if(key)
        {
            print("Incorrect configuration file!");
        }
    }
    auto stage1_start_time = get_current_time_fenced();
    std::vector<std::string> VectorListOfData = VectorOfData("Data.txt");
    auto stage2_start_time = get_current_time_fenced();
    auto stage1_time = stage2_start_time - stage1_start_time;
    float finish1 = to_us(stage1_time);
    std::cout<< "Time of reading words from file == "<<finish1 / 1000000 <<std::endl;//It will be in seconds ==> finish1 / 1000000 ceconds.


    auto stage1_start1_time = get_current_time_fenced();
    std::map<std::string,long long> Result;
    int lenO = VectorListOfData.size()%Nthreads;
    std::vector<std::string> VectorListHelp;
    //print(std::to_string(lenO));
    std::map<std::string, long long> WordMap;
    if(lenO != 0){
        for(int i = 0; i < lenO + 1; i++){
            ++WordMap[VectorListOfData[i]];
            VectorListOfData.erase(VectorListOfData.begin());
        }
    }   long long limit = VectorListOfData.size()/Nthreads;
        long long start = 0;
        long long end = limit;
        std::mutex m;
        //std::map<std::string, long long> WordMap;
        std::vector<std::thread> threads;
        for (int i = 0; i < Nthreads; i++) {   //const std::vector<std::string>& Data,long long start,long long end,std::mutex& m,std::map <std::string,long long> &Result){
            threads.emplace_back(std::thread(HistogramaThreads, VectorListOfData, start,end, std::ref(m), std::ref(WordMap)));
            start += limit;
            end += limit;
            if(i == Nthreads -1){
                threads.emplace_back(std::thread(HistogramaThreads, VectorListOfData, start,VectorListOfData.size(), std::ref(m), std::ref(WordMap)));
            }
        }
        for (auto &thread: threads) {
            thread.join(); }
    auto stage2_start2_time = get_current_time_fenced();
    auto stage1_2time = stage2_start2_time - stage1_start1_time;
    float finish2 = to_us(stage1_2time);
    std::cout<< "Time of counting "<<"(Nthreads == "<<Nthreads<<" )"<<" words(Histograma) == "<<finish2 / 1000000 <<std::endl;
    auto stage3_start_time = get_current_time_fenced();

    std::vector<std::pair<std::string, long long>>ResVector1 = SortVectorOfWordsByHistogram(WordMap);
    std::vector<std::pair<std::string, long long>>ResVector2 = SortVectorOfWordsByAlpa(WordMap);
    WriteHistogramaToFile(ResVector1, "CountingNumber.txt");
    WriteHistogramaToFile(ResVector2, "Alfabet.txt");

    auto stage3_start2_time = get_current_time_fenced();
    auto stage3_2time = stage3_start2_time - stage3_start_time;
    float finish3 = to_us(stage3_2time);
    std::cout<< "Time of writing "<<"(Nthreads == "<<Nthreads<<" )"<<" words to the file == "<<finish3 / 1000000 <<std::endl;
    std::cout<<"Total == "<<finish3 / 1000000 + finish2 / 1000000 + finish1/1000000 <<std::endl;
    return 0;

}
