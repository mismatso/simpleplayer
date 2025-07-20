#include <iostream>
#include <string>
#include <iomanip>
#include <cstdlib>
#include <ctime>
using namespace std;

// Clase Cancion
class Cancion {
public:
    string titulo;
    string artista;
    int duracion; // en segundos

    Cancion(string t, string a, int d) : titulo(t), artista(a), duracion(d) {}

    void mostrar() {
        int min = duracion / 60;
        int seg = duracion % 60;
        cout << left << setw(20) << titulo
             << left << setw(20) << artista
             << right << setw(2) << min << ":" << setfill('0') << setw(2) << seg << setfill(' ') << endl;
    }
};

// Clase NodoCancion
class NodoCancion {
public:
    Cancion* cancion;
    NodoCancion* siguiente;
    NodoCancion* anterior;

    NodoCancion(Cancion* c) : cancion(c), siguiente(nullptr), anterior(nullptr) {}
};

// Clase PlaylistMusical
class PlaylistMusical {
private:
    NodoCancion* cabeza;
    NodoCancion* cola;
    NodoCancion* cancionActual;
    int totalCanciones;

public:
    PlaylistMusical() : cabeza(nullptr), cola(nullptr), cancionActual(nullptr), totalCanciones(0) {}

    void agregarCancion(string titulo, string artista, int duracion) {
        Cancion* nueva = new Cancion(titulo, artista, duracion);
        NodoCancion* nodo = new NodoCancion(nueva);
        if (!cabeza) {
            cabeza = cola = nodo;
            cancionActual = cabeza;
        } else {
            cola->siguiente = nodo;
            nodo->anterior = cola;
            cola = nodo;
        }
        totalCanciones++;
        cout << "Canción agregada exitosamente.\n";
    }

    void reproducir() {
        if (!cancionActual) {
            cout << "La playlist está vacía.\n";
            return;
        }
        cout << "Reproduciendo:\n";
        cout << left << setw(20) << "Título" << left << setw(20) << "Artista" << "Duración\n";
        cancionActual->cancion->mostrar();
    }

    void siguiente() {
        if (!cancionActual) {
            cout << "La playlist está vacía.\n";
            return;
        }
        if (!cancionActual->siguiente) {
            cout << "Ya estás en la última canción.\n";
            return;
        }
        cancionActual = cancionActual->siguiente;
        reproducir();
    }

    void anterior() {
        if (!cancionActual) {
            cout << "La playlist está vacía.\n";
            return;
        }
        if (!cancionActual->anterior) {
            cout << "Ya estás en la primera canción.\n";
            return;
        }
        cancionActual = cancionActual->anterior;
        reproducir();
    }

    void mostrarPlaylist() {
        if (!cabeza) {
            cout << "La playlist está vacía.\n";
            return;
        }
        cout << left << setw(20) << "Título" << left << setw(20) << "Artista" << "Duración\n";
        NodoCancion* temp = cabeza;
        int idx = 1;
        while (temp) {
            cout << idx << ". ";
            temp->cancion->mostrar();
            temp = temp->siguiente;
            idx++;
        }
    }

    void duracionTotal() {
        if (!cabeza) {
            cout << "La playlist está vacía.\n";
            return;
        }
        int total = 0;
        NodoCancion* temp = cabeza;
        while (temp) {
            total += temp->cancion->duracion;
            temp = temp->siguiente;
        }
        int h = total / 3600;
        int m = (total % 3600) / 60;
        int s = total % 60;
        cout << "Duración total: " << h << "h " << m << "m " << s << "s\n";
    }

    void shuffle() {
        if (!cabeza) {
            cout << "La playlist está vacía.\n";
            return;
        }
        if (totalCanciones == 1) {
            cout << "Solo hay una canción en la playlist.\n";
            return;
        }
        srand(time(0));
        int pos = rand() % totalCanciones;
        NodoCancion* temp = cabeza;
        for (int i = 0; i < pos; ++i) temp = temp->siguiente;
        cancionActual = temp;
        cout << "Canción aleatoria seleccionada:\n";
        reproducir();
    }
};

int main() {
    PlaylistMusical playlist;
    int opcion;
    do {
        cout << "\n=== REPRODUCTOR MUSICAL ===\n";
        cout << "1. Agregar canción\n";
        cout << "2. Reproducir canción actual\n";
        cout << "3. Siguiente canción\n";
        cout << "4. Canción anterior\n";
        cout << "5. Mostrar playlist completa\n";
        cout << "6. Duración total\n";
        cout << "7. Modo shuffle\n";
        cout << "8. Salir\n";
        cout << "Seleccione una opción: ";
        cin >> opcion;
        cin.ignore();

        if (opcion == 1) {
            string titulo, artista;
            int duracion;
            cout << "Título: ";
            getline(cin, titulo);
            cout << "Artista: ";
            getline(cin, artista);
            cout << "Duración (en segundos): ";
            cin >> duracion;
            cin.ignore();
            playlist.agregarCancion(titulo, artista, duracion);
        } else if (opcion == 2) {
            playlist.reproducir();
        } else if (opcion == 3) {
            playlist.siguiente();
        } else if (opcion == 4) {
            playlist.anterior();
        } else if (opcion == 5) {
            playlist.mostrarPlaylist();
        } else if (opcion == 6) {
            playlist.duracionTotal();
        } else if (opcion == 7) {
            playlist.shuffle();
        } else if (opcion == 8) {
            cout << "¡Hasta luego!\n";
        } else {
            cout << "Opción inválida.\n";
        }
    } while (opcion != 8);

    return 0;
}