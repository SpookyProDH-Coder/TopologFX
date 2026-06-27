 /******* HashTable.h ***************************************************/ /**
 *
 * @file HashTable.h
 *
 * Hash table Abstract Data Type
 * Redesigned as a Dense Hash Map with Linear Probing "Direccionamiento Abierto"
 *  
 * @version 2
 * @author SpookyProDH-Coder
 * @date 21/06/2026
 ***************************************************************************/
#ifndef _HashTable_H
#define _HashTable_H

#include <string>
#include <cassert>
#include <vector>
#include <iostream>

using namespace std;

/***********class HashTable **************************************//**
 * 
 * @brief Represents Hash table data type with contiguous memory
 * 
 *//*************************************************************/
template<typename KeyType, typename DataType>
class HashTable
{
    public:
        struct PairData
        {
            KeyType clave;
            DataType dato;
        };
        HashTable(unsigned);
        ~HashTable();
        void clear();
        bool search(KeyType, DataType&);
        void insert(KeyType, const DataType&);
        unsigned hash(KeyType) const;
        void print(std::ostream &) const;
        float factorCarga() const;
        unsigned size() const;

        void getAllValues(std::vector<DataType*>&);

        PairData* data();
        const PairData* data() const;

        DataType* getPointer(KeyType);

    private:

        // Yet a funky way of solving weird bugs (The idea: static constexpr unsigned INVALID_INDEX = 0xFFFFFFFF;)
        enum : unsigned { INVALID_INDEX = 0xFFFFFFFF};

        // The hash table
        std::vector<unsigned> t; 

        std::vector<PairData> dense_data;	///< Auxiliar hash table
        
        const unsigned MAGIC = 2654435761u;
        
        // Dynamically grow the table if saturated
        void rehash(unsigned);
};

/**
 *	FUNCTION IMPLEMENTATIONS 
 */

/**
 * @brief Knuth multiplicative hash function
 */
template<typename KeyType, typename DataType>
unsigned HashTable<KeyType, DataType>::hash(KeyType key) const
{
    return (key * MAGIC) % t.size();
}

/** Class constructor **/
template<typename KeyType, typename DataType>
HashTable<KeyType, DataType>::HashTable(unsigned tam)
{
    if (tam == 0) 
        std::cerr << "[!] Hash table size must be greater than zero"<< std::endl;
    else
        t.resize(tam, INVALID_INDEX);

    dense_data.reserve(tam);
}

/** Class destructor **/
template<typename KeyType, typename DataType>
HashTable<KeyType, DataType>::~HashTable()
{
    this->clear();
}

/** 
 * @brief Collition resolution by Linear Probing
 **/
template<typename KeyType, typename DataType>
void HashTable<KeyType, DataType>::insert(KeyType elem, const DataType& valor)
{
    unsigned i = hash(elem);
    unsigned start_i = i;

    if (factorCarga() > 0.7f)
        rehash(t.size() * 2);
    
    while (t[i] != INVALID_INDEX)
    {
        // Si la clave ya existe, la actualizamos en caliente
        if (dense_data[t[i]].clave == elem)
        {
            dense_data[t[i]].dato = valor;
            return;
        }
        // Colisión: saltamos al siguiente bucket contiguo
        i = (i + 1) % t.size();

        // Si hemos dado la vuelta completa, no está
        if (i == start_i) break;
    }
    t[i] = (unsigned)dense_data.size();
    dense_data.emplace_back(PairData{elem, std::move(const_cast<DataType&>(valor))});
}

/** 
 * @brief Search through the HashTable.
 * @param [inout] valor The updated result of the search (does not change if not found).
 * @return Whether the key exists.
 **/
template<typename KeyType, typename DataType>
bool HashTable<KeyType, DataType>::search(KeyType elem, DataType & valor)
{
    if (t.empty()) return false;

    unsigned i = hash(elem);
    unsigned start_i = i;

    while (t[i] != INVALID_INDEX)
    {
        if (dense_data[t[i]].clave == elem)
        {
            valor = dense_data[t[i]].dato;
            return true;
        }
        i = (i + 1) % t.size();
        
        // Si hemos dado la vuelta completa, no está
        if (i == start_i) break;
    }
    return false;
}

/**
 * @brief Reconstruye los índices cuando la tabla crece
 **/
template<typename KeyType, typename DataType>
void HashTable<KeyType, DataType>::rehash(unsigned new_size)
{
    std::vector<unsigned> old_t = t;
    unsigned i, idx;
    t.assign(new_size, INVALID_INDEX);
    
    // Recalculamos los hashes para los datos que ya tenemos en dense_data
    for (idx = 0; idx < dense_data.size(); ++idx)
    {
        i = hash(dense_data[idx].clave);
        
        while (t[i] != INVALID_INDEX)
            i = (i + 1) % t.size();

        t[i] = idx;
    }
}

/** 
 * @brief "Imprime cada cubeta con sus pares (clave,dato)"
 **/
template<typename KeyType, typename DataType>
void HashTable<KeyType, DataType>::print(std::ostream & sal) const
{
    for (unsigned i = 0; i < t.size(); i++) 
    {
        sal << "[" << i << "]: ";
        if (t[i] != INVALID_INDEX)
        {
            const auto& par = dense_data[t[i]];
            sal << par.clave << ", " << par.dato;
        }
        else
            sal << "Empty";

        sal << "\n";
    }
}

/** 
 * @brief "Se calcula como elementos totales / número de t"
 **/
template<typename KeyType, typename DataType>
float HashTable<KeyType, DataType>::factorCarga() const
{
    if (t.empty()) 
        return 0;

    return (float)dense_data.size() / t.size();
}

template<typename KeyType, typename DataType>
unsigned HashTable<KeyType, DataType>::size() const
{
    return dense_data.size();
}

template<typename KeyType, typename DataType>
void HashTable<KeyType, DataType>::getAllValues(std::vector<DataType*>& out_values)
{
    out_values.reserve(out_values.size() + dense_data.size());
    for (auto& par : dense_data)
    {
        out_values.push_back(&par.dato);
    }
}

template<typename KeyType, typename DataType>
typename HashTable<KeyType, DataType>::PairData* HashTable<KeyType, DataType>::data()
{
    return dense_data.data();
}

template<typename KeyType, typename DataType>
const typename HashTable<KeyType, DataType>::PairData* HashTable<KeyType, DataType>::data() const
{
    return dense_data.data();
}

template<typename KeyType, typename DataType>
DataType* HashTable<KeyType, DataType>::getPointer(KeyType elem)
{
    if (t.empty()) 
        return nullptr;

    unsigned i = hash(elem);
    unsigned start_i = i;

    while (t[i] != INVALID_INDEX)
    {
        if (dense_data[t[i]].clave == elem)
                return &dense_data[t[i]].dato; // Devolvemos el puntero al array contiguo

        i = (i + 1) % t.size();
            
        if (i == start_i) 
            break;
    }
    return nullptr;
}

/**
 * @brief Clears the tables dense_data and t
 */
template<typename KeyType, typename DataType>
void HashTable<KeyType, DataType>::clear()
{
    this->dense_data.clear();
    std::fill(this->t.begin(), this->t.end(), INVALID_INDEX);
}
#endif