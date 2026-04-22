#ifndef _H_METRIC
#define _H_METRIC

#include <bx/math.h>
#include <fstream>
#include "Topology.h"
#include "TopologyPolicies.h"

typedef bx::Vec3 Vector;

class MetricSpace : public Topology<Vector, UsualTopologyPolicy>
{
    public:
        MetricSpace(bx::AllocatorI*, float, float);
        virtual ~MetricSpace() = default;

        float distance(unsigned, unsigned) const;
        bool isOpen(const vector<unsigned>&) const override;
        vector<unsigned> getNeighborhood(unsigned) const override;

        void generate(unsigned, float);
        void load(ostream&);

    private:
        float m_norm;
};

#endif