// Proyecto: SIMPLE PLAYER
// By. Misael Matamoros
// Fecha: 19 de julio de 2025
// Descripción: Una interfaz de línea de comandos para un reproductor de música en modo de texto, que permite cargar, reproducir, pausar y gestionar música mp3 en una lista de reproducción.
//
// Requiere la biblioteca nlohmann/json para manejar JSON
// Compilación: g++ -Wall -Wextra -std=c++17 simpleplayer.cpp -o ./bin/simpleplayer

#include <iostream>          // Para entrada/salida estándar (cout, cin, endl)
#include <fstream>           // Para manejo de archivos (ifstream, ofstream)
#include <vector>            // Para usar el contenedor vector
#include <string>            // Para manipulación de cadenas de texto (string)
#include <algorithm>         // Para algoritmos estándar (como std::shuffle)
#include <random>            // Para generación de números aleatorios (std::random_device, std::mt19937)
#include <termios.h>         // Para manipular la terminal (lectura de teclas sin ENTER)
#include <unistd.h>          // Para llamadas al sistema POSIX (fork, exec, sleep, etc.)
#include <thread>            // Para manejo de hilos (std::thread)
#include <atomic>            // Para variables atómicas (sincronización entre hilos)
#include <csignal>           // Para manejo de señales (kill, SIGKILL, etc.)
#include <sys/wait.h>        // Para esperar procesos hijos (waitpid)
#include "./vendor/json.hpp" // Para usar la clase json de nlohmann/json
#include <chrono>            // Para medir y manipular tiempo (std::chrono)
#include <mutex>             // Para exclusión mutua entre hilos (std::mutex)
#include <condition_variable>// Para sincronización entre hilos (std::condition_variable)

// Para manipulación de rutas de archivos y directorios
#include <libgen.h>          // Para dirname() y basename()
#include <climits>           // Para PATH_MAX
#include <libgen.h>          // Para dirname() y basename()

using json = nlohmann::json;
using namespace std;

// --- Clases principales ---

class Cancion {
public:
    string titulo;
    string artista;
    string archivo;
    string directorio;  // Agregar campo directorio
    double duracion_minutos;

    Cancion() = default;
    Cancion(string t, string a, string f, string d, double dur)
        : titulo(t), artista(a), archivo(f), directorio(d), duracion_minutos(dur) {}
};

class NodoCancion {
public:
    Cancion cancion;
    NodoCancion* anterior;
    NodoCancion* siguiente;

    NodoCancion(const Cancion& c) : cancion(c), anterior(nullptr), siguiente(nullptr) {}
};

class Playlist {
public:
    NodoCancion* cabeza;
    NodoCancion* cola;
    NodoCancion* actual;
    int totalCanciones;

    Playlist() : cabeza(nullptr), cola(nullptr), actual(nullptr), totalCanciones(0) {}

    ~Playlist() { limpiar(); }

    void limpiar() {
        NodoCancion* temp = cabeza;
        while (temp) {
            NodoCancion* sig = temp->siguiente;
            delete temp;
            temp = sig;
        }
        cabeza = cola = actual = nullptr;
        totalCanciones = 0;
    }

    void agregarCancion(const Cancion& c) {
        NodoCancion* nuevo = new NodoCancion(c);
        if (!cabeza) {
            cabeza = cola = nuevo;
        } else {
            cola->siguiente = nuevo;
            nuevo->anterior = cola;
            cola = nuevo;
        }
        totalCanciones++;
        if (!actual) actual = cabeza;
    }

    void eliminarPorIndice(int idx) {
        if (idx < 1 || idx > totalCanciones) return;
        NodoCancion* temp = cabeza;
        int i = 1;
        while (temp && i < idx) {
            temp = temp->siguiente;
            i++;
        }
        if (!temp) return;
        if (temp->anterior) temp->anterior->siguiente = temp->siguiente;
        else cabeza = temp->siguiente;
        if (temp->siguiente) temp->siguiente->anterior = temp->anterior;
        else cola = temp->anterior;
        if (actual == temp) actual = temp->siguiente ? temp->siguiente : temp->anterior;
        delete temp;
        totalCanciones--;
        if (totalCanciones == 0) actual = nullptr;
    }

