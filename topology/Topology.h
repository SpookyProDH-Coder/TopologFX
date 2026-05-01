#ifndef _H_TOPOS
#define _H_TOPOS

/**
 * Definition of an abstract topological space (X, Tau)
 */

#include <bx/bx.h>
#include <bx/allocator.h>
#include <cmath>
#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>

using namespace std;

template <typename Atom, typename Policy>
class Topology
{
    public:
        struct SetX 
        {
            bx::AllocatorI* allocator;
            Atom* elements;
            unsigned count;
        };

        Topology(bx::AllocatorI* _allocator, Policy _policy)
        : m_allocator(_allocator), m_policy(_policy)
        {
            m_X.elements = nullptr;
            m_X.count = 0;
            m_X.allocator = _allocator;
        }

        virtual ~Topology() { clean(); }

        virtual bool isOpen(const vector<unsigned>&) const = 0;
        virtual vector<unsigned> getNeighborhood(unsigned) const = 0;

        void generate(unsigned _numElements) 
        {
            clean();
            m_X.count = _numElements;
            m_X.elements = (Atom*)bx::alloc(m_allocator, sizeof(Atom) * _numElements);
        }

        void clean() 
        {
            if (m_X.elements) 
            {
                bx::free(m_allocator, m_X.elements);
                m_X.elements = nullptr;
            }
        }

        const SetX& getSet() const
        {
            return m_X;
        }

    protected:
        bx::AllocatorI* m_allocator;
        SetX m_X;
        Policy m_policy;
};

#endif