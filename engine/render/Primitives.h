/******* Primitives.h ***************************************************/ /**
 *
 * @file Primitives.h
 *
 * Definition of geometric and topological primitive generators
 *
 * @version 1.0
 * @author SpookyProDH-Coder
 * @date 16/06/2026
 ***************************************************************************/

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

    /**
     * @brief Generates a renderable Mesh by applying a geometric immersion to an abstract MetricSpace
     * 
     * It iterates over a 2D parameter space (U, V), extracts topological equivalence classes (quotient map)
     * to glue each vertex, and maps these abstract algebraic roots into a contiguous 1D array required by GPU index buffers.
     * 
     * @param U The discrete resolution along the parametric U-axis (Theta).
     * @param V The discrete resolution along the parametric V-axis (Phi).
     * @param useHeatap
     * @return Mesh A structure containing GPU handles for the generated vertex and index buffers
     */
    Mesh Surface(const MetricSpace&, unsigned, unsigned, bool);

    /* Coloring Auxiliars */
    /**
     * @brief Computes a chromatic discriminant for each vertex in a given complex.
     * 
     * It displays both the complex relation and orientability
     * 
     * @param id The vertex identifier
     * @param isOrientable Whether the complex is orientable or not
     */
    inline uint32_t colorSurface(unsigned, bool);

    /**
     * @brief Computes a chromatic discriminant depending on the absolute distance of a
     * given metric space point to its centroid.
     * 
     * @param distance The pre-computed distance
     * @param max_distance The gradient scaling factor
     */
    inline uint32_t colorDistance(float, float);

    /**
     * @brief Allocates GPU hardware buffers (VBO/IBO) from CPU memory vectors.
     * @param vertex Vector containing the populated vertex data layout.
     * @param indexes Vector containing the flattened 32-bit index geometry topology.
     * @param allocator Bgfx allocator interface.
     * @return Mesh A struct containing the valid bgfx resource handles
     */
    Mesh buildMesh(const PosColorVertex*, unsigned, const unsigned*, unsigned, bx::AllocatorI*);

    /**
     * This function is never called yet in the program.
     * 
     * @brief Updates GPU hardware buffers (VBO/IBO) from CPU memory vectors.
     * @param mesh A struct containing the valid bgfx resource handles
     * @param numVertices Number of the updated vertex layout.
     * @param new_indexes Updated vector containing the flattened 32-bit index geometry topology.
     * @param allocator Bgfx allocator interface.
     */
    void updateDynamicMesh(Mesh&, const PosColorVertex*, unsigned, bx::AllocatorI*);
};

#endif