// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos-plantilla.cpp
// Implementación del problema de los filósofos (sin camarero).
// Plantilla para completar.
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------


#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
num_filosofos = 5 ,
num_procesos  = 2*num_filosofos ;

const int accion_soltar = 0, accion_coger = 1, accion_sentarse = 2, accion_levantarse = 3;
const int id_camarero = 10;
//int MPI_Send(void *buf_emi, int num, MPI_Datatype type, int desst, int tag, MPI_Comm comm)
//int MPI_Recv(void *buf_emi, int num, MPI_Datatype type, int source, int tag, MPI_Comm comm, MPI_Status *status)
//int MPI_Get_count(MPI_Status * status, MPI_Datatype dtypem, int * num)
// MPI_ANY_SOURCE  MPI_ANY_TAG

//int MPI_Ssend(void * buf_emi, int count, MPI_Datatype dtype, int dest, int tag, MPI_Comm comm)
// es seguro, buf_emi puede ser modificado. Es sincrono

//MPI_Probe(int source, int tag, MPI_Comm comm, MPI_Status *status); el proceso se qeda bloqueado hasta que haya un mensaje en ese proceso
//MPI_IProbe(int source, int tag, MPI_Comm comm, int *flag, MPI_Status *status) no bloqua. flag será mayor que 0 si hay algun mensaje en ese proceso

//MPI_Isend(void *buf_emi, int num, MPI_Datatype type, int desst, int tag, MPI_Comm comm, MPI_Request *request) insegura. Inicia el envio pero retorna antes de leer el buff
//MPI_Irecv(void *buf_emi, int num, MPI_Datatype type, int source, int tag, MPI_Comm comm, MPI_Request *request) insegura. Inicia recepcion pero retorna anttes de recibir
//MPI_Wait(MPI_Request *request, int * fllag, MPI_Status *status) espera bloqueado hasta que acabe el envio o recepcion
//MPI_Test(MPI_Request *request, MPI_Status * status) comprueba si el envio o recepcion ha finalizado. No es bloqueante

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio() {
    static default_random_engine generador( (random_device())() );
    static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
    return distribucion_uniforme( generador );
}

// ---------------------------------------------------------------------

void funcion_filosofos( int id ) {
    int id_ten_izq = (id+1)              % num_procesos, //id. tenedor izq.
    id_ten_der = (id+num_procesos-1) % num_procesos; //id. tenedor der.

    MPI_Status estado;
    while ( true ) {
        //Solicita sentarse
        cout << "Filósofo " << id << " solicita sentarse" <<endl;
        MPI_Send(NULL, 0, MPI_INT, id_camarero, accion_sentarse, MPI_COMM_WORLD);

        //Espera a tener permiso
        MPI_Recv(NULL, 0, MPI_INT, id_camarero, accion_sentarse, MPI_COMM_WORLD, &estado);
        cout << "Filósofo " << id << " se sienta" <<endl;

        cout << "Filósofo " << id << " solicita ten. izq." <<id_ten_izq <<endl;
        MPI_Ssend(NULL, 0, MPI_INT, id_ten_izq, accion_coger, MPI_COMM_WORLD);

        cout << "Filósofo " << id <<" solicita ten. der." <<id_ten_der <<endl;
        MPI_Ssend(NULL, 0, MPI_INT, id_ten_der, accion_coger, MPI_COMM_WORLD);


        cout << "Filósofo " << id <<" comienza a comer" <<endl ;
        sleep_for( milliseconds( aleatorio<10,100>() ) );

        cout << "Filósofo " << id <<" suelta ten. izq. " <<id_ten_izq <<endl;
        MPI_Ssend(NULL, 0, MPI_INT, id_ten_izq, accion_soltar, MPI_COMM_WORLD);

        cout<< "Filósofo " << id <<" suelta ten. der. " <<id_ten_der <<endl;
        MPI_Ssend(NULL, 0, MPI_INT, id_ten_der, accion_soltar, MPI_COMM_WORLD);

        //Se levanta de la mesa y avisa al camarero
        cout << "Filósofo " << id << " se levanta" <<endl;
        MPI_Ssend(NULL, 0, MPI_INT, id_camarero, accion_levantarse, MPI_COMM_WORLD);

        cout << "Filosofo " << id << " comienza a pensar" << endl;
        sleep_for( milliseconds( aleatorio<10,100>() ) );
    }
}
// ---------------------------------------------------------------------

void funcion_tenedores( int id ) {
    int valor, id_filosofo ;  // valor recibido, identificador del filósofo
    MPI_Status estado ;       // metadatos de las dos recepciones

    while ( true ) {
        MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, accion_coger, MPI_COMM_WORLD, &estado);

        id_filosofo = estado.MPI_SOURCE;
        cout << "Ten. " <<id <<" ha sido cogido por filo. " << id_filosofo << endl;

        MPI_Recv(&id_filosofo, 1, MPI_INT, id_filosofo, accion_soltar, MPI_COMM_WORLD, &estado);
        cout << "Ten. "<< id << " ha sido liberado por filo. " << id_filosofo << endl ;
    }
}
// ---------------------------------------------------------------------

void funcion_camarero( int id ) {
    int valor;
    int filosofos_sentados = 0;
    MPI_Status estado;

    while (true) {
        //Si hay 4 filosofos comiendo solo podrá levantarse uno. Si hay menos, también sentarse
        if (filosofos_sentados < 4) {
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);
        } else {
            MPI_Probe(MPI_ANY_SOURCE, accion_levantarse, MPI_COMM_WORLD, &estado);
        }

        valor = estado.MPI_SOURCE;
        if (estado.MPI_TAG == accion_sentarse) {
            MPI_Recv(NULL, 0, MPI_INT, valor, accion_sentarse, MPI_COMM_WORLD, &estado);
            filosofos_sentados++;

            MPI_Send(NULL, 0, MPI_INT, valor, accion_sentarse, MPI_COMM_WORLD);
            cout << "El filosofo " << valor << " se sienta (" << filosofos_sentados << ")" << endl;
        }
        if (estado.MPI_TAG == accion_levantarse) {
            MPI_Recv(NULL, 0, MPI_INT, valor, accion_levantarse, MPI_COMM_WORLD, &estado);
            filosofos_sentados--;

            MPI_Send(NULL, 0, MPI_INT, valor, accion_levantarse, MPI_COMM_WORLD);
            cout << "El filosofo " << valor << " se levanta (" << filosofos_sentados << ")" << endl;
        }
    }
}

// ---------------------------------------------------------------------
int main( int argc, char** argv ) {
    int id_propio, num_procesos_actual ;

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
    MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


    if (num_procesos_actual == 11) {
        // ejecutar la función correspondiente a 'id_propio'
        if (id_propio == 10) {
            funcion_camarero(id_propio);
        } else if (id_propio % 2 == 0) {
            funcion_filosofos( id_propio ); //   es un filósofo
        } else {                               // si es impar
            funcion_tenedores( id_propio ); //   es un tenedor
        }
    } else {
        if ( id_propio == 0 ) { // solo el primero escribe error, indep. del rol
            cout << "el número de procesos esperados es:    " << num_procesos + 1 << endl
            << "el número de procesos en ejecución es: " << num_procesos_actual << endl
            << "(programa abortado)" << endl ;
        }
    }

    MPI_Finalize( );
    return 0;
}

// ---------------------------------------------------------------------
