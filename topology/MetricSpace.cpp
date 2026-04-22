#include <cmath>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>

#include "Topology.h"
#include "MetricSpace.h"
#include "bx/allocator.h"

using namespace std;

typedef bx::Vec3 Vector;

MetricSpace::MetricSpace(bx::AllocatorI* _allocator, float norm, float _epsilon)
    : Topology<Vector, UsualTopologyPolicy>(_allocator, UsualTopologyPolicy(_epsilon))
    , m_norm(norm)
{}

float MetricSpace::distance(uint32_t _idxA, uint32_t _idxB) const
{
    float sum, result = 0;
    float dx, dy, dz;

    const Vector& a = m_X.elements[_idxA];
    const Vector& b = m_X.elements[_idxB];

    dx = abs(a.x - b.x);
    dy = abs(a.y - b.y);
    dz = abs(a.z - b.z);

    if (isinf(m_norm))
        result = std::max({dx, dy, dz});
    else if (m_norm == 1.0f) 
        result = dx + dy + dz;
    else
    {
        sum = pow(dx, m_norm) + pow(dy, m_norm) + pow(dz, m_norm);
        result = pow(sum, 1.0f / m_norm);
    }

    return result;
}

void MetricSpace::generate(unsigned _numElements, float _range)
{
    Topology<Vector, UsualTopologyPolicy>::generate(_numElements);
    float x, y, z;

    for (unsigned i = 0; i < _numElements; ++i)
    {
        x = ((float)rand() / RAND_MAX) * 2.0f * _range - _range;
        y = ((float)rand() / RAND_MAX) * 2.0f * _range - _range;
        z = ((float)rand() / RAND_MAX) * 2.0f * _range - _range;

        m_X.elements[i] = { x, y, z };
    }
}

bool MetricSpace::isOpen(const vector<unsigned>& _set) const
{
    if (_set.empty()) 
        return true;

    for (unsigned pointIdx : _set) 
    {
        // Revisamos todos los puntos del espacio total
        for (unsigned j = 0; j < m_X.count; ++j) 
        {
            float dist = distance(pointIdx, j);
            
            if (m_policy.isInNeighborhood(dist)) 
            {
                bool jIsInSet = std::find(_set.begin(), _set.end(), j) != _set.end();
                
                if (!jIsInSet) 
                    return false;
            }
        }
    }
    
    return true; 
}

vector<unsigned> MetricSpace::getNeighborhood(unsigned point_index) const
{
    vector<unsigned> neighborhood;
    float dist;

    for (unsigned i = 0; i < m_X.count; ++i)
    {
        dist = distance(point_index, i);
        
        if (m_policy.isInNeighborhood(dist)) 
            neighborhood.push_back(i);
    }

    return neighborhood;
}