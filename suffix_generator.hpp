#ifndef SUFFIX_GENERATOR_HPP
#define SUFFIX_GENERATOR_HPP
#pragma once

#include <cmath>
#include <string>

class SuffixGenerator
{
public:
    static std::string GetNext()
    {
        i++;
        if (i == 100)
            i = - 99;

        j = i;
        return (std::abs(i) < 10 ? "0" : "") + std::to_string(i);
    }

public:
    static int i;
    static int j;
};

#endif // SUFFIX_GENERATOR_HPP
