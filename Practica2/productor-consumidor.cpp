#include <iostream>
#include <random>
#include "HoareMonitor.hpp"

using namespace std;
using namespace HM;

template< int min, int max > int aleatorio() {
    static default_random_engine generador((random_device())());
    static uniform_int_distribution<int> distribucion_uniforme(min, max) ;
    return distribucion_uniforme(generador);
}

class BufferMonitor : public HoareMonitor {
    private:
        int slots;
        int contador = 0;
        int * buffer;
        CondVar produciendo;
        CondVar consumiendo;
    public:
        BufferMonitor(int slots) {
            this->slots = slots;
            produciendo = newCondVar();
            consumiendo = newCondVar();

            buffer = new int[slots];
        }

        ~BufferMonitor() {
            delete[] buffer;
        }

        int producir() {
            if (contador == slots) {
                produciendo.wait();
            }
            int valor = aleatorio<0,9>();
            buffer[contador] = valor;
            contador++;
            consumiendo.signal();
            return valor;
        }

        int consumir() {
            int valor;
            if (contador == 0) {
                consumiendo.wait();
            }
            contador--;
            valor = buffer[contador];
            produciendo.signal();
            return valor;
        }
};

void func_productora(MRef<BufferMonitor> buffer) {
    while (true) {
        int producido = buffer->producir();
        cout << "producido: " << producido << endl << flush ;
        const int ms = aleatorio<0,500>();
        this_thread::sleep_for(chrono::milliseconds(ms));
    }
}

void func_consumidora(MRef<BufferMonitor> buffer) {
    while (true) {
        int consumido = buffer->consumir();
        cout << "                  consumido: " << consumido << endl ;
        const int ms = aleatorio<0,500>();
        this_thread::sleep_for(chrono::milliseconds(ms));
    }
}

int main() {
    MRef<BufferMonitor> buffer = Create<BufferMonitor>(30);
    thread hebra_productora(func_productora, buffer);
    thread hebra_consumidora(func_consumidora, buffer);

    hebra_productora.join();
    hebra_consumidora.join();
}
