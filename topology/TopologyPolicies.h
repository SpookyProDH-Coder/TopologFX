/******* TopologyPolicies.h ***************************************************/ /**
 *
 * @file TopologyPolicies.h
 *
 * A collection of structures to modelize, embed and induce topological data structures in the real space.
 * It includes:
 * - Simplicial complexes
 * - Disjoint Set Union (DSU)
 * - Poligonal surfaces (WordSurfacePolicy)
 * - Built-in torus, klein, projective plane, sphere and mobis strip. With its respective geometrical embeddings.
 *  
 * @version 2.0
 * @author SpookyProDH-Coder
 * @date 13/06/2026
 ***************************************************************************/

#ifndef _H_TOPOLICIES
#define _H_TOPOLICIES

#include <vector>
#include <map>
#include <numeric>
#include <queue>
#include <cassert>
#include <cmath>
#include <bx/math.h>

/**
 * TODO: Remove <bx/math.h> include
 */

namespace TopologyPolicies
{
    /** A get-arround vector of 3 dimensions for not using templates */
    struct Vec3 {float x, y, z; };
    /**
     * @brief Standard parametritation signature for R³ embedding
     * @param cu
     * @param su
     * @param cv
     * @param sv
     */
    using EmbeddingFunction = Vec3(*)(float, float);

    struct Simplex1D { unsigned v0, v1; };
    struct Simplex2D { unsigned v0, v1, v2; };

    struct SimplicialComplex
    {
        std::vector<unsigned> quotientMap;
        std::vector<Simplex1D> edges;
        std::vector<Simplex2D> triangles;
        unsigned uniqueVerticesCount = 0;
        bool isOrientable = true;
        unsigned gridU = 0;
        unsigned gridV = 0;
    };

    /**
     * @brief A letter of a poligonal P_2n
     */
    struct WordLetter
    {
        char symbol;
        int8_t sign; // 1 normal; -1 inverse
    };

    struct DSU
    {
        std::vector<unsigned> parent;
        DSU(unsigned n) 
        {
            parent.resize(n);
            std::iota(parent.begin(), parent.end(), 0);
        }
        unsigned find(unsigned i) 
        {   // Path compression
            return (parent[i] == i) ? i : (parent[i] = find(parent[i]));
        }
        void unite(unsigned i, unsigned j) 
        {
            unsigned root_i = find(i);
            unsigned root_j = find(j);
            if (root_i != root_j) 
                parent[root_i] = root_j;
        }
    };

    class WordSurfacePolicy
    {
        public:
            // Receives the fundamental word as a 4-letter vector
            WordSurfacePolicy(const std::vector<WordLetter>& word, unsigned segmentsU, unsigned segmentsV, EmbeddingFunction embedding)
            : m_word(word), m_segmentsU(segmentsU), m_segmentsV(segmentsV), m_embedding(embedding)
            {
                assert(word.size() == 4 && "For a bidimensional grid, word must have 4 letters.");
            }

            inline EmbeddingFunction getEmbedding() const
            { return m_embedding; }

            inline std::vector<WordLetter> getWord() const
            { return m_word; }

