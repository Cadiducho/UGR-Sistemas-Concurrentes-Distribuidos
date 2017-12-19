#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include "HoareMonitor.hpp"

using namespace std ;
using namespace HM ;

// numero de fumadores
const int num_clientes = 10 ;

// mutex de escritura en pantalla
mutex mtx ;

//**********************************************************************

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//**********************************************************************

void Imprimir(int ih)
{
   mtx.lock();
   cout << "El usuario " << ih << " está imprimiendo" << flush << endl ;
   mtx.unlock();

   this_thread::sleep_for( chrono::milliseconds( aleatorio<300,600>() ));

   mtx.lock();
   cout << "El usuario " << ih << " ha finalizado de imprimir" << flush << endl ;
   mtx.unlock();
}
//----------------------------------------------------------------------

void ImprimirSuperU()
{
   mtx.lock();
   cout << "\tEl superusuario está imprimiendo" << flush <<  endl ;
   mtx.unlock();

   this_thread::sleep_for( chrono::milliseconds( aleatorio<300,600>() ));

   mtx.lock();
   cout << "\tEl superusuario ha finalizado de imprimir" << flush <<  endl ;
   mtx.unlock();
}

// *****************************************************************************
// clase para monitor

class Impresora : public HoareMonitor
{
  private: 
    //Delcarar variable/s condición
    //Delcarar variable/s estado
     CondVar cola;
     bool ocupada;
     CondVar supersuEsperando;

 public:          // constructor y métodos públicos
   Impresora() ;    // constructor
   void obtenerImpresora( int ih );
   void liberarImpresora( int ih );
   void obtenerImpresoraSuperU( );
   void liberarImpresoraSuperU( );
} ;
// -----------------------------------------------------------------------------

Impresora::Impresora(  )
{
  //Inicializar variable/s condición
  //Inicializar variable/s estado
  cola = newCondVar();
  ocupada = false;
  supersuEsperando = newCondVar();
}
// -----------------------------------------------------------------------------

void Impresora::obtenerImpresora( int ih )
{
  cout << "El usuario " << ih << " quiere imprimir" << flush << endl;
  //Si la impresora está ocupada
  //  el usuario se bloquea en la cola de usuarios
  if (ocupada) {
    cout << "La impresora está ocupada" << endl << flush; 
    cola.wait();
  }
  //En este punto el usuario ih puede obtener la impresora
  //modificar la/s variable/s de estado del monitor para indicar que la impresora está ocupada
  cout << "El usuario " << ih << " obtiene la impresora" << flush << endl;
  ocupada = true;
}
// -----------------------------------------------------------------------------

void Impresora::liberarImpresora( int ih )
{
  //modificar la/s variable/s de estado del monitor para indicar que la impresora está ahora libre
  ocupada = false;

  //Si el superusuario está esperando
  //  liberar al superusuario para que pueda acceder a la impresora
  //Si no
  //  liberar al siguiente usuario para que pueda acceder a la impresora
  if (!supersuEsperando.empty()) {
    supersuEsperando.signal();
  } else {
    cola.signal();
  }
}

// -----------------------------------------------------------------------------

void Impresora::obtenerImpresoraSuperU( )
{
  cout << "\tEl superusuario quiere imprimir" << endl ;
  //Si la impresora está ocupada
  //  el superusuario se bloquea en una cola de superusuarios
  if (ocupada) {
    supersuEsperando.wait();
  }
  //En este punto el superusuario puede obtener la impresora
  //modificar la/s variable/s de estado del monitor para indicar que la impresora está ocupada
  ocupada = true;
}
// -----------------------------------------------------------------------------

void Impresora::liberarImpresoraSuperU( )
{
  //modificar la/s variable/s de estado del monitor para indicar que la impresora está ahora libre
  cout << "La impresora deja de estar ocupada por el superusuario" << endl << flush;
  ocupada = false;
  //liberar al siguiente usuario para que pueda acceder a la impresora
  cola.signal();
}

// *****************************************************************************
// funciones de hebras
// -----------------------------------------------------------------------------

void funcion_hebra_usuario( MRef<Impresora> monitor, int num_cliente )
{
   while( true )
   {
      this_thread::sleep_for( chrono::milliseconds( aleatorio<100,600>() ));
      monitor->obtenerImpresora( num_cliente );
      Imprimir( num_cliente );
      monitor->liberarImpresora( num_cliente );
   }
}

// -----------------------------------------------------------------------------

void funcion_hebra_superusuario( MRef<Impresora> monitor)
{
   while( true )
   {
      this_thread::sleep_for( chrono::milliseconds( aleatorio<1000,1200>() ));
      monitor->obtenerImpresoraSuperU( );
      ImprimirSuperU( );
      monitor->liberarImpresoraSuperU( );
   }
}

// *****************************************************************************

int main()
{
  MRef<Impresora> impresora = Create<Impresora>();
  thread hebra_usuarios[num_clientes];
  thread hebra_superusuario(funcion_hebra_superusuario, impresora);

  for (int i = 0; i < num_clientes; i++) {
    hebra_usuarios[i] = thread(funcion_hebra_usuario, impresora, i);
  }
  for (int i = 0; i < num_clientes; i++) {
    hebra_usuarios[i].join();
  }
  hebra_superusuario.join();

}

