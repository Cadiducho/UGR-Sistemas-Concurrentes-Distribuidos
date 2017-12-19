#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

//**********************************************************************
// variables compartidas

const int num_items = 40 ,   // número de items a producir
	       tam_vec   = 10 ;   // tamaño de los buffers

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//**********************************************************************
// funciones comunes
//----------------------------------------------------------------------

int producir_dato()
{
   int numero = 0 ;
   //**********************************************************************
   //AQUI generar numero aleatorio en variable numero
   //**********************************************************************
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "producido: " << numero << endl << flush ;

   return numero;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                  consumido: " << dato << endl ;   
}

//**********************************************************************

void  funcion_hebra_productora(  )
{
    for( unsigned i = 0 ; i < num_items ; i++ )
    {
      int dato = producir_dato() ;
      //....
    }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(  )
{
    //Bucle for desde 0 hasta N/2, donde N es el numero total de elementos que se van a producir
    {
      int dato ;
      //...
      consumir_dato( dato ) ;
    }
}

//**********************************************************************

int main()
{
  //Inicializar y lanzar las hebras
  //....
  //....

  return 0;
}
