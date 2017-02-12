/**
* @file util.h
* @author Dounm <niuchong893184@gmail.com>
* @date 2017-02-09
*/

#ifndef PSW2V_UTIL_H
#define PSW2V_UTIL_H

#define DIALLOW_COPY_AND_ASSIGN(TypeName) \
        TypeName(const TypeName&); \
        TypeName& operator=(const TypeName&)

#define ERROR_LOG(content, arg...) do { \
    printf("[%s:%d %s()]" content "\n", __FILE__, __LINE__, __FUNCTION__, ## arg) \
} while(0)

#include <stdio.h>

namespace psw2v {

static const double RAND_RANGE = 1e5;

inline float rand_01() {
    return (rand() - RAND_RANGE) / RAND_RANGE;
}

}

#endif
