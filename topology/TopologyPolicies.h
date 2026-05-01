#ifndef _H_TOPOLICIES
#define _H_TOPOLICIES

#include <vector>
#include <algorithm>
#include <bx/math.h>

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

/**
 * Quotient Policies
 */

struct Vec2
{
    float x, y;
};

struct ParametricGrid
{
    unsigned n_size, m_size;
    vector<Vec2> uv_points;
    vector<unsigned> indexes;

    explicit ParametricGrid(unsigned n, unsigned m) : n_size(n), m_size(m) {}
};

struct KleinParameters
{
    float base_thick = 4.0f;
    float height_factor = 16.0f;
    float neck_curvature = 6.0f;
    float camera_scale = 0.15f;
};

class QuotientPolicy
{
    public:
        virtual void applyRule(ParametricGrid&) const = 0;

        unsigned getIndex(unsigned i, unsigned j, unsigned n) const
        {
            return j * n + i;
        }
};

class KleinBottlePolicy : public QuotientPolicy
{
    public:
        KleinParameters kp;
        
        void applyRule(ParametricGrid& pg) const override
        {
            unsigned i, j, k, idx;

            for (k = 0; k < pg.indexes.size(); k++)
            {
                idx = pg.indexes[k];
                i = idx % pg.n_size;
                j = idx / pg.n_size;

                if (j >= pg.m_size - 1)
                    j = 0;

                if (i >= pg.n_size - 1)
                {
                    i = 0;
                    j = pg.m_size - 1 - j;
                }

                pg.indexes[k] = getIndex(i, j, pg.n_size - 1);
            }
        }
};

struct MorbiusParameters
{
    float camera_scale = 1.0f;
};

class MorbiusPolicy : public QuotientPolicy
{
    public:
        MorbiusParameters kp;

        void applyRule(ParametricGrid& pg) const override
        {
            unsigned i, j, k, idx;

            for (k = 0; k < pg.indexes.size(); k++)
            {
                idx = pg.indexes[k];
                i = idx % pg.n_size;
                j = idx / pg.n_size;

                if (j >= pg.m_size - 1)
                    j = 0;

                pg.indexes[k] = getIndex(i, j, pg.n_size - 1);
            }
        }
        
};

enum class Structure
{
    KLEIN,
    MORBIUS
};

template <typename Vec3>
class Embedding
{
    public:
        Embedding(Structure _struct) : _structure(_struct) {}

        Structure getType() const
        {
            return this->_structure;
        }

        Vec3 map(const Vec2& uv, const KleinParameters& kp, const MorbiusParameters& mp) const
        {
            switch (_structure)
            {
                case Structure::KLEIN:
                    return klein_map(uv, kp);
                case Structure::MORBIUS:
                    return morbius_map(uv, mp);
                default:
                    return {0,0,0};
            }
        }
        
    private:
        Structure _structure;

        Vec3 klein_map(const Vec2& uv, const KleinParameters& kp) const
        {
            float u, v, r, sinU, sinV, cosU, cosV, x, y, z;
            u = uv.x * bx::kPi2;
            v = uv.y * bx::kPi2;

            sinU = bx::sin(u);
            cosU = bx::cos(u);
            sinV = bx::sin(v);
            cosV = bx::cos(v);
            
            r = kp.base_thick * (1.0f - cosU / 2.0f);

            if (u < bx::kPi)
            {
                x = kp.neck_curvature * cosU * (1.0f + sinU) + r * cosU * cosV;
                y = kp.height_factor * sinU + r * sinU * cosV;
            }
            else
            {
                x = kp.neck_curvature * cosU * (1.0f + sinU) - r * cosV;
                y = kp.height_factor * sinU;
            }
            z = r * sinV;
            
            return {x * kp.camera_scale, y * kp.camera_scale, z *  kp.camera_scale};
        }

        Vec3 morbius_map(const Vec2& uv, const MorbiusParameters& kp) const
        {
            float u, v, r, sinU, cosU, sinU2, cosU2, x, y, z;
            u = uv.x * bx::kPi2;
            v = uv.y * 2.0f - 1;

            sinU2 = bx::sin(u/2.0f);
            cosU2 = bx::cos(u/2.0f);
            sinU = bx::sin(u);
            cosU = bx::cos(u);

            r = (1 + v/2.0f * cosU2);
            x = r * cosU;
            y = r * sinU;
            z = v/2.0f * sinU2;
            
            return {x * kp.camera_scale, y * kp.camera_scale, z *  kp.camera_scale};
        }
};

#endif