            SimplicialComplex generateComplex() const
            {
                SimplicialComplex complex;
                unsigned totalGridVertices = (m_segmentsU + 1) * (m_segmentsV + 1);
                unsigned i, u, v, e1, e2, len, v1, v2, root, uniqueVertices;
                unsigned bl, br, tl, tr;
                unsigned m_bl, m_br, m_tl, m_tr;
                bool reverse;
                
                DSU dsu(totalGridVertices);
                std::vector<std::vector<unsigned>> paths(4);
                std::map<char, std::vector<unsigned>> symbols;
                std::vector<int> class_to_id(totalGridVertices, -1);

                // 1. Extract every outline of the fundamental polygon (Counter-Clockwise orientation)
                for(u = 0; u <= m_segmentsU; ++u) paths[0].push_back(u); // Lower edge (->)
                for(v = 0; v <= m_segmentsV; ++v) paths[1].push_back(v * (m_segmentsU + 1) + m_segmentsU); // Right edge (^)
                for(u = 0; u <= m_segmentsU; ++u) paths[2].push_back(m_segmentsV * (m_segmentsU + 1) + (m_segmentsU - u)); // Upper edge (<-)
                for(v = 0; v <= m_segmentsV; ++v) paths[3].push_back((m_segmentsV - v) * (m_segmentsU + 1)); // Left edge (v)

                // 2. Topological quotient mapping
                for(u = 0; u < 4; u++) 
                    symbols[m_word[u].symbol].push_back(u);

                for(const std::pair<const char, std::vector<unsigned int>>& pair : symbols) 
                {
                    if (pair.second.size() != 2) 
                        continue; // A closed surface glues exactly 2 edges
                    
                    e1 = pair.second[0];
                    e2 = pair.second[1];

                    if (paths[e1].size() == paths[e2].size())
                    {
                        // Topological glue error
                        continue;
                    }
                    
                    /**
                     * Check orientability:
                     * If two words have the same sign, there exist a topological twist, thus making it non-orientable
                     */
                    reverse = (m_word[e1].sign != m_word[e2].sign);
                    if (!reverse)
                        complex.isOrientable = false;

                    len = paths[e1].size(); 
                    
                    for(i = 0; i < len; i++)
                    {
                        v1 = paths[e1][i];
                        v2 = reverse ? paths[e2][len - 1 - i] : paths[e2][i];
                        dsu.unite(v1, v2);
                    }
                }

                // 3. Extract equivalence classes (Generates quotient space)
                complex.quotientMap.resize(totalGridVertices);
                
                uniqueVertices = 0;

                for(i = 0; i < totalGridVertices; i++) 
                {
                    root = dsu.find(i);
                    if(class_to_id[root] == -1)
                        class_to_id[root] = uniqueVertices++; // New topological vertex

                    complex.quotientMap[i] = class_to_id[root];
                }

                complex.uniqueVerticesCount = uniqueVertices;
                complex.gridU = m_segmentsU;
                complex.gridV = m_segmentsV;

                // 4. Triangulation using the quotient map
                complex.triangles.reserve(m_segmentsU * m_segmentsV * 2);

                for(u = 0; u < m_segmentsU; u++) 
                {
                    for(v = 0; v < m_segmentsV; v++) 
                    {
                        // Logical vertices from quad
                        bl = v * (m_segmentsU + 1) + u;
                        br = bl + 1;
                        tl = (v + 1) * (m_segmentsU + 1) + u;
                        tr = tl + 1;
                        
                        // Quotient projection
                        m_bl = complex.quotientMap[bl];
                        m_br = complex.quotientMap[br];
                        m_tl = complex.quotientMap[tl];
                        m_tr = complex.quotientMap[tr];
                        
                        complex.triangles.push_back({m_bl, m_br, m_tl});
                        complex.triangles.push_back({m_br, m_tr, m_tl});
                        complex.edges.push_back({m_bl, m_br});
                        complex.edges.push_back({m_bl, m_tl});
                        complex.edges.push_back({m_br, m_tl});
                        complex.edges.push_back({m_br, m_tr});
                        complex.edges.push_back({m_tl, m_tr});
                    }
                }

                return complex;
            }
            
        private:
            std::vector<WordLetter> m_word;
            unsigned m_segmentsU, m_segmentsV;

            EmbeddingFunction m_embedding;
    };
}

using namespace TopologyPolicies;

/**
 * @brief Geometrical Embeddings for each word surface.
 */
struct GeometricEmbedding
{
    /**
     * @brief Auxiliar function to map p from range [-1, 1] to [0, 2*pi]
     */
    static inline float MapToTwoPi(float p)
    {
        return (p + 1.0f) * bx::kPi; // [-1, 1] -> [0, 2] -> [0, 2*PI]
    }

    /**
     * @brief Auxiliar function to map p from range [-1, 1] to [-pi, pi]
     */
    static inline float MapToPiRange(float p)
    {
        return p * bx::kPi; // [-1, 1] -> [-PI, PI]
    }

    /**
     * @brief Torus geometrical embedding function
     */
    static inline Vec3 Torus(float pu, float pv)
    {
        float u = MapToTwoPi(pu);
        float v = MapToTwoPi(pv);

        const float R = 2.0f;
        const float r = 0.75f;

        float cosU = bx::cos(u);
        float cosV = bx::cos(v);
        
        return { (R + r * cosU) * cosV, 
                 (R + r * cosU) * bx::sin(v), 
                 r * bx::sin(u) };
    }

