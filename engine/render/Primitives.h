#ifndef _H_PRIMITIVES
#define _H_PRIMITIVES

#include <vector>
#include "VertexLayouts.h"
#include "../../topology/MetricSpace.h"
#include "../../topology/TopologyPolicies.h"
#include "../core/Registry.h"

namespace Primitives 
{
    /* Mesh builders */
    Mesh MS_Plane(const MetricSpace&, float);

    Mesh QT_Space(const MetricSpace&, unsigned, const QuotientPolicy&, const Embedding<bx::Vec3>&);

    /* Auxiliars */
    Mesh buildMesh(const vector<PosColorVertex>&, const vector<unsigned>&);
};

#endif