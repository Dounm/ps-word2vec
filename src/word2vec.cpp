/**
* @file word2vec.cpp
* @author Dounm <niuchong893184@gmail.com>
* @date 2017-02-10
*/

#include "word2vec.h"
#include <stdlib.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <boost/algorithm/string.hpp>
#include "ps/ps.h"
#include "util.h"

#define CLEAR_SS(SS) \
        do { \
            SS.str(""); \
            SS.clear(); \
        } while(0) \

namespace psw2v {

void Word2Vec::load_dict(const std::string& dict_path) {
    std::ifstream fin(dict_path.c_str());
    DCHECK(fin.good()) << "[PS-WORD2VEC] cannot open dict file: " << dict_path << std::endl;
    size_t word_num = 0;
    std::string line;
    std::stringstream ss;

    getline(fin, line);
    ss << line;
    ss >> word_num;
    _word_ids.resize(word_num);
    _word_freqs.resize(word_num);

    size_t wordid = 0;
    size_t index = 0;
    size_t freq = 0;
    size_t line_cnt = 1;
    while (getline(fin, line)) {
        CLEAR_SS(ss);
        ss << line;
        ss >> wordid >> index >> freq;
        _word_ids[index] = wordid;
        _word_freqs[index] = freq;
        line_cnt ++;
    }
    std::cout  << "line_cnt: " << line_cnt << std::endl;

    size_t para_cnt = word_num * _vec_dims;
    std::vector<ps::Key> keys(para_cnt, 0);
    std::vector<float> vals(para_cnt, 0);      
    for (size_t i = 0;i < para_cnt; ++i) {
        keys[i] = rand() % para_cnt; 
        vals[i] = (rand_01() - 0.5) / sqrt(1.0 + _vec_dims); 
    }
    std::cout << "start push" << std::endl;
    _kv->Wait(_kv->Push(keys, vals));
    std::cout << "stop push" << std::endl;
    DLOG(INFO) << "[PS-WORD2VEC] rank " << ps::MyRank() << " has send the inited data";
    
    _negative.reserve(para_cnt);
    for (size_t i = 0;i < word_num; ++i) {
        size_t tmp = static_cast<size_t>(pow(_word_freqs[i], 0.75));
        for (size_t j = 0;j < tmp; ++j) {
            _negative.push_back(i);
        }
    }
    std::vector<size_t>(_negative).swap(_negative);     // swap trick to trim capacity
}

void Word2Vec::train(const std::string& data_path) {
    std::ifstream fin(data_path.c_str());
    DCHECK(fin.good()) << "[PS-WORD2VEC] cannot open data file: " << data_path << std::endl;
    std::string line;
    while(getline(fin, line)) {
        std::vector<std::string> svec;        
        boost::split(svec, line, boost::is_any_of("\t"));
        size_t word_size = svec.size();
        std::cout << "word_size: " << word_size << std::endl;
    }
}

}
