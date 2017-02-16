/**
* @file sample.h
* @author Dounm <niuchong893184@gmail.com>
* @date 2017-02-13
*/

#ifndef PSW2V_SAMPLE_H
#define PSW2V_SAMPLE_H

#include <string>
#include <vector>
#include "util.h"

namespace psw2v {

class Sample {
public:
    Sample();
    Sample(size_t win_size);
    Sample(bool label, size_t target, size_t win_size);
    ~Sample() { }

    Sample(const Sample& other);
    Sample& operator=(const Sample& other);

    void set_label(bool label) {
        _label = label;
    }
    bool get_label() const {
        return _label;
    }
    void set_target(size_t target) {
        _target = target;
    }
    bool get_target() const {
        return _target;
    }

    void set_contexts(const std::vector<size_t>& wordids, 
            size_t begin, size_t end, size_t mid);
    size_t get_contexts_size() {
        return _contexts.size();
    }
    size_t get_context_by_id(size_t id) {
        return _contexts.at(id);
    }
    const std::vector<size_t>& get_contexts() const {
        return _contexts;
    }

private:
    bool _label;
    size_t _target;
    std::vector<size_t> _contexts;
};

}

#endif
