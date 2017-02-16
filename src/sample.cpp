/**
* @file sample.cpp
* @author Dounm <niuchong893184@gmail.com>
* @date 2017-02-13
*/

#include "sample.h"
#include "util.h"
#include "dlog.h"

namespace psw2v {

Sample::Sample() : _label(false), _target(0), _contexts() { }

Sample::Sample(size_t win_size) : _label(false), _target(0), _contexts() {
    _contexts.reserve(win_size);
}

Sample::Sample(bool label, size_t target, size_t win_size) :
        _label(label), _target(target), _contexts() {
    _contexts.reserve(win_size);
}

Sample::Sample(const Sample& other) {
    if (this == &other) return;
    _label = other._label;
    _target = other._target;
    _contexts = other._contexts;
}

Sample& Sample::operator=(const Sample& other) {
    if (this == &other) return *this;
    _label = other._label;
    _target = other._target;
    _contexts = other._contexts;
    return *this;
}

void Sample::set_contexts(const std::vector<size_t>& wordids, 
        size_t begin, size_t end, size_t mid) {
    DCHECK(end <= wordids.size()) << "[PSW2V] range exceeds";
    //TODO: which is better: assign() or below?
    for (size_t i = begin; i < end; ++i) {
        if (i == mid) continue;
        _contexts.push_back(wordids[i]);    // no unnecessary memory allocation
    }
}

}
