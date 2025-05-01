#include <iostream>
#include <chrono>
#include <algorithm>
#include <random>
#include <cstdint>
#include "LEER_GUARDARdisco.c++"


// implementaciones de ordenamiento que tendremos 
void merge();
void quick();

int aridad_optima_mergeSort = -1;  // aridad optima para los ordenamientos (a) valor global. 
//  mejor aridad 𝑎 de Mergesort externo


int main() {
    const int MB = 1024 * 1024; // 1MB = 1024 * 1024 bytes
    const int M = 50 * MB; // 50MB
    const int B = 4096; // tamaño del bloque en bytes (4KB)
    const int64_t tamañoEntero = sizeof(int64_t); // tamaño de un entero (64 bits)
    const int64_t N = (4 * M) / tamañoEntero;  // ejemplo con tamaño 4M

    std::cout << "Generando secuencia de " << N << " elementos (64 bits)\n";

    // Crear arreglo ordenado
    int64_t* arreglo = new int64_t[N];
    for (int64_t i = 0; i < N; ++i)
        arreglo[i] = i;

    // desordenar el arreglo
    std::shuffle(arreglo, arreglo + N, std::mt19937_64(std::random_device{}())); // desordenar

    // Guardar en disco, y tiempo total. 
    auto t1 = std::chrono::high_resolution_clock::now();
    guardarEnDisco(reinterpret_cast<int*>(arreglo), N, B);
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "Escritura(Guardar en disco) Tiempo: " << std::chrono::duration<double>(t2 - t1).count() << " s\n";

    //MergesortExt
    // contadores en 0
    contadorEscritura_Lectura.contadorLectura = 0;
    contadorEscritura_Lectura.contadorEscritura = 0;
    auto t3 = std::chrono::high_resolution_clock::now();
    merge();
    auto t4 = std::chrono::high_resolution_clock::now();
    std::cout << "Merge Tiempo: " << std::chrono::duration<double>(t4 - t3).count() << " s\n";
    std::cout << "Merge Accesos a disco: " << contadorEscritura_Lectura.contadorLectura << "\n";

    // QuicksortExt
    // contadores en 0
    contadorEscritura_Lectura.contadorLectura = 0;
    contadorEscritura_Lectura.contadorEscritura = 0;
    auto t5 = std::chrono::high_resolution_clock::now();
    quick();
    auto t6 = std::chrono::high_resolution_clock::now();
    std::cout << "Quicksort Tiempo -------->" << std::chrono::duration<double>(t6 - t5).count() << " s\n";
    std::cout << "Quicksort Accesos a disco ----------->" << contadorEscritura_Lectura.contadorLectura << "\n";

    // eliminar el arreglo, new con delete, pues es memo dinamica
    delete[] arreglo;
    return 0;
}