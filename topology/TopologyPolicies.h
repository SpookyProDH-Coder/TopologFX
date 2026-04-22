#ifndef _H_TOPOLICIES
#define _H_TOPOLICIES

#include <bx/allocator.h>
#include <vector>
#include <algorithm>

using namespace std;

struct UsualTopologyPolicy 
{
    float epsilon;
    explicit UsualTopologyPolicy(float _e) : epsilon(_e) {}

    bool isInNeighborhood(float distance) const 
    {
        return distance < epsilon;
    }
};

struct SorgenfreyPolicy 
{
    float h;
    SorgenfreyPolicy(float _h) : h(_h) {}

    bool isInNeighborhood(float a, float b) const 
    {
        return (b >= a) && (b < a + h);
    }
};

struct ParticularPointPolicy
{
    unsigned p_index;

    explicit ParticularPointPolicy(unsigned p) : p_index(p) {}

    bool isOpen(const vector<unsigned>& _subset) const 
    {
        bool ok = false;
        if (_subset.empty()) 
            ok = true;
        else
            ok = std::find(_subset.begin(), _subset.end(), p_index) != _subset.end();
        return ok;
    }
};

#endif