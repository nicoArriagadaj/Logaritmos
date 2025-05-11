#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <random>
#include <cstdint>
#include "CHRISTIAN_CODIGO.c++"  

// implementaciones de ordenamiento que tendremos
void MergeSort();
void QuickSort();

int main() {
    const int64_t N = 64;                   // pequeño número de elementos (64 enteros de 64 bits)
    const int B = 64;                       // bloque de 64 bytes (8 enteros de 64 bits)
    const int64_t tamañoEntero = sizeof(int64_t);

    std::mt19937_64 rng(std::random_device{}());

    int64_t arreglo[N];
    for (int64_t i = 0; i < N; ++i) arreglo[i] = i;
    std::shuffle(arreglo, arreglo + N, rng);

    // Escritura a disco
    auto t1 = std::chrono::high_resolution_clock::now();
    guardarEnDisco(arreglo, N, B);
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "Escritura a disco completada en "
              << std::chrono::duration<double>(t2 - t1).count() << " segundos.\n";

    // MergeSort externo simulado
    contadorEscritura_Lectura.contadorLectura = 0;
    contadorEscritura_Lectura.contadorEscritura = 0;
    auto t3 = std::chrono::high_resolution_clock::now();
    MergeSort();
    auto t4 = std::chrono::high_resolution_clock::now();
    std::cout << "[Merge] Tiempo: "
              << std::chrono::duration<double>(t4 - t3).count()
              << " s | Lecturas: " << contadorEscritura_Lectura.contadorLectura << "\n";

    // QuickSort externo simulado
    contadorEscritura_Lectura.contadorLectura = 0;
    contadorEscritura_Lectura.contadorEscritura = 0;
    auto t5 = std::chrono::high_resolution_clock::now();
    QuickSort();
    auto t6 = std::chrono::high_resolution_clock::now();
    std::cout << "[Quick] Tiempo: "
              << std::chrono::duration<double>(t6 - t5).count()
              << " s | Lecturas: " << contadorEscritura_Lectura.contadorLectura << "\n";

    return 0;
}
