/**
* @file word2vec.h
* @author Dounm <niuchong893184@gmail.com>
* @date 2017-02-09
*/

#ifndef PSW2V_WORD2VEC_H
#define PSW2V_WORD2VEC_H

#include <string>
#include <vector>
#include <memory>
#include "ps/ps.h"
#include "util.h"

namespace psw2v {

class Word2Vec {
public:
    Word2Vec(size_t iters, size_t batch_size,
            size_t vec_dims, size_t win_size, float learning_rate) : 
            _iters(iters), _batch_size(batch_size), 
            _vec_dims(vec_dims), _win_size(win_size), _learning_rate(learning_rate),
            _kv(new ps::KVWorker<float>(0)) { }
    ~Word2Vec() { }

    void load_dict(const std::string& dict_path);
    void train(const std::string& data_path);

    
private:
    DISALLOW_COPY_AND_ASSIGN(Word2Vec);
    
    size_t _iters;
    size_t _batch_size;
    size_t _vec_dims;
    size_t _win_size;
    float _learning_rate;

    std::vector<size_t> _word_ids;
    std::vector<size_t> _word_freqs;
    std::vector<size_t> _negative;

    std::shared_ptr<ps::KVWorker<float> > _kv;
};

}

#endif
