/******* main.cpp ***************************************************/ /**
 *
 * @file Evaluate.cpp
 *
 * @version 1.0
 * @author SpookyProDH-Coder
 * @date 23/06/2026
 ***************************************************************************/

#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <cstring>

#include "HashTableOld.h"
#include "../../engine/core/HashTable.h"

using namespace std;

/* Simulated Entity from graphic engine */
struct Entity_Fake
{
    unsigned mesh_id;
    unsigned material_id;

    float position[3] = { 0.0f, 0.0f, 0.0f };
    float scale[3]    = { 1.0f, 1.0f, 1.0f };
    float rotation[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    float transform[16] = {0.0f};
    
    /// Raw code from bx::mtxIdentity()
    Entity_Fake() : mesh_id(0), material_id(0)
    {
        memset(transform, 0, sizeof(transform));
        transform[0] = transform[5] = transform[10] = transform[15] = 1.0f;
    }
};

const unsigned MAX_ELEM = 50000; 
const unsigned INI_PRUEBA = 1000;
const unsigned FIN_PRUEBA = MAX_ELEM;
const unsigned INC_PRUEBA = 1000;
const unsigned REPETICIONES = 5000;

/** Evil global variables */
volatile float acumulador_global = 0.0f;
HashTable<unsigned, Entity_Fake> HASH_NEW(MAX_ELEM*2);
HashTableOld<unsigned, Entity_Fake> HASH_OLD(MAX_ELEM*2);

void evaluarTiempo(unsigned ini, unsigned fin, unsigned inc, unsigned (*t)(unsigned), ostream& f);
void calcularCoste(unsigned ini, unsigned fin, unsigned inc, unsigned (*t)(unsigned), ostream& f);
unsigned fHashNew(unsigned n);
unsigned fHashOld(unsigned n);

int main(void)
{
    ofstream f("New_Steps.data");
    ofstream g("Old_Steps.data");
    ofstream h("New_Time.data");
    ofstream i("Old_Time.data");

    if (!f || !g || !h || !i)
    {
        cerr << "[!] Error: Couldn't create files." << endl;
        return -1;
    }

    cout << "[*] Inserting test elements in each hash.";
    Entity_Fake dummy;

    /* Filling up the tables to the maximum */
    for (unsigned i = 0; i < MAX_ELEM; i++) 
    {
        HASH_NEW.insert(i * 2, dummy);
        HASH_OLD.insert(i * 2, dummy);
    }

    cout << "[*] Calculating Sparse Dense Hash Table...";

    calcularCoste(INI_PRUEBA, FIN_PRUEBA, INC_PRUEBA, fHashNew, f);
    f.close();
    evaluarTiempo(INI_PRUEBA, FIN_PRUEBA, INC_PRUEBA, fHashNew, h);
    h.close();

    cout << "[*] Calculating Chained Hash Table mean access steps...";

    calcularCoste(INI_PRUEBA, FIN_PRUEBA, INC_PRUEBA, fHashOld, g);
    g.close();
    evaluarTiempo(INI_PRUEBA, FIN_PRUEBA, INC_PRUEBA, fHashOld, i);
    i.close();

    cout << "[*] Successfuly computed." << endl;

    return 0;
}

void calcularCoste(unsigned ini, unsigned fin, unsigned inc, unsigned (*t)(unsigned), ostream& f)
{
    vector<double> v;
    size_t k;
    unsigned n, diferencia, n_puntos, val_compro, i, pasos;
    
    /**
     * Detectamos errores en el intervalo de entrada
     */
    diferencia=fin-ini;
    if (ini >= fin)
        cerr << "[!] Error: El valor inicial ini es MAS GRANDE O IGUAL  que fin!"<< endl;
    else if (diferencia > fin)
        cerr << "[!] Error: NO hay espacio para crear los puntos discretos de I!"<< endl;
    else if (inc == 0)
        cerr << "[!] El incremento NO PUEDE SER negativo o 0" << endl;
    else
    {
        /**
         * Generamos un intervalo de puntos [ini,fin] y analizamos el tiempo medio que cuesta
         * realizar la funcion f segun el tamano del vector v.
         */
        //Creamos el número de puntos de I: funcion_suelo(diff / inc) + 1
        n_puntos = (diferencia / inc) + 1;
        //OJO, y si el redondeo para el calculo nos sale nulo o negativo?
        if (n_puntos == 0)
            cerr << "[!] Error interno: número de puntos inválido!" << endl;
        else
        {
            //Ya creamos el vector con las tallas (valores n)...
            v.resize(n_puntos);
            //Rellenamos v con: ini, ini+inc, ini+2*inc, ... (todos <= fin)
            val_compro=ini;
            i=0;
            while(i<n_puntos && val_compro <= fin)
            {
                v[i] = ini + i * inc;
                i++;
                val_compro = ini+ i * inc;
            }
            // Volcamos al fichero los pares "(n,t(n))" uno por línea.
            for (k = 0; k < v.size();k++)
            {
                n = v[k];
                pasos=t(n);    //Llamamos al puntero a funcion
                // Volcamos los datos en el fichero
                f << n << " " << pasos << endl;
            }
        }
    }
}

/***** Funcion evaluarTiempo **************************************//**
 * 
 * @brief Permite volcar en un fichero de salida (f) los tamaños del vector y su correspondiente 
 * coste temporal [Seria el par (n,t(n))] de una función cualquiera (t) para un rango de n entre 
 * ini y fin con un incremento determinado (inc). El parámetro t será un PUNTERO a función,
 * lo que nos permitirá pasarle cualquier función que cumpla la especificación 
 * (Sea I:=[ini,fin]:={x0,x1,x2,x3,x4,...}; donde x[i] - x[i-1] = inc)
 * 
 * @pre {n > 0 ^ ini <= fin}
 * 
 * @param [in] n Tamaño del vector a evaluar.
 * @param [in] ini Inicio del rango.
 * @param [in] fin Fin del rango.
 * @param [in] t  Es el puntero a la funcion a evaluar
 * @param [inout] f Este sera el flujo de salida/escritura de salida
 **************************************************************/ 
void evaluarTiempo(unsigned ini, unsigned fin, unsigned inc, unsigned (*t)(unsigned), ostream& f)
{
    vector<double> v;
    size_t k;
    unsigned n, diferencia,n_puntos,val_compro,i;
    struct timespec ini_time, fin_time;
    double time;
    
    /**
     * Detectamos errores en el intervalo de entrada
     */
    diferencia=fin-ini;
    if (ini >= fin)
        cerr << "[!] Error: El valor inicial ini es MAS GRANDE O IGUAL  que fin!"<< endl;
    else if (diferencia > fin)
        cerr << "[!] Error: NO hay espacio para crear los puntos discretos de I!"<< endl;
    else if (inc == 0)
        cerr << "[!] El incremento NO PUEDE SER negativo o 0" << endl;
    else
    {
        /**
         * Generamos un intervalo de puntos [ini,fin] y analizamos el tiempo medio que cuesta
         * realizar la funcion f segun el tamano del vector v.
         */
        //Creamos el número de puntos de I: funcion_suelo(diff / inc) + 1
        n_puntos = (diferencia / inc) + 1;
        //OJO, y si el redondeo para el calculo nos sale nulo o negativo?
        if (n_puntos == 0)
            cerr << "[!] Error interno: número de puntos inválido!" << endl;
        else
        {
            //Ya creamos el vector con las tallas (valores n)...
            v.resize(n_puntos);
            //Rellenamos v con: ini, ini+inc, ini+2*inc, ... (todos <= fin)
            val_compro=ini;
            i=0;
            while(i<n_puntos && val_compro <= fin)
            {
                v[i] = ini + i * inc;
                i++;
                val_compro= ini+ i * inc;
            }
            // Volcamos al fichero los pares "(n,t(n))" uno por línea.
            for (k = 0; k < v.size();k++)
            {
                // Aqui llamamos a clock_gettime para contar cuanto tiempo
                // tarda la funcion en esta instancia...
                
                n = v[k];

                clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ini_time);
                for (unsigned rep = 0; rep < REPETICIONES; rep++)
                    (void)t(n);    //Llamamos al puntero a función, ignorando los parametros de salida con ignore
                clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &fin_time);

                time = (fin_time.tv_sec - ini_time.tv_sec) + (fin_time.tv_nsec * 1.0e-9 - ini_time.tv_nsec * 1.0e-9);
                double time_sec = (time / REPETICIONES);
                // Volcamos los datos en el fichero
                f << n << " " << time_sec << endl;
            }
        }
    }
}

