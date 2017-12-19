#include <mpi.h>
#include <thread>
#include <random>
#include <chrono>
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int num_usuarios = 5;
const int num_procesos = num_usuarios + 1 + 2;
const int accion_solicitar_caja = 0, accion_otorgar_caja = 1, accion_tramite = 2, accion_comunicar_fin = 3;
const int id_proceso_controlador = 5;


template< int min, int max > int aleatorio() {
    static default_random_engine generador( (random_device())() );
    static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
    return distribucion_uniforme( generador );
}

// ---------------------------------------------------------------------

void funcion_usuario(int id) {
    int caja_destino;
    MPI_Status estado;
    while ( true ) {
        //Solicita servicio
        cout << "Usuario " << id << " solicita una caja" << endl;
        MPI_Send(NULL, 0, MPI_INT, id_proceso_controlador, accion_solicitar_caja, MPI_COMM_WORLD);

        //Espera a tener la respuesta den controlador sobre a qué caja ir
        MPI_Recv(&caja_destino, 1, MPI_INT, id_proceso_controlador, accion_solicitar_caja, MPI_COMM_WORLD, &estado);
        cout << "Usuario " << id << " va a la caja " << caja_destino << endl;

        MPI_Send(NULL, 0, MPI_INT, caja_destino, accion_tramite, MPI_COMM_WORLD); //llegada a caja
        MPI_Recv(NULL, 0, MPI_INT, caja_destino, accion_tramite, MPI_COMM_WORLD, &estado); //tramite finalizado

        cout << "Usuario " << id <<" termina el tramite en caja " << caja_destino << endl;
        MPI_Send(&caja_destino, 1, MPI_INT, id_proceso_controlador, accion_comunicar_fin, MPI_COMM_WORLD);

        sleep_for(milliseconds(aleatorio<100,1000>()));
    }
}

void funcion_caja(int id) {
    int id_usuario;
    MPI_Status estado;
    while (true) {
        MPI_Recv(NULL, 0, MPI_INT, MPI_ANY_SOURCE, accion_tramite, MPI_COMM_WORLD, &estado);

        id_usuario = estado.MPI_SOURCE;
        cout << "La caja " << id << " está tramitando a " << id_usuario << endl;

        sleep_for(milliseconds(aleatorio<100,1000>()));

        MPI_Ssend(NULL, 0, MPI_INT, id_usuario, accion_tramite, MPI_COMM_WORLD);
    }
}

void funcion_controlador(int id) {
    int emisor;
    int mostradores_ocupados = 0;
    bool cajas[] = {false, false};
    MPI_Status estado;

    while (true) {
        //Si no hay mmostradores libres solo acepta mensajes de que estos han sido liberados
        if (mostradores_ocupados >= 2) {
            MPI_Probe(MPI_ANY_SOURCE, accion_comunicar_fin, MPI_COMM_WORLD, &estado);
        } else {
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);
        }

        emisor = estado.MPI_SOURCE;
        if (estado.MPI_TAG == accion_solicitar_caja) {
            MPI_Recv(NULL, 0, MPI_INT, emisor, accion_solicitar_caja, MPI_COMM_WORLD, &estado);
            mostradores_ocupados++;

            //busco cajas libres
            bool caja_encontrada = false;
            int id_buscada = -1;
            while (!caja_encontrada) {
                id_buscada++;
                if (!cajas[id_buscada]) {
                    cajas[id_buscada] = true;
                    caja_encontrada = true;
                }
            }
            int id_caja = id_buscada + 6; //0 + 6 = 6, 1 + 6 = 7;

            MPI_Ssend(&id_caja, 1, MPI_INT, emisor, accion_solicitar_caja, MPI_COMM_WORLD);
            cout << "EL controlador asigna al usuario " << emisor << " la caja " << id_caja << " (" << mostradores_ocupados << " ocupadas)" << endl;
        }
        if (estado.MPI_TAG == accion_comunicar_fin) {
            int id_caja;
            MPI_Recv(&id_caja, 1, MPI_INT, emisor, accion_comunicar_fin, MPI_COMM_WORLD, &estado);

            //establezco que la caja ID ahora está libre
            mostradores_ocupados--;
            cajas[id_caja - 6] = false;

            cout << "El controlador gestiona el fin del usuario " << emisor << " en la caja " << id_caja << " (" << mostradores_ocupados << " ocupadas)" << endl;
        }
    }
}

// ---------------------------------------------------------------------
int main( int argc, char** argv ) {
    int id_propio, num_procesos_actual ;

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
    MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


    if (num_procesos_actual == num_procesos) {
        // ejecutar la función correspondiente a 'id_propio'
        if (id_propio == id_proceso_controlador) {
            funcion_controlador(id_propio);
        } else if (id_propio == 6 || id_propio == 7) {
            funcion_caja(id_propio);  //cajas
        } else {
            funcion_usuario( id_propio ); //usuarios
        }
    } else {
        if ( id_propio == 0 ) { // solo el primero escribe error, indep. del rol
            cout << "el número de procesos esperados es:    " << num_procesos << endl
            << "el número de procesos en ejecución es: " << num_procesos_actual << endl
            << "(programa abortado)" << endl ;
        }
    }

    MPI_Finalize();
    return 0;
}
