/**
* @file word2vec.cpp
* @author Dounm <niuchong893184@gmail.com>
* @date 2017-02-10
*/

#include "word2vec.h"
#include <stdlib.h>
#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "ps/ps.h"
#include "util.h"
#include "sample.h"

#define CLEAR_SS(S) \
        do { \
            S.str(""); \
            S.clear(); \
        } while(0) \

namespace psw2v {

inline void sum_vector(std::vector<float>* sum_vec, const std::vector<float>& vec) {
    DCHECK(sum_vec->size() == vec.size()) << "[PSW2V] vecs to be added don't have same dims";
    for (size_t i = 0; i < vec.size(); ++i) {
        sum_vec->at(i) += vec[i];
    }
}

inline std::vector<float> multi_vector(float x, const std::vector<float>& vec) {
    std::vector<float> res(vec.size(), 0);
    for (size_t i = 0; i < vec.size(); ++i) {
        res[i] = x * vec[i];
    }
    return res;     // no need worry for overhead due to RVO
}

inline float dot(const std::vector<float>& vec1, const std::vector<float>& vec2) {
    DCHECK(vec1.size() == vec2.size()) << "[PSW2V] vecs to be dot multiplied don't have same dims";
    float res = 0;
    for (size_t i = 0; i < vec1.size(); ++i) {
        res += vec1[i] * vec2[i];
    }
    return res;
}

//TODO: reduce time of sigmoid() in the google source code way
inline float sigmoid(float x) {
    const float SIGMOID_MIN = -6.0; 
    const float SIGMOID_MAX = 6.0;
    if (x < SIGMOID_MIN) return -1.0;
    if (x > SIGMOID_MAX) return 1.0;
    return 1.0/(1.0+exp(0-x));
}

void Word2Vec::load_dict(const std::string& dict_path) {
    std::ifstream fin(dict_path.c_str());
    DCHECK(fin.good()) << "[PS-WORD2VEC] cannot open dict file: " << dict_path << std::endl;
    size_t word_num = 0;
    std::string line;
    std::stringstream ss;

    getline(fin, line);
    ss << line;
    ss >> word_num;
    _words.resize(word_num);
    _word_freqs.resize(word_num);

    std::string word;
    size_t index = 0;
    size_t freq = 0;
    size_t line_cnt = 1;
    while (getline(fin, line)) {
        CLEAR_SS(ss);
        ss << line;
        ss >> word >> index >> freq;
        _words[index] = word;
        _word_freqs[index] = freq;
        line_cnt ++;
    }

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
        boost::split(svec, line, boost::is_any_of(" "));
        size_t sample_num = svec.size();

        std::vector<size_t> wordids(sample_num);
        for (size_t i = 0;i < sample_num; ++i) {
            size_t wordid = boost::lexical_cast<size_t>(svec[i]);
            wordids.at(i) = wordid;
        }
        
        std::vector<Sample> samples;
        // TODO: which is better: resize() or reserve()?
        samples.resize(sample_num*(_win_size+1), Sample(2*_win_size));
        size_t cnt = 0;
        for(size_t i = 0;i < sample_num; ++i) {
            if(skip_high_freq_word(wordids[i])) continue;
            int tmp = static_cast<int>(i) - static_cast<int>(_win_size);
            size_t begin = (0 > tmp) ? 0 : tmp;
            size_t end = (i+_win_size+1 < sample_num) ? i+_win_size+1 : sample_num;
            samples[cnt].set_contexts(wordids, begin, end, i);
            samples[cnt].set_label(true);
            samples[cnt].set_target(wordids[i]);
            cnt ++;

            // negative sampling: sample '_win_size' negative samples
            for (size_t j = 0;j < _win_size; ++j) {
                size_t a = rand();
                size_t b = _negative.size();
                size_t tmp = a % b;
                if (_negative[tmp] == wordids[i]) continue;
                if (skip_high_freq_word(_negative[tmp])) continue;
                
                samples[cnt].set_contexts(wordids, begin, end, i);
                samples[cnt].set_label(false);
                samples[cnt].set_target(_negative[tmp]);
                cnt ++;
            }
        }
        samples.erase(samples.begin()+cnt, samples.end());
        std::cout << "real sample nums:" << samples.size() << std::endl;

        
        std::cout << "start train single batch" << std::endl;
        size_t batch_num = samples.size() / _batch_size;
        for (size_t i = 0;i <= batch_num; ++i) {    // remainder as a single sample
            size_t begin = i * _batch_size;
            size_t end = std::min((i+1)*_batch_size, samples.size());
            std::vector<Sample> single_batch(samples.begin()+begin, samples.begin()+end);
            train_single_batch(single_batch);
        }
    }
    if (ps::MyRank() == 0) {
        //TODO: save model after all workers has training done
        save_model("psw2v.model");
    }
}