    void mostrarPlaylist() {
        NodoCancion* temp = cabeza;
        int i = 1;
        while (temp) {
            cout << i << ". " << temp->cancion.titulo << " - " << temp->cancion.artista << endl;
            temp = temp->siguiente;
            i++;
        }
    }

    double duracionTotal() {
        double total = 0;
        NodoCancion* temp = cabeza;
        while (temp) {
            total += temp->cancion.duracion_minutos;
            temp = temp->siguiente;
        }
        return total;
    }

    int contar() { return totalCanciones; }

    void guardar(const string& ruta) {
        json j;
        NodoCancion* temp = cabeza;
        while (temp) {
            j.push_back({
                {"titulo", temp->cancion.titulo},
                {"artista", temp->cancion.artista},
                {"archivo", temp->cancion.archivo},
                {"directorio", temp->cancion.directorio},  // Incluir directorio
                {"duracion_minutos", temp->cancion.duracion_minutos}
            });
            temp = temp->siguiente;
        }
        ofstream f(ruta);
        f << j.dump(4);
    }

    void cargar(const string& ruta) {
        limpiar();
        ifstream f(ruta);
        if (!f.is_open()) return;
        json j;
        f >> j;
        for (auto& item : j) {
            agregarCancion(Cancion(
                item["titulo"], 
                item["artista"], 
                item["archivo"], 
                item["directorio"],  // Cargar directorio
                item["duracion_minutos"]
            ));
        }
    }

    // Devuelve el índice (1-based) de la canción actual
    int indiceActual() {
        int idx = 1;
        NodoCancion* temp = cabeza;
        while (temp && temp != actual) {
            temp = temp->siguiente;
            idx++;
        }
        return temp ? idx : 0;
    }

    // Devuelve el nodo en la posición idx (1-based)
    NodoCancion* nodoEn(int idx) {
        int i = 1;
        NodoCancion* temp = cabeza;
        while (temp && i < idx) {
            temp = temp->siguiente;
            i++;
        }
        return temp;
    }

    // Devuelve un vector con punteros a los nodos en orden actual
    vector<NodoCancion*> nodosVector() {
        vector<NodoCancion*> v;
        NodoCancion* temp = cabeza;
        while (temp) {
            v.push_back(temp);
            temp = temp->siguiente;
        }
        return v;
    }
};

// --- Funciones para el cronometro ---

atomic<int> tiempoActualSegundos(0);
atomic<bool> contadorActivo(false);
atomic<bool> contadorSalir(false);
atomic<bool> contadorResetear(false);
mutex mtxContador;
condition_variable cvContador;

// Función para mostrar el tiempo actual en formato 0h 0m 00s
void mostrarTiempoActual(int segundos) {
    int h = segundos / 3600;
    int m = (segundos % 3600) / 60;
    int s = segundos % 60;
    // Sube 5 líneas (ajusta si tu interfaz cambia)
    cout << "\033[9A\r";
    cout << "Tiempo actual: " << h << "h " << m << "m " << s << "s      ";
    cout << "\033[9B" << flush; // Regresa a la posición original
}

// Hilo contador de tiempo
void hiloContador(int duracionSegundos) {
    tiempoActualSegundos = 0;
    while (!contadorSalir) {
        unique_lock<mutex> lk(mtxContador);
        cvContador.wait_for(lk, chrono::seconds(1), []{ return !contadorActivo || contadorSalir || contadorResetear; });
        if (contadorSalir) break;
        if (contadorResetear) {
            tiempoActualSegundos = 0;
            contadorResetear = false;
            continue;
        }
        if (contadorActivo) {
            tiempoActualSegundos++;
            if (tiempoActualSegundos > duracionSegundos) break;
            mostrarTiempoActual(tiempoActualSegundos);
        }
    }
}

// --- Función para obtener la ruta del ejecutable ---

