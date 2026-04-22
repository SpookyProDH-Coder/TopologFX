#pragma once
#include "VertexLayouts.h"
#include "../../topology/MetricSpace.h"
#include "../core/Registry.h"

namespace Primitives 
{
    Mesh CreateCube();
    Mesh CreateFromMetricSpace(const MetricSpace&, float);
}