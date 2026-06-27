/******* Topology.h ***************************************************/ /**
 *
 * @file Topology.h
 *
 * Abstract representation of a topological space (X, tau)
 *  
 * @version 2.0
 * @author SpookyProDH-Coder
 * @date 12/06/2026
 ***************************************************************************/

#ifndef _H_TOPOS
#define _H_TOPOS

#include <bx/bx.h>
#include <bx/allocator.h>
#include <type_traits>
#include <new>
#include <functional>

namespace topos
{
template <typename Atom, typename Policy>
class Topology
{
    public:
        /**
         * @brief Finite set X of the topological space
         */
        struct SetX 
        {
            bx::AllocatorI* allocator;  //< Memory-management allocator
            Atom* elements;             //< Element array
            unsigned count;             //< Number of elements in X
        };


        Topology(bx::AllocatorI* _allocator, Policy _policy)
        : m_allocator(_allocator), m_policy(_policy)
        {
            m_X.elements = nullptr;
            m_X.count = 0;
            m_X.allocator = _allocator;
        }

        virtual ~Topology() 
        {
            clean();
        }

        Topology(const Topology&) = delete;
        Topology& operator=(const Topology&) = delete;

        Topology(Topology&& _other) noexcept
        : m_allocator(_other.m_allocator), m_X(_other.m_X), m_policy(std::move(_other.m_policy))
        {
            _other.m_X.elements = nullptr;
            _other.m_X.count = 0;
        }

        Topology& operator=(Topology&& _other) noexcept
        {
            if (this != &_other)
            {
                clean();
                m_allocator = _other.m_allocator;
                m_X = _other.m_X;
                m_policy = std::move(_other.m_policy);
                _other.m_X.elements = nullptr;
                _other.m_X.count = 0;
            }
            return *this;
        }

        void allocateSet(unsigned _numElements)
        {
            clean();
            m_X.count = _numElements;

            if (_numElements > 0)
                m_X.elements = static_cast<Atom*>(bx::alloc(m_allocator, sizeof(Atom) * _numElements));
        }

        /**
         * Function isOpen
         * 
         * @brief Determines whether a subset U is open
         * 
         * @param U
         * @return open
         */
        template<typename SubsetPredicate>
        bool isOpen(SubsetPredicate U) const
        {
            unsigned i = 0;
            bool open = true;
            while (i < m_X.count && open)
            {
                if (U(m_X.elements[i]))
                    if(!m_policy.verifyLocalOpenness(i, m_X, U, *this))
                        open = false;
                i++;
            }
            return open;
        }

        /**
         * Function runNeighborhood
         * 
         * @brief Executes a callback for each element of a neighborhood's point
         * 
         * @param _atomIdx
         * @param _callback Callback
         */
        template<typename NeighborCallback>
        void runNeighborhood(unsigned _atomIdx, NeighborCallback _callback) const
        {
            static_assert(std::is_invocable_v<NeighborCallback, unsigned>, "NeighborCallback must be type unsigned.");
            executeNeighborhoodQuery(_atomIdx, _callback);
        }

        /**
         * Function isInterior
         * 
         * @brief Determines whether a point is in the interior of U
         * 
         * @param _atomIdx Index of the queried element.
         * @param U
         * @return Int
         */
        template<typename SubsetPredicate>
        bool isInterior(unsigned _atomIdx, SubsetPredicate U) const
        {
            bool Int = true;

            runNeighborhood(_atomIdx, [&](unsigned _neighborIdx) {
                if (Int && U(m_X.elements[_neighborIdx]))
                    Int = false;
            });
            return Int;
        }

        /**
         * Function isClosure
         * 
         * @brief Determines whether an element of X belongs to the closure of U
         * 
         * Any point belongs to the closure of U if its neighborhood contains
         * at least one element of U
         * 
         * @param _atomIdx Index of the queried element.
         * @param U
         * @return Close
         */
        template<typename SubsetPredicate>
        bool isClosure(unsigned _atomIdx, SubsetPredicate U) const
        {
            bool Close = false;

            runNeighborhood(_atomIdx, [&](unsigned _neighborIdx) {
                if (U(m_X.elements[_neighborIdx]))
                    Close = true;
            });
            return Close;
        }
        
        /**
         * Function isBoundary
         * 
         * @brief Determines whether a point belongs to the boundary of U.
         *
         * A boundary point is contained in the closure of U but not in
         * its interior.
         *
         * @param _atomIdx Index of the queried element.
         * @param U
         *
         * @retval true If a point is a boundary point
         */
        template<typename SubsetPredicate>
        bool isBoundary(unsigned _atomIdx, SubsetPredicate U) const
        {
            return isClosure(_atomIdx, U) && !isInterior(_atomIdx, U);
        }

        /**
         * Function isHausdorff
         * 
         * @brief Determines whether the topology satisfies the Hausdorff (T2) axiom.
         * 
         * A topological space is T2 if every pair of distinct points
         * admits disjoint neighborhoods.
         * 
         * @return T2
         */
        virtual bool isHausdorff() const
        {
            if (m_X.count < 2)
                return true;
    
            bool T2 = true;
            unsigned i = 0, j = 0;

            while (i < m_X.count && T2)
            {
                j = i+1;
                while (j < m_X.count && T2)
                {
                    runNeighborhood(i, [&](unsigned neighI) {
                        runNeighborhood(j, [&](unsigned neighJ) {
                            if (neighI == neighJ)
                                T2 = false;
                        });
                    });
                    j++;
                }
                i++;
            }
            return T2;
        }
        /**
         * Function generate
         * 
         * @brief Allocates and constructs a set containing the specified number of elements.
         *
         * @param _numElements Number of elements to create.
         */
        void generate(unsigned _numElements) 
        {
            unsigned i;
            clean();
            m_X.count = _numElements;

            if (_numElements > 0)
            {
                m_X.elements = static_cast<Atom*>(bx::alloc(m_allocator, sizeof(Atom) * _numElements));

                if constexpr (!std::is_trivially_default_constructible_v<Atom>)
                    if constexpr (std::is_default_constructible_v<Atom>)
                        for (i = 0; i < _numElements; i++)
                            ::new (static_cast<void*>(&m_X.elements[i])) Atom();
            }
        }

        /** 
         * Function clean
         * 
         * @brief Releases all resources associated with the set X, invoking required destructors
         */
        void clean() 
        {
            unsigned i;
            if (m_X.elements) 
            {
                if constexpr (!std::is_trivially_destructible_v<Atom>)
                    for (i = 0; i < m_X.count; i++)
                        m_X.elements[i].~Atom();

                bx::free(m_allocator, m_X.elements);
                m_X.elements = nullptr;
                m_X.count = 0;
            }
        }

        inline const SetX& getSet() const { return m_X; }
        inline const Policy& getPolicy() const { return m_policy; }


    protected:
        /**
         * Function executeNeighborhoodQuery
         * 
         * @brief Executes a neighborhood query.
         *
         * Must be implemented by derived classes.
         *
         * @param _atomIdx Index of the queried element.
         * @param _internalCallback Callback invoked for each neighbor.
         */
        virtual void executeNeighborhoodQuery(unsigned _atomIdx, const std::function<void(unsigned)>& _internalCallback) const = 0;

        bx::AllocatorI* m_allocator;    //< Memory-management allocator
        SetX m_X;                       //< Set X
        Policy m_policy;                //< Defined topological policy
};
}

#endif