string obtenerRutaEjecutable() {
    char resultado[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", resultado, PATH_MAX);
    if (count != -1) {
        resultado[count] = '\0';
        return string(dirname(resultado));
    }
    return "."; // Fallback al directorio actual
}

// --- Utilidades de consola ---

void limpiarPantalla() {
    cout << "\033[2J\033[1;1H";
}

void pausa() {
    cout << "Presiona ENTER para continuar...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// --- Cargar canciones disponibles ---
vector<Cancion> cargarCancionesDisponibles(const string& ruta) {
    vector<Cancion> canciones;
    ifstream archivo(ruta);
    if (!archivo.is_open()) {
        cerr << "No se pudo abrir el archivo de canciones." << endl;
        return canciones;
    }
    json j;
    archivo >> j;
    for (auto& item : j) {
        canciones.push_back(Cancion(
            item["titulo"], 
            item["artista"], 
            item["archivo"], 
            item["directorio"],  // Cargar directorio
            item["duracion_minutos"]
        ));
    }
    return canciones;
}

// --- Detección de tecla sin ENTER ---

char leerTecla() {
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

// --- Reproducción de audio con mpg123 (en hilo) ---

atomic<bool> reproduciendo(false);
atomic<bool> detener(false);
atomic<bool> pausado(false);
pid_t pid_mpg123 = 0;

void reproducirCancion(const Cancion& cancion) {
    string ruta = cancion.directorio + "/" + cancion.archivo;  // Usar directorio de la canción
    pid_mpg123 = fork();
    if (pid_mpg123 == 0) {
        execlp("mpg123", "mpg123", "-q", ruta.c_str(), nullptr);
        exit(1);
    }
    // Iniciar el contador con la reproducción
    contadorActivo = true;
    cvContador.notify_all();
}

void detenerCancion() {
    if (pid_mpg123 > 0) {
        kill(pid_mpg123, SIGKILL);
        waitpid(pid_mpg123, nullptr, 0);
        pid_mpg123 = 0;
    }
}

void pausarCancion() {
    if (pid_mpg123 > 0) kill(pid_mpg123, SIGSTOP);
}

void reanudarCancion() {
    if (pid_mpg123 > 0) kill(pid_mpg123, SIGCONT);
}

// --- Modo reproductor interactivo ---

void mostrarVistaReproductor(Playlist& pl, bool shuffle, int idx, NodoCancion* nodo) {
    limpiarPantalla();
    cout << "=== SIMPLE PLAYER ===" << endl;
    cout << "Tu playlist actual contiene " << pl.contar() << " canciones," << endl;
    cout << "con un total de " << (int)pl.duracionTotal() << " minutos de música." << endl;
    cout << "------------------------------------------" << endl;
    cout << "Canción actual: " << idx << endl;
    cout << "Título: " << nodo->cancion.titulo << endl;
    cout << "Artista: " << nodo->cancion.artista << endl;
    int min = (int)nodo->cancion.duracion_minutos;
    int seg = (int)((nodo->cancion.duracion_minutos - min) * 60);
    cout << "Duración: 0h " << min << "m " << seg << "s" << endl;
    // Línea de tiempo actual
    cout << "Tiempo actual: 0h 0m 0s" << endl;
    cout << "------------------------------------------" << endl;
    cout << "Presiona un comando en cualquier momento:" << endl;
    cout << endl;
    cout << "[R] = Reproducir | [P] = Pausar" << endl;
    cout << "[S] = Siguiente  | [A] = Anterior" << endl;
    cout << "[M] = Modo aleatorio [" << (shuffle ? "On" : "Off") << "]" << endl;
    cout << "[Q] = Detener" << endl;
    cout << "------------------------------------------" << endl;
}

void modoReproductor(Playlist& pl) {
    if (pl.contar() == 0) {
        cout << "Tu playlist está vacía." << endl;
        pausa();
        return;
    }
    bool shuffle = false;
    vector<NodoCancion*> orden;
    int idx = pl.indiceActual();
    NodoCancion* nodo = pl.actual ? pl.actual : pl.cabeza;
    int idxShuffle = 0;
    string ultimaCancion = nodo->cancion.archivo;

    auto recalcularShuffle = [&]() {
        orden = pl.nodosVector();
        random_device rd;
        mt19937 g(rd());
        std::shuffle(orden.begin(), orden.end(), g);
        idxShuffle = 0;
        for (size_t i = 0; i < orden.size(); ++i) {
            if (orden[i]->cancion.archivo == ultimaCancion) {
                idxShuffle = i;
                break;
            }
        }
    };

    if (shuffle) recalcularShuffle();

    bool salir = false;

    // --- INICIO: Reproducir automáticamente al entrar ---
    reproduciendo = true;
    pausado = false;
    int duracionSegundos = (int)(nodo->cancion.duracion_minutos * 60);
    contadorSalir = false;
    contadorResetear = false;
    contadorActivo = true;
    thread thContador(hiloContador, duracionSegundos);
    reproducirCancion(nodo->cancion);
    // --- FIN ---

    while (!salir) {
        if (!shuffle) {
            idx = pl.indiceActual();
            nodo = pl.nodoEn(idx);
        } else {
            if (orden.empty()) recalcularShuffle();
            nodo = orden[idxShuffle];
            idx = idxShuffle + 1;
        }
        mostrarVistaReproductor(pl, shuffle, idx, nodo);
        mostrarTiempoActual(tiempoActualSegundos);

        char tecla = leerTecla();
        switch (tecla) {
            case 'r':
            case 'R':
                if (reproduciendo) detenerCancion();
                reproduciendo = true;
                pausado = false;
                contadorResetear = true;
                contadorActivo = true;
                cvContador.notify_all();
                duracionSegundos = (int)(nodo->cancion.duracion_minutos * 60);
                reproducirCancion(nodo->cancion);
                break;
            case 'p':
            case 'P':
                if (reproduciendo && !pausado) {
                    pausarCancion();
                    pausado = true;
                    contadorActivo = false;
                } else if (reproduciendo && pausado) {
                    reanudarCancion();
                    pausado = false;
                    contadorActivo = true;
                    cvContador.notify_all();
                }
                break;
            case 's':
            case 'S':
                if (reproduciendo) detenerCancion();
                if (!shuffle) {
                    if (nodo->siguiente) {
                        pl.actual = nodo->siguiente;
                        ultimaCancion = pl.actual->cancion.archivo;
                        reproduciendo = true;
                        pausado = false;
                        contadorResetear = true;
                        contadorActivo = true;
                        cvContador.notify_all();
                        nodo = pl.actual;
                        duracionSegundos = (int)(nodo->cancion.duracion_minutos * 60);
                        reproducirCancion(pl.actual->cancion);
                    } else {
                        // Pausar el contador antes de mostrar el mensaje
                        contadorActivo = false;
                        cvContador.notify_all();
                        cout << "\rFin de la lista." << endl;
                        pausa();
                    }
                } else {
                    if (idxShuffle + 1 < (int)orden.size()) {
                        idxShuffle++;
                        ultimaCancion = orden[idxShuffle]->cancion.archivo;
                        reproduciendo = true;
                        pausado = false;
                        contadorResetear = true;
                        contadorActivo = true;
                        cvContador.notify_all();
                        nodo = orden[idxShuffle];
                        duracionSegundos = (int)(nodo->cancion.duracion_minutos * 60);
                        reproducirCancion(orden[idxShuffle]->cancion);
                    } else {
                        // Pausar el contador antes de mostrar el mensaje
                        contadorActivo = false;
                        cvContador.notify_all();
                        cout << "\rFin de la lista aleatoria." << endl;
                        pausa();
                    }
                }
                break;
            case 'a':
            case 'A':
                if (reproduciendo) detenerCancion();
                if (!shuffle) {
                    if (nodo->anterior) {
                        pl.actual = nodo->anterior;
                        ultimaCancion = pl.actual->cancion.archivo;
                        reproduciendo = true;
                        pausado = false;
                        contadorResetear = true;
                        contadorActivo = true;
                        cvContador.notify_all();
                        nodo = pl.actual;
                        duracionSegundos = (int)(nodo->cancion.duracion_minutos * 60);
                        reproducirCancion(pl.actual->cancion);
                    } else {
                        // Pausar el contador antes de mostrar el mensaje
                        contadorActivo = false;
                        cvContador.notify_all();
                        cout << "\rInicio de la lista." << endl;
                        pausa();
                    }
                } else {
                    if (idxShuffle > 0) {
                        idxShuffle--;
                        ultimaCancion = orden[idxShuffle]->cancion.archivo;
                        reproduciendo = true;
                        pausado = false;
                        contadorResetear = true;
                        contadorActivo = true;
                        cvContador.notify_all();
                        nodo = orden[idxShuffle];
                        duracionSegundos = (int)(nodo->cancion.duracion_minutos * 60);
                        reproducirCancion(orden[idxShuffle]->cancion);
                    } else {
                        // Pausar el contador antes de mostrar el mensaje
                        contadorActivo = false;
                        cvContador.notify_all();
                        cout << "\rInicio de la lista aleatoria." << endl;
                        pausa();
                    }
                }
                break;
            case 'm':
            case 'M':
                shuffle = !shuffle;
                if (shuffle) {
                    recalcularShuffle();
                } else {
                    NodoCancion* buscar = pl.cabeza;
                    while (buscar && buscar->cancion.archivo != ultimaCancion) buscar = buscar->siguiente;
                    if (buscar) pl.actual = buscar;
                }
                break;
            case 'q':
            case 'Q':
                if (reproduciendo) detenerCancion();
                contadorSalir = true;
                contadorActivo = false;
                cvContador.notify_all();
                salir = true;
                break;
            default:
                break;
        }
    }
    // Finalizar hilo contador
    contadorSalir = true;
    contadorActivo = false;
    cvContador.notify_all();
    if (thContador.joinable()) thContador.join();
}

// --- Menú principal ---

void menuPrincipal() {
    cout << "=== SIMPLE PLAYER ===" << endl;
    cout << "1. Mostrar canciones disponibles" << endl;
    cout << "2. Agregar canción a mi playlist" << endl;
    cout << "3. Mostrar playlist completa" << endl;
    cout << "4. Mostrar duración total de la lista" << endl;
    cout << "5. Reproducir playlist" << endl;
    cout << "6. Guardar mi lista" << endl;
    cout << "7. Eliminar canción de mi playlist" << endl;
    cout << "8. Salir" << endl;
    cout << "Seleccione una opción: ";
}

int main() {
    string rutaEjecutable = obtenerRutaEjecutable();
    string rutaCanciones = rutaEjecutable + "/canciones.json";
    string rutaPlaylist = rutaEjecutable + "/playlist.json";

    vector<Cancion> cancionesDisponibles = cargarCancionesDisponibles(rutaCanciones);
    Playlist miPlaylist;
    miPlaylist.cargar(rutaPlaylist);

    int opcion;
    do {
        limpiarPantalla();
        menuPrincipal();
        cin >> opcion;
        cin.ignore();
        switch (opcion) {
            case 1:
                limpiarPantalla();
                cout << "Canciones disponibles:" << endl;
                for (size_t i = 0; i < cancionesDisponibles.size(); ++i) {
                    cout << i+1 << ". " << cancionesDisponibles[i].titulo << " - " << cancionesDisponibles[i].artista << endl;
                }
                pausa();
                break;
            case 2: {
                limpiarPantalla();
                cout << "¿Qué canción deseas agregar? Ingresa el número: ";
                int num;
                cin >> num;
                cin.ignore();
                if (num >= 1 && num <= (int)cancionesDisponibles.size()) {
                    miPlaylist.agregarCancion(cancionesDisponibles[num-1]);
                    cout << "Agregada: " << cancionesDisponibles[num-1].titulo << endl;
                } else {
                    cout << "Número inválido." << endl;
                }
                pausa();
                break;
            }
            case 3:
                limpiarPantalla();
                cout << "Tu playlist:" << endl;
                miPlaylist.mostrarPlaylist();
                pausa();
                break;
            case 4:
                limpiarPantalla();
                cout << "Duración total: " << (int)miPlaylist.duracionTotal() << " minutos" << endl;
                pausa();
                break;
            case 5:
                modoReproductor(miPlaylist);
                break;
            case 6:
                miPlaylist.guardar(rutaPlaylist);
                cout << "Playlist guardada en " << rutaPlaylist << endl;
                pausa();
                break;
            case 7:
                limpiarPantalla();
                cout << "Tu playlist:" << endl;
                miPlaylist.mostrarPlaylist();
                cout << "¿Qué canción deseas eliminar? Ingresa el número: ";
                int num;
                cin >> num;
                cin.ignore();
                miPlaylist.eliminarPorIndice(num);
                cout << "Eliminada (si existía)." << endl;
                pausa();
                break;
            case 8:
                miPlaylist.guardar(rutaPlaylist);
                cout << "¡Hasta luego!" << endl;
                break;
            default:
                cout << "Opción no válida." << endl;
                pausa();
        }
    } while (opcion != 8);

    return 0;
}