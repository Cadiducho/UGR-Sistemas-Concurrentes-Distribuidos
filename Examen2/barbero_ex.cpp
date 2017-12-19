#include <iostream>
#include <random>
#include "HoareMonitor.hpp"

using namespace std;
using namespace HM;

const int NUM_CLIENTES = 5;

template<int min, int max> int aleatorio() {
    static default_random_engine generador( (random_device())() );
    static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
    return distribucion_uniforme( generador );
}

class Barberia : public HoareMonitor {
    private:
        CondVar salaEspera;
        CondVar silla;
        CondVar barbero;
        int visitas;
    public:
        Barberia() {
            salaEspera = newCondVar();
            silla = newCondVar();
            barbero = newCondVar();
            visitas = 0;
        }

        void siguienteCliente() {
            if (salaEspera.empty() && silla.empty()) {
                cout << "El barbero de echa a dormir" << endl << flush;
                barbero.wait();
            }

            cout << "El barbero coge a un nuevo cliente" << endl << flush;
            salaEspera.signal();
        }

        void cortarPelo(int clientId) {
            const int random = aleatorio<0,1>();
            
            if (random) {
                cout << "El cliente " << clientId << " hace una visita corta" << endl << flush;
                visitas++;  
            } else {
                if (!silla.empty()) {
                    cout << "La silla está ocupada" << endl << flush;
                    salaEspera.wait();
                }

                cout << "El barbero comienza a cortar el pelo a " << clientId << "..." << endl << flush;
                barbero.signal();
                silla.wait();
			}
        }

        void visitaCorta(int clientId) {
            cout << "El cliente " << clientId << " hace una visita corta" << endl << flush;
            visitas++;
        }

        void finCliente() {
            cout << "El barbero termina con el cliente" << endl << flush;

            if (visitas >= 3) {
                cout << "Voy a prohibir las visitas" << endl << flush;
                visitas = 0;
            }
            silla.signal();
        }
};

//Función que representa al barbero
void func_barbero(MRef<Barberia> barberia) {
    while (true) {
        barberia->siguienteCliente();

        const int ms = aleatorio<0,3000>();
        cout << "Cortando el pelo (" << ms << "ms)..." << endl << flush;
        this_thread::sleep_for(chrono::milliseconds(ms));

        barberia->finCliente();
    }
}

//Función que representa a cada cliente
void func_cliente(MRef<Barberia> barberia, int cliente) {
    while (true) {
		barberia->cortarPelo(cliente);
        const int ms = aleatorio<0,3000>();
        this_thread::sleep_for(chrono::milliseconds(ms));
    }
}

int main() {
    MRef<Barberia> barberia = Create<Barberia>();
    thread hebra_barbero(func_barbero, barberia);
    thread hebra_clientes[NUM_CLIENTES];

    for (int i = 0; i < NUM_CLIENTES; i++) {
        hebra_clientes[i] = thread(func_cliente, barberia, i);
    }
    for (int i = 0; i < NUM_CLIENTES; i++) {
        hebra_clientes[i].join();
    }
    hebra_barbero.join();
}