    /**
     * @brief Klein Bottle geometrical embedding function
     */
    static inline Vec3 KleinBottle(float pu, float pv)
    {
        float x, y, z;
        x = y = z = 0;

        // Inverted!
        float v = MapToTwoPi(pu);
        float u = -MapToTwoPi(pv);

        // Parámetros ajustables
        const float base_thick = 2;
        const float height_factor = 8;
        const float neck_curvature = 3;
        const float scale = 0.75;

        float cosU = bx::cos(u);
        float sinU = bx::sin(u);
        float cosV = bx::cos(v);
        float sinV = bx::sin(v);

        float r = base_thick * (1.0f - cosU / 2.0f);

        if (u < bx::kPi)
        {
            x = neck_curvature * cosU * (1.0f + sinU) + r * cosU * cosV;
            y = height_factor * sinU + r * sinU * cosV;
        }
        else
        {
            x = neck_curvature * cosU * (1.0f + sinU) - r * cosV;
            y = height_factor * sinU;
        }
        z = r * sinV;

        return { x * scale, y * scale, z * scale };
    }

    /**
     * @brief Sphere geometrical embedding function
     */
    static inline Vec3 Sphere(float pu, float pv)
    {
        pu++;
        pv++;
        float theta = pu * 2.0f * bx::kPi; 
        float phi   = pv * bx::kPi;

        // Parameters
        const float R = 2.0f;   // Sphere radius

        float cosPhi = bx::cos(phi);
        float sinPhi = bx::sin(phi);
        float cosTheta = bx::cos(theta);
        float sinTheta = bx::sin(theta);
        
        return {
            R * sinPhi * cosTheta,
            R * sinPhi * sinTheta,
            R * cosPhi
        };
    }

    /**
     * @brief Mobius Strip geometrical embedding function
     */
    static inline Vec3 MobiusStrip(float pu, float pv)
    {
        float u = MapToTwoPi(pu);
        float v = pv;

        /// Parameters
        const float R = 2.0f;    // Central ring radius
        const float w = 1;       // Strip half-width

        float cosU = bx::cos(u);
        float sinU = bx::sin(u);
        float cosU2 = bx::cos(u * 0.5f);
        float sinU2 = bx::sin(u * 0.5f);

        float x = (R + v * cosU2 * w) * cosU;
        float y = (R + v * cosU2 * w) * sinU;
        float z = v * sinU2 * w;

        return { x, y, z };
    }

    /**
     * @brief Mobius Strip geometrical embedding function
     */
    static inline Vec3 RomanSurface(float pu, float pv)
    {
        pu++;
        pv++;
        float u = pu * bx::kPi;
        float v = pv * bx::kPi;
        
        const float R = 2.5f;
        return {
            R * bx::cos(u) * bx::cos(u) * bx::sin(v) * bx::cos(v),
            R * bx::sin(u) * bx::cos(u) * bx::sin(v) * bx::cos(v),
            R * bx::sin(u) * bx::cos(v) * bx::cos(v)
        };
    }

    /** @brief For non-implemented surfaces yet geometrical embedding function */
    static inline Vec3 Fallback(float pu, float pv)
    {
        return { pu, pv, 0.0f }; 
    }
};

static const unsigned surf_size = 10;

    // Torus: a b a^-1 b^-1
static WordSurfacePolicy torus({{'a', 1}, {'b', 1}, {'a', -1}, {'b', -1}}, surf_size, surf_size, GeometricEmbedding::Torus);

    // Klein Bottle: a b a b^-1
    static WordSurfacePolicy klein({{'a', 1}, {'b', 1}, {'a', 1}, {'b', -1}}, surf_size, surf_size, GeometricEmbedding::KleinBottle);

    // Real Projective Plane: a b a b (Non-orientable surface)
    static WordSurfacePolicy proj({{'a', 1}, {'b', 1}, {'a', 1}, {'b', 1}}, surf_size, surf_size, GeometricEmbedding::RomanSurface);

    // Sphere: a b b^-1 a^-1
    static WordSurfacePolicy sphere({{'a', 1}, {'b', 1}, {'b', -1}, {'a', -1}}, surf_size, surf_size, GeometricEmbedding::Sphere);

    // Mobius Band: b c b
    static WordSurfacePolicy mobius({{'a', 1}, {'b', 1}, {'c', 1}, {'b', 1}}, surf_size, surf_size, GeometricEmbedding::MobiusStrip);

#endif