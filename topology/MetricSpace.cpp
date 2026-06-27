/******* MetricSpace.cpp ***************************************************/ /**
 *
 * @file MetricSpace.cpp
 * 
 * @version 2.0
 * @author SpookyProDH-Coder
 * @date 14/06/2026
 ***************************************************************************/

#include <queue>
#include <limits>
#include <algorithm>
#include <cassert>

#include "MetricSpace.h"

MetricSpace::MetricSpace(bx::AllocatorI* _allocator, const TopologyPolicies::WordSurfacePolicy& _policy)
: topos::Topology<unsigned, TopologyPolicies::WordSurfacePolicy>(_allocator, _policy)
{
    m_complex = m_policy.generateComplex();

    // Isomorphism to the quotient space X/~
    std::map<unsigned, unsigned> rootToContiguous;
    unsigned currentContiguousId = 0;

    /// 1. Remap the quotient map to a point
    for (unsigned& root : m_complex.quotientMap) 
    {
        if (rootToContiguous.find(root) == rootToContiguous.end()) 
            rootToContiguous[root] = currentContiguousId++;

        root = rootToContiguous[root];
    }

    /// 2. Update the simplices
    for (TopologyPolicies::Simplex1D& edge : m_complex.edges) 
    {
        edge.v0 = rootToContiguous[edge.v0];
        edge.v1 = rootToContiguous[edge.v1];
    }

    for (TopologyPolicies::Simplex2D& tri : m_complex.triangles) 
    {
        tri.v0 = rootToContiguous[tri.v0];
        tri.v1 = rootToContiguous[tri.v1];
        tri.v2 = rootToContiguous[tri.v2];
    }

    /// 3. Set the cardinality of the quotient space
    m_uniqueVerticesCount = currentContiguousId;
    m_complex.uniqueVerticesCount = currentContiguousId;

    this->allocateSet(m_uniqueVerticesCount);

    for (unsigned i = 0; i < m_uniqueVerticesCount; i++) 
        m_X.elements[i] = i; 

    buildAdjacencyStructure();
}

void MetricSpace::buildAdjacencyStructure()
{
    m_adjList.resize(m_uniqueVerticesCount);

    for (const TopologyPolicies::Simplex1D& edge : m_complex.edges) 
    {
        m_adjList[edge.v0].push_back({edge.v1, 1.0f});
        m_adjList[edge.v1].push_back({edge.v0, 1.0f});
    }

    // Remove corner duplicates 
    for (unsigned i = 0; i < m_uniqueVerticesCount; i++) 
    {
        std::sort(m_adjList[i].begin(), m_adjList[i].end());
        m_adjList[i].erase(std::unique(m_adjList[i].begin(), m_adjList[i].end()), m_adjList[i].end());
    }
}

float MetricSpace::distance(unsigned x, unsigned y) const
{
    assert(x < m_uniqueVerticesCount && y < m_uniqueVerticesCount);

    if (m_currentCachedSource != x)
        precomputeDistances(x);
    
    return m_distanceField[y];
}

void MetricSpace::executeNeighborhoodQuery(unsigned _atomIdx, const std::function<void(unsigned)>& _internalCallback) const
{
    assert(_atomIdx < m_uniqueVerticesCount && "Target atom does not exist in the quotient space.");
    for (const auto& edge : m_adjList[_atomIdx])
        _internalCallback(edge.first);
}

std::vector<unsigned> MetricSpace::getNeighborhood(unsigned _atomIdx) const
{
    assert(_atomIdx < m_uniqueVerticesCount && "Target atom does not exist in the quotient space.");
    std::vector<unsigned> result(m_adjList[_atomIdx].size());

    for (const auto& edge : m_adjList[_atomIdx])    // Should be optimized
        result.push_back(edge.first);
    return result;
}

bool MetricSpace::isOpen(const std::vector<unsigned>& _subset) const
{
    for (unsigned x : _subset)
        for (const auto& neighbor : m_adjList[x])
            if (std::find(_subset.begin(), _subset.end(), neighbor.first) == _subset.end())
                return false; 

    return true;
}

TopologicalClusters MetricSpace::getConnectedComponentsBFS() const
{
    TopologicalClusters clusters;
    clusters.atom_to_cluster_id.assign(m_uniqueVerticesCount, std::numeric_limits<unsigned>::max());
    unsigned i, u, v, currentClusterId = 0;
    std::queue<unsigned> q;

    for (i = 0; i < m_uniqueVerticesCount; i++) 
    {
        if (clusters.atom_to_cluster_id[i] != std::numeric_limits<unsigned>::max()) 
            continue;

        q.push(i);
        clusters.atom_to_cluster_id[i] = currentClusterId;

        while (!q.empty()) 
        {
            u = q.front();
            q.pop();

            for (const auto& edge : m_adjList[u]) 
            {
                v = edge.first;
                if (clusters.atom_to_cluster_id[v] == std::numeric_limits<unsigned>::max()) 
                {
                    clusters.atom_to_cluster_id[v] = currentClusterId;
                    q.push(v);
                }
            }
        }
        currentClusterId++;
    }

    clusters.total_clusters = currentClusterId;
    return clusters;
}

