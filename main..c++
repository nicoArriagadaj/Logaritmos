#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <random>
#include <cstdint>
#include "LEER_GUARDARdisco.c++"

// implementaciones de ordenamiento que tendremos 
void MergeSort();
void QuickSort();

int aridad_optima_mergeSort = -1;  // aridad optima para los ordenamientos (a) valor global. 
//  mejor aridad 𝑎 de Mergesort externo

int main() {
    const int MB = 1024 * 1024;        // 1MB = 1024 * 1024 bytes
    const int M = 50 * MB;            // 50MB
    const int B = 4096;              // tamaño del bloque en bytes (4KB)
    const int64_t tamañoEntero = sizeof(int64_t);  // tamaño de un entero (64 bits)

    // Abre o crear un archivo de texto llamado resultados.csv para escritura, Si el archivo ya existe, lo sobrescribe desde cero.
    std::ofstream resultados("resultados.csv");
    // Escribe la primera línea CSV, que corresponde a los nombres de las columnas.
    resultados << "Tamaño_MB,Secuencia,Algoritmo,Tiempo_segundos,Accesos_disco\n";

    // Crea el encabezado para el archivo de promedios
    std::ofstream encabezado("promedios.csv");
    encabezado << "N,Algoritmo,Promedio_Tiempo,Promedio_Accesos\n";
    encabezado.close();

    // Inicializa el generador de números aleatorios con una semilla aleatoria.
    std::mt19937_64 rng(std::random_device{}());

    int64_t* arregloQuickTiempo = new int64_t[5]; // lo lleno yo, el tiempo 
    int64_t* arregloAccesoDiscoQuick = new int64_t[5]; // debe llenarlo quicksort
    int64_t* arregloAccesoDiscoMerge = new int64_t[5];  // debe llenarlo mergesort
    int64_t* arregloMergeTiempo = new int64_t[5];  // lo lleno yo, el tiempo 

    // recorrer 4M 8M 12M ....60M
    for (int mult = 4; mult <= 60; mult += 4) { 
        const int64_t N = (int64_t(mult) * M) / tamañoEntero;

        // por cada 4M, 8M..60M se hace 5 experimentos
        for (int secuencia = 1; secuencia <= 5; ++secuencia) {
            std::cout << "\n-------------------------------\n";
            std::cout << ">> Tamaño: " << mult << "MB | Secuencia #" << secuencia << "\n";

            // Crear arreglo ordenado
            int64_t* arreglo = new int64_t[N];
            for (int64_t i = 0; i < N; ++i)
                arreglo[i] = i;

            // desordenar el arreglo
            std::shuffle(arreglo, arreglo + N, rng);

            // Guardar en disco, y tiempo total. 
            auto t1 = std::chrono::high_resolution_clock::now();
            guardarEnDisco((arreglo), N, B);  // se escribe en "datos.bin"
            auto t2 = std::chrono::high_resolution_clock::now();
            std::cout << "Escritura(Guardar en disco) Tiempo: " << std::chrono::duration<double>(t2 - t1).count() << " s\n";

            delete[] arreglo;  // liberar memoria

            // Mergesort externo
            // contadores en 0
            contadorEscritura_Lectura.contadorLectura = 0;
            contadorEscritura_Lectura.contadorEscritura = 0;
            auto t3 = std::chrono::high_resolution_clock::now();
            MergeSort();  // se asume lectura y ordenamiento externo dentro de esta función
            auto t4 = std::chrono::high_resolution_clock::now();
            std::cout << "MergeExt Tiempo: " << std::chrono::duration<double>(t4 - t3).count() << " s\n";
            std::cout << "MergeExt Accesos a disco: " << contadorEscritura_Lectura.contadorLectura << "\n";
            
            // esto guarda en resultados -> resultados.csv TODO los datos del algoritmo
            resultados << mult << "," << secuencia << ",Merge," 
                       << std::chrono::duration<double>(t4 - t3).count() << "," 
                       << contadorEscritura_Lectura.contadorLectura << "\n";
            arregloMergeTiempo[secuencia - 1] = std::chrono::duration<double>(t4 - t3).count(); // guardar tiempo de MergeSort
            arregloAccesoDiscoMerge[secuencia - 1] = contadorEscritura_Lectura.contadorLectura; // guardar acceso a disco de mergesort

            // Quicksort externo
            // contadores en 0
            contadorEscritura_Lectura.contadorLectura = 0;
            contadorEscritura_Lectura.contadorEscritura = 0;
            auto t5 = std::chrono::high_resolution_clock::now();
            QuickSort();  // también trabaja en disco usando mismo archivo binario
            auto t6 = std::chrono::high_resolution_clock::now();
            std::cout << "Quicksort Tiempo --------> " << std::chrono::duration<double>(t6 - t5).count() << " s\n";
            std::cout << "Quicksort Accesos a disco -----------> " << contadorEscritura_Lectura.contadorLectura << "\n";

             // esto guarda en resultados -> resultados.csv TODO los datos del algoritmo
            resultados << mult << "," << secuencia << ",Quick," 
                       << std::chrono::duration<double>(t6 - t5).count() << "," 
                       << contadorEscritura_Lectura.contadorLectura << "\n";
            arregloQuickTiempo[secuencia - 1] = std::chrono::duration<double>(t6 - t5).count(); // guardar tiempo de QuickSort
            arregloAccesoDiscoQuick[secuencia - 1] = contadorEscritura_Lectura.contadorLectura; // guardar acceso a disco de quicksort

            std::cout << "Merge y Quick completados para " << mult << "MB - secuencia " << secuencia << "\n";
        }

        double sumaTiempoMerge = 0, sumaAccesosMerge = 0;
        double sumaTiempoQuick = 0, sumaAccesosQuick = 0;
        for (int i = 0; i < 5; i++) {
            sumaTiempoMerge += arregloMergeTiempo[i];
            sumaAccesosMerge += arregloAccesoDiscoMerge[i];
            sumaTiempoQuick += arregloQuickTiempo[i];
            sumaAccesosQuick += arregloAccesoDiscoQuick[i];
        }
        double promedioTiempoMerge = sumaTiempoMerge / 5.0;
        double promedioAccesosMerge = sumaAccesosMerge / 5.0;
        double promedioTiempoQuick = sumaTiempoQuick / 5.0;
        double promedioAccesosQuick = sumaAccesosQuick / 5.0;

        // guardar los promedios en un nuevo archivo CSV, llamado resumen.csv, aqui graficaremos.
        std::ofstream resumen("promedios.csv", std::ios::app);
        resumen << N << ",Merge," << promedioTiempoMerge << "," << promedioAccesosMerge << "\n";
        resumen << N << ",Quick," << promedioTiempoQuick << "," << promedioAccesosQuick << "\n";
        resumen.close();
    }

    resultados.close();
    std::cout << "\n Experimentos completados. Resultados en 'resultados.csv'\n";
    return 0;
}
