#include <iostream>
#include <random>
#include "HoareMonitor.hpp"

using namespace std;
using namespace HM;

template<int min, int max> int aleatorio() {
    static default_random_engine generador((random_device())());
    static uniform_int_distribution<int> distribucion_uniforme(min, max) ;
    return distribucion_uniforme(generador);
}

class Estanco : public HoareMonitor {
    private:
        int ingrediente_anterior;
        CondVar cond_estanquero;
        CondVar cond_fumador[3];

    public:
        Estanco() {
            ingrediente_anterior = -1;
            cond_estanquero = newCondVar();
            for (int i = 0; i < 3; i++)
                cond_fumador[i] = newCondVar();
        }

        void obtenerIngrediente(int ingrediente) {
            if (ingrediente != ingrediente_anterior) {
                cond_fumador[ingrediente].wait();
            }

            //El fumador fuma el inrediente y vuelve a -1
            ingrediente_anterior = -1;
            cond_estanquero.signal();
        }

        void ponerIngrediente(int ingrediente) {
            ingrediente_anterior = ingrediente;
            cond_fumador[ingrediente].signal();
        }

        void esperarRecogidaIngrediente() {
            if (ingrediente_anterior != -1) {
                cond_estanquero.wait();
            }
        }

};

//Función que realiza el estanquero
void producirIngrediente(MRef<Estanco> estanco) {
    while (true) {
        int ingrediente = aleatorio<0,2>();
        estanco->ponerIngrediente(ingrediente);
        cout << "Estanquero pone ingrediente " << ingrediente << endl << flush;
        estanco->esperarRecogidaIngrediente();
    }
}

//Función que realiza cada fumador
void fumar(MRef<Estanco> estanco, int ingrediente) {
    while (true) {
        estanco->obtenerIngrediente(ingrediente);
        const int ms = aleatorio<0,3000>();
        cout << "Fumando ingrediente " << ingrediente << " (" << ms << "ms)"<< endl << flush;
        this_thread::sleep_for(chrono::milliseconds(ms));
    }
}

int main() {
    MRef<Estanco> estanco = Create<Estanco>();
    thread hebra_estanquero(producirIngrediente, estanco);
    thread hebra_humadores[3];

    for (int i = 0; i < 3; i++) {
        hebra_humadores[i] = thread(fumar, estanco, i);
    }
    for (int i = 0; i < 3; i++) {
        hebra_humadores[i].join();
    }
    hebra_estanquero.join();
}