void Word2Vec::save_model(const std::string& out_path) {
    std::ofstream fout(out_path.c_str());
    for (size_t wordid = 0;wordid < _words.size(); ++wordid) {
        if (_word_freqs.at(wordid) < 5) continue;
        std::vector<ps::Key> keys(_vec_dims, 0);
        for (size_t j = 0;j < _vec_dims; ++j) {
            keys[j] = j + wordid * _vec_dims;
        }
        std::vector<float> values;
        _kv->Wait(_kv->Pull(keys, &values));
        fout << wordid << '\t' << _words[wordid] << '\t';
        for (auto value : values) {
            fout << value << ' ';
        }
        fout << std::endl;
    }
}
 
// private functions

bool Word2Vec::skip_high_freq_word(size_t word) {
    float freq = static_cast<float>(_word_freqs[word]);
    float t = 1000.0 / freq;   // actualy, it's "t / (freq / sum(freq))" in the equation
    float prob = sqrt(t) + t;
    if (rand_01() < prob) {
        return false;
    } else {
        return true;
    }
}

void Word2Vec::train_single_batch(const std::vector<Sample>& batch) {
    if (batch.size() == 0) return;
    std::unordered_map<size_t, std::vector<float> > paras;
    std::unordered_map<size_t, std::vector<float> > grads;

    // get words whose paras would be updated by this batch
    std::set<size_t> words_set;
    for (const auto& sample: batch) {
        words_set.insert(sample.get_target());
        for (auto con_word : sample.get_contexts()) {
            words_set.insert(con_word);
        }
    }

    // pull down origin paras of words, and keys must be ordered
    std::vector<ps::Key> keys;
    keys.reserve(words_set.size() * _vec_dims);
    for (auto wordid : words_set) {
        for(size_t i = wordid*_vec_dims; i < (wordid+1)*_vec_dims; ++i) {
            keys.push_back(i);
        }
    }
    std::vector<float> values;
    _kv->Wait(_kv->Pull(keys, &values));
    DCHECK(values.size() == keys.size()) << "[PSW2V] not one val for one key";

    // set origin paras to 'paras'
    size_t i = 0;
    for (auto wordid : words_set) {
        std::vector<float> para(values.begin() + i * _vec_dims,
                                values.begin() + (i + 1) * _vec_dims);
        paras.insert(std::make_pair(wordid, para));
        i += 1;
    }
    DCHECK(i == words_set.size()) << "[PSW2V] fail when setting origin paras to 'paras'";

    // cal grads
    for (const auto& kv : paras) {
        grads[kv.first] = std::vector<float>(_vec_dims, 0);
    }
    DCHECK(grads.size() == words_set.size()) << "[PSW2V] fail when initializing 'grads'";
    for (const auto& sample : batch) {
        train_single_sample(sample, paras, &grads);
    }

    // push grads
    i = 0;
    for (auto wordid : words_set) {
        auto it = grads.find(wordid);
        DCHECK(it != paras.end()) << "[PSW2V] cannot find the grads of wordid: " << wordid;
        const auto& word_grads = it->second;
        for (size_t j = 0; j < _vec_dims; ++j) {
            values.at(i*_vec_dims + j) = word_grads.at(j);
        }
        i++;
    }
    _kv->Wait(_kv->Push(keys, values));
}

void Word2Vec::train_single_sample(const Sample& sample, 
        const std::unordered_map<size_t, std::vector<float> >& paras,
        std::unordered_map<size_t, std::vector<float> >* grads) {
    // float lambda = 1e-5;
    size_t tword = sample.get_target();
    const auto& cwords = sample.get_contexts();
    
    auto it = paras.find(tword);
    DCHECK(it != paras.end()) << "[PSW2V] cannot find the paras of wordid: " << tword;
    const auto& tword_paras = it->second;

    std::vector<float> x_w(_vec_dims, 0);
    for(auto cword : cwords) {
        it = paras.find(cword); 
        DCHECK(it != paras.end()) << "[PSW2V] cannot find the paras of wordid: " << cword;
        const auto& cword_paras = it->second;
        sum_vector(&x_w, cword_paras);
    }

    float pred = dot(x_w, tword_paras);
    pred = sigmoid(pred);
    float label = sample.get_label() ? 1 : 0 ;
    float error = _learning_rate * (label - pred);

    // update grad
    auto it2 = grads->find(tword);
    DCHECK(it2 != paras.end()) << "[PSW2V] cannot find the grads of wordid: " << tword;
    auto& tword_grads = it2->second;

    std::vector<float> e = multi_vector(error, tword_paras);    //e = e + g * \theta^u
    auto tmp = multi_vector(error, x_w);
    sum_vector(&tword_grads, tmp);  // \theta^u = \theta^u + g * X_w
    for(auto cword : cwords) {
        auto it = grads->find(cword); 
        DCHECK(it != grads->end()) << "[PSW2V] cannot find the grads of wordid: " << cword;
        auto& cword_grads = it->second;

        sum_vector(&cword_grads, e);    // v(u) = v(u) + e
    }
}

}
