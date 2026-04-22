 /******* HashTable.h ***************************************************/ /**
 *
 * @file HashTable.h
 *
 * Hash table Abstract Data Type
 *  
 * @version 0
 * @author SpookyProDH-Coder
 * @date 15/01/2025
 ***************************************************************************/
#ifndef _HashTable_H
#define _HashTable_H

#include <string>
#include <assert.h>
#include <vector>
#include <list>
#include <iostream>

using namespace std;

/***********class HashTable **************************************//**
 * 
 * @brief Represents Hash data type
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
		bool search(KeyType, DataType&); 
		void insert(KeyType, const DataType&); 
		unsigned hash(KeyType) const;
		void print(ostream & sal) const;
		float factorCarga() const;
		/**
		 * Número de posiciones de la HashTable hash que tienen n elementos.
		 * @param n número de elementos en una posición
		 */
		unsigned num_posN(int n) const;
	private:
		typedef list<PairData> DataList;
		vector<DataList> t;
		const unsigned MAGIC = 2654435761; //Appr. 2^32 / phi
		const unsigned HASHBITS = 6;
};

/**
 *	FUNCTION IMPLEMENTATIONS 
 */

/**
 * Hash function:
 * Knuth multiplicative hash function
 */
template<typename KeyType, typename DataType>
unsigned HashTable<KeyType, DataType>::hash(KeyType key) const
{
	return (key * MAGIC) >> (32 - HASHBITS);
}

/** Class constructor **/
template<typename KeyType, typename DataType>
HashTable<KeyType, DataType>::HashTable(unsigned tam)
{
	if (tam <= 0) 
		cerr << "[!] Hash table size must be greater than zero"<< endl;
	else
		t.resize(tam);
}

/** 
 * FUNCTION insert
 * @brief "Si la clave existe actualiza el dato; si no, inserta al final" 
 **/
template<typename KeyType, typename DataType>
void HashTable<KeyType, DataType>::insert(KeyType elem, const DataType& valor)
{
	unsigned i = hash(elem);
	auto it = t[i].begin();
	bool compro = true;
	
	while((it != t[i].end()) && compro)
	{
		if(it->clave == elem)
		{
			it->dato = valor;
			compro=false;
		}
		++it;
	}
	if(compro)
		t[i].push_back(PairData{elem, valor});
	
	return;
}

/** 
 * FUNCION search:
 * @brief "Devuelve true y copia el dato en 'valor' si existe la clave" 
 **/
template<typename KeyType, typename DataType>
bool HashTable<KeyType, DataType>::search(KeyType elem, DataType & valor)
{
	bool compro = false;
	unsigned i = hash(elem);
	auto it = t[i].begin();
	
	while((it != t[i].end()) && !compro)
	{
		if(it->clave == elem)
		{
			//it->dato = valor;
			valor = it->dato;
			compro=true;
		}
		++it;
	}
	return compro;
}

/** 
 * FUNCION print
 * @brief "Imprime cada cubeta con sus pares (clave,dato)"
 **/
template<typename KeyType, typename DataType>
void HashTable<KeyType, DataType>::print(ostream & sal) const
{
	unsigned i =0;
	for (i = 0; i < t.size(); ++i) 
	{
		sal << "[" << i << "]:" << endl;
		for (auto & par : t[i])
		{
			sal << par.clave << ", " << par.dato << endl;
		}
		sal << "\n";
	}
}

/** 
 * FUNCION factorCarga
 * "Se calcula como elementos totales / número de buckets
 **/
template<typename KeyType, typename DataType>
float HashTable<KeyType, DataType>::factorCarga() const
{
	float factor = 0.0;
	double total = 0;
	
	if (!t.empty())
	{
		for(auto & lst : t) 
			total += double(lst.size());
		factor = float(total) / float(t.size());
	}

	return factor;
}

/**
 * FUNCION num_posN
 * @brief "Cuenta cuantas posiciones tienen exactamente n elementos"
 **/
template<typename KeyType, typename DataType>
unsigned HashTable<KeyType, DataType>::num_posN(int n) const
{
	assert(n >= 0);

	unsigned valor = 0;

	if (n >= 0)
	{
		for (auto & lst : t)
			if (int(lst.size()) == n) 
				++valor;
	}

	return valor;
}
#endif
