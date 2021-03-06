#ifndef SUFFIX_GENERATOR_HPP
#define SUFFIX_GENERATOR_HPP
#pragma once

#include <cmath>
#include <string>

const int maxCountValue = 99;

class SuffixGenerator
{
public:
    static std::string GetNext()
    {
        i++;
        if (i == maxCountValue + 1)
            i = - maxCountValue;

        j = i;
        if (std::abs(i) < 10 && i < 0)
            return (std::abs(i) < 10 ? "-0" : "") + std::to_string(std::abs(i));
        else
            return (std::abs(i) < 10 ? "0" : "") + std::to_string(i);
    }

public:
    static int i;
    static int j;
};

#endif // SUFFIX_GENERATOR_HPP