std::vector<std::vector<unsigned>> MetricSpace::getConnectedComponentsDFS() const
{
    std::vector<std::vector<unsigned>> components;
    std::vector<bool> visited(m_uniqueVerticesCount, false);

    for (unsigned i = 0; i < m_uniqueVerticesCount; i++)
    {
        if (!visited[i])
        {
            std::vector<unsigned> component;
            dfsHelper(i, visited, component);
            components.push_back(component);
        }
    }
    return components;
}

void MetricSpace::dfsHelper(unsigned _origen, std::vector<bool>& _visited, std::vector<unsigned>& _component) const
{
    _visited[_origen] = true;
    _component.push_back(_origen);

    for (const auto& v : m_adjList[_origen])
        if (!_visited[v.first])
            dfsHelper(v.first, _visited, _component);
}

void MetricSpace::computeCentroid()
{
    if (m_uniqueVerticesCount == 0)
    {
        m_centroid = { 0, 0, 0 };
        return;
    }

    float totalX, totalY, totalZ;
    totalX = totalY = totalZ = 0;

    for (unsigned i = 0; i < m_uniqueVerticesCount; i++)
    {
        totalX += m_vertexPositions[i][0];
        totalY += m_vertexPositions[i][1];
        totalZ += m_vertexPositions[i][2];
    }

    m_centroid.x = totalX / m_uniqueVerticesCount;
    m_centroid.y = totalY / m_uniqueVerticesCount;
    m_centroid.z = totalZ / m_uniqueVerticesCount;
}

void MetricSpace::precomputeDistances(unsigned source) const
{
    assert(m_vertexPositions != nullptr || m_uniqueVerticesCount == 0);

    if (m_currentCachedSource == source || source >= m_uniqueVerticesCount)
        return;
    
    m_currentCachedSource = source;

    std::fill(m_distanceField.begin(), m_distanceField.end(), std::numeric_limits<float>::infinity());
    m_distanceField[source] = 0.0f;
    
    using DistNode = std::pair<float, unsigned>;
    std::priority_queue<DistNode, std::vector<DistNode>, std::greater<DistNode>> pq;
    pq.push({0.0f, source});

    auto run_dijkstra = [&](auto dist_func) 
    {
        float new_dist;
        unsigned v;

        while (!pq.empty())
        {
            auto [current_dist, u] = pq.top();
            pq.pop();

            if (current_dist <= m_distanceField[u]) 
            {
                for (const auto& edge : m_adjList[u])
                {
                    v = edge.first;

                    new_dist = current_dist + dist_func(m_vertexPositions[u], m_vertexPositions[v]);

                    if (new_dist < m_distanceField[v])
                    {
                        m_distanceField[v] = new_dist;
                        pq.push({new_dist, v});
                    }
                }
            }
        }
    };

    if (m_norm == 1.0f) 
    {
        run_dijkstra([](const float* p1, const float* p2) -> float {
            return std::abs(p1[0] - p2[0]) + std::abs(p1[1] - p2[1]) + std::abs(p1[2] - p2[2]);
        });
    }

    else if (m_norm == 2.0f) 
    {
        run_dijkstra([](const float* p1, const float* p2) -> float {
            float dx = p1[0] - p2[0], dy = p1[1] - p2[1], dz = p1[2] - p2[2];
            return std::sqrt(dx*dx + dy*dy + dz*dz);
        });
    }
   
    else if (std::isinf(m_norm)) 
    {
        run_dijkstra([](const float* p1, const float* p2) -> float {
            return std::max({std::abs(p1[0] - p2[0]), std::abs(p1[1] - p2[1]), std::abs(p1[2] - p2[2])});
        });
    }

    else
    {
        run_dijkstra([this](const float* p1, const float* p2) -> float {
            float dx = std::abs(p1[0] - p2[0]);
            float dy = std::abs(p1[1] - p2[1]);
            float dz = std::abs(p1[2] - p2[2]);
            return std::pow(
                std::pow(dx, m_norm) + std::pow(dy, m_norm) + std::pow(dz, m_norm), 
                1.0f / m_norm
            );
        });
    }
}

void MetricSpace::buildNorm(float p)
{
    unsigned U = m_complex.gridU;
    unsigned V = m_complex.gridV;
    unsigned u, v, logicalIdx, realId;
    float pu, pv;

    if (U == 0 || V == 0) return;

    m_norm = p;

    TopologyPolicies::EmbeddingFunction embed = m_policy.getEmbedding();
    m_spatialCoords.resize(m_uniqueVerticesCount);

    TopologyPolicies::Vec3 pos;

    // Evaluate parametric surface to find each quotient vertex
    for (v = 0; v <= V; v++)
    {
        for (u = 0; u <= U; u++)
        {
            logicalIdx = v * (U + 1) + u;
            realId = m_complex.quotientMap[logicalIdx];

            // [0, Subdivitions] -> [-1.0, 1.0] Embedding mapping
            pu = ((float)u / U) * 2.0f - 1.0f;
            pv = ((float)v / V) * 2.0f - 1.0f;

            pos = embed(pu, pv);
            
            m_spatialCoords[realId].x = pos.x;
            m_spatialCoords[realId].y = pos.y;
            m_spatialCoords[realId].z = pos.z;
        }
    }

    m_vertexPositions = reinterpret_cast<const float(*)[3]>(m_spatialCoords.data());

    m_distanceField.assign(m_uniqueVerticesCount, std::numeric_limits<float>::infinity());
    m_currentCachedSource = 0xFFFFFFFF;
}