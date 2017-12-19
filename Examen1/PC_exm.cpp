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

const int num_items = 40 ,   // número de items
	  tam_vec   = 10 ;   // tamaño del buffer

//Balance y su semaforo para evitar errores de concurrencia
int balance = 0;
Semaphore puedeCambiarBalance = 1; //inicialmente se puede

//Defino los buffer
int bufferA[num_items];
int * punteroEscrituraA = bufferA;
int * punteroLecturaA = bufferA;

Semaphore puedeProducirA = tam_vec; //inicialmente se puede producir hasta llenar el vector
Semaphore puedeConsumirA = 0; //no se puede consumir inicialmente

int bufferB[num_items];
int * punteroEscrituraB = bufferB;
int * punteroLecturaB = bufferB;

Semaphore puedeProducirB = tam_vec;
Semaphore puedeConsumirB = 0;

//Func para consumir ambos buffer según sea el A o el B, se pasa por referencia sus semáforos y punteros
void consumirBuffer(Semaphore & semConsumir, Semaphore & semProducir, int * buffer, int * puntLectura, char c);

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
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato()
{
   int numero = 0 ;
   numero = aleatorio<5,15>();
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "producido: " << numero << endl << flush ;

   return numero;
}
//----------------------------------------------------------------------
void consumir_dato(unsigned dato, char c)
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                  consumido en " << c << ": " << dato << endl ;

   puedeCambiarBalance.sem_wait();
   if ((dato % 2) == 0) {
	balance--;
   } else {
   	balance++;
   }
   puedeCambiarBalance.sem_signal();
}

//----------------------------------------------------------------------

void funcion_hebra_productora() {
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato = producir_dato();

      // alternar entre buffer A y B
      if ((i % 2) == 0) {
          puedeProducirA.sem_wait();

	  *punteroEscrituraA = dato;
	  //punteroEscrituraA = (punteroEscrituraA + 1) % tam_vec;
	   if ((punteroEscrituraA + 1) > (bufferA+tam_vec)) {
	      punteroEscrituraA = bufferA;
	   } else {
	      punteroEscrituraA++;
           }
           puedeConsumirA.sem_signal();
      } else {
          puedeProducirB.sem_wait();

	  *punteroEscrituraB = dato;
	  //punteroEscrituraB = (punteroEscrituraB + 1) % tam_vec;
	   if ((punteroEscrituraB + 1) > (bufferB+tam_vec)) {
	      punteroEscrituraB = bufferB;
	   } else {
	      punteroEscrituraB++;
           }
           puedeConsumirB.sem_signal();
      }
   }
}

//----------------------------------------------------------------------
void funcion_hebra_consumidora(bool userBufferA) {
    if (userBufferA) {
        consumirBuffer(puedeConsumirA, puedeProducirA, bufferA, punteroLecturaA, 'A');
    } else {
        consumirBuffer(puedeConsumirB, puedeProducirB, bufferB, punteroLecturaB, 'B');
    }
    //Al no poder usar el operador = en semaphore, para no repetir código reuno todo en una misma función y paso el sem de cada buffer por referencia
}

void consumirBuffer(Semaphore & semConsumir, Semaphore & semProducir, int * buffer, int * puntLectura, char c)  {
	for( unsigned i = 0 ; i < (num_items / 2) ; i++ ) {
		int dato ;
		semConsumir.sem_wait();

		dato = *puntLectura;
		//puntLectura = (puntLectura + 1) % tam_vec;
		if ((puntLectura + 1) > (buffer+tam_vec)) {
			puntLectura = buffer;
		} else {
			puntLectura++;
		}

		semProducir.sem_signal();
        	consumir_dato(dato, c) ;
	}
}
//----------------------------------------------------------------------

int main()
{
   cout << "--------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución LIFO)." << endl
        << "--------------------------------------------------------" << endl
        << flush ;

   thread hebra_productora (funcion_hebra_productora),
          hebra_consumidoraA(funcion_hebra_consumidora, true),
          hebra_consumidoraB(funcion_hebra_consumidora, false);

   hebra_productora.join() ;
   hebra_consumidoraA.join() ;
   hebra_consumidoraB.join() ;

   cout << "El balance de numeros pares e impares consumido es " << balance << endl << flush;
   cout << "fin" << endl;

   return 1; 
}
