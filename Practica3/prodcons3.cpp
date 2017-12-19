
#include <iostream>
#include <cmath>
#include "mpi.h"

using namespace std;

const int PRODUCTOR_TAG = 1;
const int CONSUMIDOR_TAG = 2;
const int BUFFER_PROC_ID = 5;

void productor(int rank) {
    for(int i = 0; i < 4; i++) {
        cout << "Productor " << rank << " produce valor " << i << endl << flush;
        MPI_Ssend(&i, 1, MPI_INT, BUFFER_PROC_ID, PRODUCTOR_TAG, MPI_COMM_WORLD);
    }
}

void consumidor(int rank) {
    int valor;
    int peticion=1;
    MPI_Status status;

    for (int i = 0; i < 5; i++) {
        MPI_Ssend(&peticion, 1, MPI_INT, BUFFER_PROC_ID, CONSUMIDOR_TAG, MPI_COMM_WORLD);

        MPI_Recv (&valor, 1, MPI_INT, BUFFER_PROC_ID, 0, MPI_COMM_WORLD, &status);
        cout << "Valor " << valor << " consubido por C-" << rank << endl << flush;
    }
}

void buffer() {
    const int TAM = 5;
    int valor[TAM];
    int peticion;
    int pos = 0; 
    int tipo;
    MPI_Status status;

    for (int i = 0; i < 40; i++) {
        if (pos == 0) {
            tipo = 0;
        } else if (pos == TAM) {
            tipo = 1;
        } else {
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == PRODUCTOR_TAG) {
                tipo = 0;
            } else {
                tipo = 1;
            }
        }

        if (tipo == 0) {
            //recibe
            MPI_Recv(&value[pos], 1, MPI_INT, MPI_ANY_SOURCE, PRODUCTOR_TAG, MPI_COMM_WORLD, &status);
            cout << "Valor " << valor[pos] << " recibido de P-" << status.MPI_SOURCE << endl << flush;
            pos++;
        } else if (tipo == 1) {
            //envia
            MPI_Recv(&peticion, 1, MPI_INT, MPI_ANY_SOURCE, CONSUMIDOR_TAG, MPI_COMM_WORLD, &status);
            MPI_Ssend(&value[pos-1], 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
            cout << "Valor " << valor[pos-1] << " enviado a C-" << status.MPI_SOURCE << endl << flush;
            pos--;
        }
    }
}

int main(int argc, char *argv[]) {

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size!=10) {
        if (rank == 0) cout << "-np 10 " << endl;
    } else {
        if (rank < BUFFER_PROC_ID) {
            productor(rank);
        } else if (rank == BUFFER_PROC_ID) {
            buffer();
        } else {
            consumidor(rank);
        }
    }

    MPI_Finalize();
    return 0;
}