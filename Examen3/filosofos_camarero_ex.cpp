#include <mpi.h>
#include <thread>
#include <random>
#include <chrono>
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
num_filosofos = 5 ,
num_procesos  = 2*num_filosofos ;

const int accion_soltar = 0, accion_coger = 1, accion_sentarse = 2, accion_levantarse = 3;
const int accion_coger_cucharilla = 4, accion_soltar_cucharilla = 5, accion_cucharilla_otorgada = 6, accion_cucharilla_devuelta = 7;
const int id_camarero = 10;
const int id_camarero_postre = 11;



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

        //EXAMEN: Se come un postre
        cout << "Filósofo " << id << " solicita una cucharilla." <<endl;
        MPI_Ssend(NULL, 0, MPI_INT, id_camarero_postre, accion_coger_cucharilla, MPI_COMM_WORLD);
        //espera a tener la cucharilla
        MPI_Recv(NULL, 0, MPI_INT, id_camarero_postre, accion_cucharilla_otorgada, MPI_COMM_WORLD, &estado);

        cout << "Filósofo " << id <<" comienza a comerse un postre. " << endl ;
        sleep_for( milliseconds( aleatorio<10,100>() ) );

        cout<< "Filósofo " << id <<" suelta la cucharilla " <<endl;
        MPI_Ssend(NULL, 0, MPI_INT, id_camarero_postre, accion_soltar_cucharilla, MPI_COMM_WORLD);
        MPI_Recv(NULL, 0, MPI_INT, id_camarero_postre, accion_cucharilla_devuelta, MPI_COMM_WORLD, &estado);

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

void funcion_camarero_postre(int id) {
    int valor;
    int cucharillas_ocupadas = 0;
    MPI_Status estado;

    while (true) {
        //Si hay 3 cucharillas ocupadas sólo podrá soltar. Si no también podrá solicitarlas
        if (cucharillas_ocupadas < 3) {
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);
        } else {
            MPI_Probe(MPI_ANY_SOURCE, accion_soltar_cucharilla, MPI_COMM_WORLD, &estado);
        }

        valor = estado.MPI_SOURCE;
        if (estado.MPI_TAG == accion_coger_cucharilla) {
            MPI_Recv(NULL, 0, MPI_INT, valor, accion_coger_cucharilla, MPI_COMM_WORLD, &estado);
            cucharillas_ocupadas++;

            MPI_Send(NULL, 0, MPI_INT, valor, accion_cucharilla_otorgada, MPI_COMM_WORLD);
            cout << "El filosofo " << valor << " coge una cucharilla (" << cucharillas_ocupadas << " ocupadas de 3)" << endl;
        }
        if (estado.MPI_TAG == accion_soltar_cucharilla) {
            MPI_Recv(NULL, 0, MPI_INT, valor, accion_soltar_cucharilla, MPI_COMM_WORLD, &estado);
            cucharillas_ocupadas--;

            MPI_Send(NULL, 0, MPI_INT, valor, accion_cucharilla_devuelta, MPI_COMM_WORLD);
            cout << "El filosofo " << valor << " devuelve la cucharilla (" << cucharillas_ocupadas << " ocupadas de 3)" << endl;
        }
    }
}

// ---------------------------------------------------------------------
int main( int argc, char** argv ) {
    int id_propio, num_procesos_actual ;

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
    MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


    if (num_procesos_actual == 12) {
        // ejecutar la función correspondiente a 'id_propio'
        if (id_propio == id_camarero) {
            funcion_camarero(id_propio);
        } else if (id_propio == id_camarero_postre) {
            funcion_camarero_postre(id_propio);
        } else if (id_propio % 2 == 0) {
            funcion_filosofos( id_propio ); //   es un filósofo
        } else {                               // si es impar
            funcion_tenedores( id_propio ); //   es un tenedor
        }
    } else {
        if ( id_propio == 0 ) { // solo el primero escribe error, indep. del rol
            cout << "el número de procesos esperados es:    " << num_procesos + 2 << endl
            << "el número de procesos en ejecución es: " << num_procesos_actual << endl
            << "(programa abortado)" << endl ;
        }
    }

    MPI_Finalize( );
    return 0;
}

// ---------------------------------------------------------------------
