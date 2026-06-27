/******* MetricSpace.h ***************************************************/ /**
 *
 * @file MetricSpace.h
 *
 * The representation of a combinational metric space (X,d) with an induced poligonal surface.
 * An extension of a base topology to provide tools for metric analysis.
 * 
 * @version 2.0
 * @author SpookyProDH-Coder
 * @date 13/06/2026
 ***************************************************************************/

#ifndef _H_COMBINATORIAL_METRIC
#define _H_COMBINATORIAL_METRIC

#include <vector>
#include <functional>
#include <memory>
#include <bx/math.h>
#include <unordered_map>
#include <ranges>

#include "Topology.h"
#include "TopologyPolicies.h"

typedef bx::Vec3 Vector;

/** 
 * Structure for storing the result of the decompotition of connected components.
 * Maps each atom/vertex to an unique cluster index.
 */
struct TopologicalClusters
{
    std::vector<unsigned> atom_to_cluster_id;
    unsigned total_clusters = 0;
};

class MetricSpace : public topos::Topology<unsigned, TopologyPolicies::WordSurfacePolicy>
{
    public:
        /**
         * @brief MetricSpace constructor.
         * @param allocator Pointer to the bx library memory allocator.
         * @param policy Topological word configurator.
         */
        MetricSpace(bx::AllocatorI*, const TopologyPolicies::WordSurfacePolicy&);

        /**
         * @brief Virtual destructor.
         */
        virtual ~MetricSpace() = default;

        /**
         * @brief The distance operator. Must comply with the axioms.
         * @param x First vertex identifier
         * @param y Second vertex identifier
         * @return float Distance between x and y
         */
        float distance(unsigned, unsigned) const;
        
        /**
         * @brief Checks if a given subset of vertices make an open set in the metric space.
         * @param _subset Vector of vertex identifiers, representant of the subset to evaluate.
         */
        bool isOpen(const std::vector<unsigned>&) const;
        
        /**
         * @brief Collects every inmediate neighbor given a specific vertex.
         * 
         * A subset U is open iff forall x in U, there exist a neighbour strictly contained in U.
         * 
         * @param vertex Target vertex identifier.
         * @return std::vector<unsigned> A list of neighbor vertex identifiers.
         */
        std::vector<unsigned> getNeighborhood(unsigned) const;

        /**
         * @brief Extracts the connected components using Breadth-First Search (BFS) algorithm.
         * @return TopologicalClusters A structure that maps vertices to their respective cluster identifiers.
         */
        TopologicalClusters getConnectedComponentsBFS() const;
        
        /**
         * @brief Extracts the connected components using Depth-First Search (BFS) algorithm.
         * @return TopologicalClusters A structure that maps vertices to their respective cluster identifiers.
         * @warning DEPRECATED (No topological cluster used, not optimized)
         */
        std::vector<std::vector<unsigned>> getConnectedComponentsDFS() const;

        /**
         * @brief Getter method for the geometric centroid.
         * @return Vector The position of the centroid,
         */
        Vector getCentroid() const 
        { return m_centroid; }

        /**
         * @brief Calculates the geometric centroid
         */
        void computeCentroid();

        /**
         * @brief Getter method for the adjacency list.
         * @return The reference to the adjacency list.
         */
        auto getPositions() const 
        { return std::views::iota(0u, m_uniqueVerticesCount); }

        /**
         * @brief Executes a callback function for each neighbour of a given vertex.
         * @param vertex Identifier of the source vertex.
         * @param callback Lambda function that receives each neighbor-id as a parameter.
         */
        void executeNeighborhoodQuery(unsigned, const std::function<void(unsigned)>&) const override;

        /**
         * @brief Getter method for the simplicial complex.
         * @return The reference to the simplicial complex.
         */
        const TopologyPolicies::SimplicialComplex& getComplex() const 
        { return m_complex; }

        inline void setNorm(float p) { m_norm = p; }
        /**
         * @brief Relocate a given coordinate point of the metric space by its p-norm.
         * Assigns a new norm and rebuilds both geometry and distances of the MetricSpace
         * @param p The new p-norm
         */
        void buildNorm(float);

    protected:
        /**
         * @brief Recursive helper method used in getConnectedComponentsDFS function.
         */
        void dfsHelper(unsigned, std::vector<bool>&, std::vector<unsigned>&) const;

    private:
        /**
         * @brief Method for building the adjacency list structure from the simplicial complex.
         */
        void buildAdjacencyStructure();
        
        mutable std::vector<float> m_distanceField;    ///< Distance matrix ( n x 1 )
        mutable unsigned m_currentCachedSource = 0xFFFFFFFF;

        /**
         * @brief Method for computting each distance before building the full metric space, applying Dijkstra algorithm
         */
        void precomputeDistances(unsigned) const;

        TopologyPolicies::SimplicialComplex m_complex;    ///< Simplicial complex associated.
        std::vector<std::vector<std::pair<unsigned, float>>> m_adjList;     ///< Adjacency list of the induced surface.
        unsigned m_uniqueVerticesCount;                   ///< Total count of the unique vertices.

        float m_norm = 2.0f;                                    ///< MetricSpace's p-norm
        const float (*m_vertexPositions)[3] = nullptr;          ///< Position of the oriented vertices
        std::vector<std::array<float, 3>> m_local_vertices;     ///< Vertex position storage to the local space of the complex
        std::vector<TopologyPolicies::Vec3> m_spatialCoords;    ///< Quotient map to 3D coordinates inmersion


        Vector m_centroid = {0,0,0};                      ///< MetricSpace's 3D position of its centroid.
};

#endif