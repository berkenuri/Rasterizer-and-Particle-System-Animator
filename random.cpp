#ifndef __RANDOM_CPP__
#define __RANDOM_CPP__

#include "random.h"

std::mt19937 Random::s_RandomEngine;
std::uniform_int_distribution<std::mt19937::result_type> Random::s_Distribution;

#endif