unsigned fHashNew(unsigned n)
{
    unsigned i, iterados = 0;
    unsigned max_elems = HASH_NEW.size();
    auto* array_data = HASH_NEW.data();
    
    unsigned limit = (n < max_elems) ? n : max_elems;

    float suma_local = 0.0f;
    for (i = 0; i < limit; i++)
    {
        // Leemos la memoria contigua
        suma_local += array_data[i].dato.position[0]; 
        iterados++;
    }
    
    acumulador_global = suma_local; 
    return iterados;
}

unsigned fHashOld(unsigned n)
{
    unsigned limit, iterados = 0;
    size_t i;
    float suma_local;

    const vector<list<HashTableOld<unsigned int, Entity_Fake>::PairData>>* t_ptr = HASH_OLD.AUX_GET_TABLE();
    
    list<HashTableOld<unsigned, Entity_Fake>::PairData>::const_iterator it;
    
    if (t_ptr == nullptr)
        return 0;

    limit = (n < MAX_ELEM) ? n : MAX_ELEM;
    suma_local = 0.0f;

    for (i = 0; i < t_ptr->size() && iterados < limit; i++) 
    {
        for (it = (*t_ptr)[i].begin(); it != (*t_ptr)[i].end() && iterados < limit; ++it) 
        {
            // Saltamos por los nodos fragmentados!
            suma_local += it->dato.position[0];
            iterados++;
        }
    }
    
    acumulador_global = suma_local;
    return iterados;
}