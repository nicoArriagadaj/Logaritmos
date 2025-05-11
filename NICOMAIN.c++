#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <random>
#include <cstdint>
#include <algorithm>
#include <chrono>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>

const char *FILENAME = "datos.bin";

struct ContadorEscritura_Lectura {
  int contadorEscritura = 0;
  int contadorLectura = 0;
} contadorEscritura_Lectura;
void leerDesdeDisco(int64_t* arreglo, int64_t N, int64_t B, int64_t i = 0) {
    FILE* archivo = fopen(FILENAME, "rb");
    if (!archivo) {
        perror("Error abriendo archivo");
        exit(1);
    }

    int64_t elementosPorBloque = B / sizeof(int64_t);
    int64_t* buffer = new int64_t[elementosPorBloque];

    for (; i < N; i += elementosPorBloque) {
        int64_t cantidad = std::min(elementosPorBloque, N - i);
        int64_t offsetBytes = i * sizeof(int64_t);
        if (fseek(archivo, offsetBytes, SEEK_SET) != 0) {
            perror("fseek error");
            exit(1);
        }

        size_t leidos = fread(buffer, sizeof(int64_t), cantidad, archivo);
        if (leidos != cantidad) {
            perror("Error al leer del archivo");
            exit(1);
        }

        memcpy(&arreglo[i], buffer, cantidad * sizeof(int64_t));
        contadorEscritura_Lectura.contadorLectura++;
    }

    delete[] buffer;
    fclose(archivo);
}

void guardarEnDisco(int64_t* arreglo, int64_t N, int64_t B, int64_t i = 0) {
    FILE* archivo = fopen(FILENAME, "wb");
    if (!archivo) {
        perror("Error abriendo archivo");
        exit(1);
    }

    int64_t elementosPorBloque = B / sizeof(int64_t);
    int64_t* buffer = new int64_t[elementosPorBloque];

    while (i < N) {
        int64_t elementosPorCopiar = std::min(elementosPorBloque, N - i);

        // copiar sólo lo que toque, sin padding
        memcpy(buffer, &arreglo[i], elementosPorCopiar * sizeof(int64_t));

        int64_t offsetBytes = i * sizeof(int64_t);
        if (fseek(archivo, offsetBytes, SEEK_SET) != 0) {
            perror("fseek error");
            exit(1);
        }

        size_t escritos = fwrite(buffer, sizeof(int64_t), elementosPorCopiar, archivo);
        if (escritos != elementosPorCopiar) {
            perror("Error al escribir en el archivo");
            exit(1);
        }

        contadorEscritura_Lectura.contadorEscritura++;
        i += elementosPorCopiar;
    }

    delete[] buffer;
    fclose(archivo);
}

void crearSubarchivosOrdenados(int64_t N, int64_t B, int64_t M,
                               int &totalSubarchivos) {
  FILE *archivo = fopen(FILENAME, "rb");
  if (!archivo) {
    perror("Error abriendo archivo original");
    exit(1);
  }

  int64_t elementosEnRAM = M / sizeof(int64_t);
  int64_t *buffer = new int64_t[elementosEnRAM];
  totalSubarchivos = 0;

  for (int64_t i = 0; i < N; i += elementosEnRAM) {
    int64_t cantidad = (i + elementosEnRAM <= N) ? elementosEnRAM : N - i;
    fseek(archivo, i * sizeof(int64_t), SEEK_SET);
    if (fread(buffer, sizeof(int64_t), cantidad, archivo) != cantidad) {
      perror("Error al leer del archivo");
      exit(1);
    }
    std::sort(buffer, buffer + cantidad);

    char nombreArchivo[32];
    sprintf(nombreArchivo, "subarchivoOrdenado%d.bin", totalSubarchivos++);
    FILE *f = fopen(nombreArchivo, "wb");
    fwrite(buffer, sizeof(int64_t), cantidad, f);
    fclose(f);

    contadorEscritura_Lectura.contadorLectura++;
    contadorEscritura_Lectura.contadorEscritura++;
  }

  delete[] buffer;
  fclose(archivo);
}

void fusionarArchivosOrdenados(char **nombresArchivos, int k, int64_t B,
                               const char *archivoSalida) {
  FILE *salida = fopen(archivoSalida, "wb");
  if (!salida) {
    perror("Error abriendo archivo de salida");
    exit(1);
  }

  int64_t enterosPorBloque = B / sizeof(int64_t);
  FILE **archivos = new FILE *[k];
  int64_t **buffers = new int64_t *[k];
  int *indices = new int[k];
  int *tamaños = new int[k];
  bool *activo = new bool[k];

  for (int i = 0; i < k; ++i) {
    archivos[i] = fopen(nombresArchivos[i], "rb");
    buffers[i] = new int64_t[enterosPorBloque];
    tamaños[i] =
        fread(buffers[i], sizeof(int64_t), enterosPorBloque, archivos[i]);
    contadorEscritura_Lectura.contadorLectura++;
    indices[i] = 0;
    activo[i] = tamaños[i] > 0;
  }

  int64_t *bufferSalida = new int64_t[enterosPorBloque];
  int cantidadSalida = 0;

  while (true) {
    int64_t minimo = INT64_MAX;
    int origen = -1;
    for (int i = 0; i < k; ++i) {
      if (activo[i] && indices[i] < tamaños[i] &&
          buffers[i][indices[i]] < minimo) {
        minimo = buffers[i][indices[i]];
        origen = i;
      }
    }
    if (origen == -1)
      break;

    bufferSalida[cantidadSalida++] = minimo;
    indices[origen]++;

    if (indices[origen] >= tamaños[origen]) {
      tamaños[origen] = fread(buffers[origen], sizeof(int64_t),
                              enterosPorBloque, archivos[origen]);
      contadorEscritura_Lectura.contadorLectura++;
      indices[origen] = 0;
      activo[origen] = tamaños[origen] > 0;
    }

    if (cantidadSalida == enterosPorBloque) {
      fwrite(bufferSalida, sizeof(int64_t), cantidadSalida, salida);
      contadorEscritura_Lectura.contadorEscritura++;
      cantidadSalida = 0;
    }
  }

  if (cantidadSalida > 0) {
    fwrite(bufferSalida, sizeof(int64_t), cantidadSalida, salida);
    contadorEscritura_Lectura.contadorEscritura++;
  }

  for (int i = 0; i < k; ++i) {
    delete[] buffers[i];
    fclose(archivos[i]);
  }

  delete[] archivos;
  delete[] buffers;
  delete[] indices;
  delete[] tamaños;
  delete[] activo;
  delete[] bufferSalida;
  fclose(salida);
}

void mergesortExterno(int64_t N, int64_t B, int64_t M, int aridad) {
  int totalSubarchivos;
  crearSubarchivosOrdenados(N, B, M, totalSubarchivos);

  char **nombresSubarchivos = new char *[totalSubarchivos];
  for (int i = 0; i < totalSubarchivos; ++i) {
    nombresSubarchivos[i] = new char[32];
    sprintf(nombresSubarchivos[i], "subarchivoOrdenado%d.bin", i);
  }

  int etapa = 0;
  while (totalSubarchivos > 1) {
    int nuevosSubarchivosCount = (totalSubarchivos + aridad - 1) / aridad;
    char **nuevosNombresSubarchivos = new char *[nuevosSubarchivosCount];

    for (int i = 0; i < nuevosSubarchivosCount; ++i) {
      int inicio = i * aridad;
      int fin = (inicio + aridad <= totalSubarchivos) ? (inicio + aridad)
                                                      : totalSubarchivos;
      int k = fin - inicio;

      nuevosNombresSubarchivos[i] = new char[32];
      sprintf(nuevosNombresSubarchivos[i], "merge_etapa%d_%d.bin", etapa, i);

      fusionarArchivosOrdenados(&nombresSubarchivos[inicio], k, B,
                                nuevosNombresSubarchivos[i]);
    }

    for (int i = 0; i < totalSubarchivos; ++i)
      delete[] nombresSubarchivos[i];
    delete[] nombresSubarchivos;

    nombresSubarchivos = nuevosNombresSubarchivos;
    totalSubarchivos = nuevosSubarchivosCount;
    etapa++;
  }

  rename(nombresSubarchivos[0], "salida.bin");
  delete[] nombresSubarchivos[0];
  delete[] nombresSubarchivos;
}

int evaluarAridad(int64_t *arreglo, int64_t N, int64_t B, int64_t M,
                  int aridad) {
  guardarEnDisco(arreglo, N, B);
  contadorEscritura_Lectura = {0, 0};
  mergesortExterno(N, B, M, aridad);
  return contadorEscritura_Lectura.contadorLectura +
         contadorEscritura_Lectura.contadorEscritura;
}

int encontrarMejorAridad(int64_t *arreglo, int64_t N, int64_t B, int64_t M) {
  int b = B / sizeof(int64_t);
  int low = 2, high = b;

  while (high - low > 3) {
    int m1 = low + (high - low) / 3;
    int m2 = high - (high - low) / 3;

    int costo1 = evaluarAridad(arreglo, N, B, M, m1);
    int costo2 = evaluarAridad(arreglo, N, B, M, m2);

    if (costo1 < costo2) {
      high = m2;
    } else {
      low = m1;
    }
  }

  int mejorA = low;
  int minCosto = evaluarAridad(arreglo, N, B, M, mejorA);
  for (int a = low + 1; a <= high; ++a) {
    int costo = evaluarAridad(arreglo, N, B, M, a);
    if (costo < minCosto) {
      minCosto = costo;
      mejorA = a;
    }
  }
  return mejorA;
}

void quicksort_in_memory(int64_t* arr, int64_t n) {
    if (n <= 1) return;
    int64_t pivot = arr[n / 2];
    int64_t i = 0, j = n - 1;
    while (i <= j) {
        while (arr[i] < pivot) ++i;
        while (arr[j] > pivot) --j;
        if (i <= j) {
            std::swap(arr[i], arr[j]);
            ++i;
            if (j > 0) --j;
        }
    }
    if (j > 0) quicksort_in_memory(arr, j + 1);
    quicksort_in_memory(arr + i, n - i);
}


void quicksort_local(int64_t* arr, int64_t left, int64_t right) {
    if (left >= right) return;
    int64_t pivot = arr[right];
    int64_t i = left - 1;
    for (int64_t j = left; j < right; ++j) {
        if (arr[j] < pivot) {
            ++i;
            std::swap(arr[i], arr[j]);
        }
    }
    std::swap(arr[i + 1], arr[right]);
    quicksort_local(arr, left, i);
    quicksort_local(arr, i + 2, right);
}


int64_t minimo(int64_t a, int64_t b) {
    return (a < b) ? a : b;
}


/// @brief 
/// @param inicio posición del primer entero del archivo que vamos a ordenar
/// @param fin posición del último entero 
/// @param M cantidad máxima de enteros que caben en la memoria
/// @param B tamaño bloque
/// @param a tamaño particiones
void quicksortExterno(int64_t inicio, int64_t fin, int64_t M, int64_t B, int64_t a) {
    int64_t N = fin - inicio; //Número total de elementos a ordenar
    //Caso base 1: no hay elementos que ordenar
    if (N <= 0) return;

    //Caso base 2: Si el número de elementos cabe en memoria, ordenamos directamente
    if (N <= M) {
        //reservamos la memoria 
        int64_t* arreglo = new int64_t[N];
        leerDesdeDisco(arreglo, N, B, inicio);
        quicksort_in_memory(arreglo, N);
        guardarEnDisco(arreglo, N, B, inicio);

        //liberamos memoria
        delete[] arreglo;
        return;
    }

    //Calculamos cuántos enteros caben en un bloque
    //y cuántos enteros hay en el rango [inicio, fin]
    int64_t elementosPorBloque = B / sizeof(int64_t); 
    int64_t totalBloques = N / elementosPorBloque;        //calculamos los enteros por bloque de disco que caben 
    if (N % elementosPorBloque != 0) totalBloques++;  //que sean exactamente todos los elementos

    //seteamos nuestro random generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int64_t> distBloque(0, totalBloques - 1);

    //same as before, elegimos un bloque al azar
    int64_t bloqueAleatorio = distBloque(gen);
    int64_t* buffer = new int64_t[elementosPorBloque];
    //leemos ese bloque en el disco
    leerDesdeDisco(buffer, elementosPorBloque, B, inicio + bloqueAleatorio * elementosPorBloque);

    //tomamos los (a-1) elementos aleatorios que están dentro del bloque
    //y los usamos como pivotes
    int64_t* pivotes = new int64_t[a - 1];
    std::uniform_int_distribution<int64_t> distIndice(0, elementosPorBloque - 1);
    for (int64_t i = 0; i < a - 1; ++i)
        pivotes[i] = buffer[distIndice(gen)];
    //liberamos el buffer original del bloque
    delete[] buffer;

    //ordenamos los pivotes que habíamos elegido
    //para usarlos para particionar
    quicksort_local(pivotes, 0, a - 2);

    //Crear arreglos que nos servirán para almacenar
    // las particiones en memoria 
    int64_t** particiones = new int64_t*[a]; //aquí guardaremos los enteros de la partición => cada partición será un arreglo de enteros
    int64_t* tamPart = new int64_t[a];        // guardamos el tamaño de la partición
    int64_t* capPart = new int64_t[a];        //guardamos la capacidad de las particiones

    //vamos a la i-ésima partición y inicializamos con capacidad base
    for (int64_t i = 0; i < a; ++i) {
        capPart[i] = N / a + 10; //haremos que la capacidad de las particiones sea un poco más grande
        //=> como dividimos cada partición en N/a elementos a veces algunos tendrán un poco más o menos
        particiones[i] = new int64_t[capPart[i]];
        tamPart[i] = 0; //vacío al principio
    }

    //Leemos el archivo en bloques
    buffer = new int64_t[elementosPorBloque];
    for (int64_t i = inicio; i < fin; i += elementosPorBloque) {
        //cantidad de elementos por leer
        int64_t cantidad = minimo(elementosPorBloque, fin - i);
        leerDesdeDisco(buffer, cantidad, B, i);
        //Ahora calculamos a qué partición tenemos que dirigirnos usando los pivotes
        //=> aquí ubicamos cada elemento en su partición 
        for (int64_t j = 0; j < cantidad; ++j) {
            int64_t valor = buffer[j];

            //comparamos los pivotes para encontrarla
            int64_t k = 0;
            while (k < a - 1 && valor >= pivotes[k]) ++k;

            //si no hay espacio en la partición, la tenemos que duplicar
            if (tamPart[k] >= capPart[k]) {
                //Aumentar capacidad si se llena
                int64_t nuevaCap = capPart[k] * 2;
                int64_t* nuevo = new int64_t[nuevaCap];
                //traemos los datos que teníamos al nuevo arreglo que tiene mayor tamaño
                for (int64_t m = 0; m < capPart[k]; ++m)
                    nuevo[m] = particiones[k][m]; 
                //liberamos el arreglo anterior y actualizamos los punteros
                delete[] particiones[k];
                particiones[k] = nuevo;
                capPart[k] = nuevaCap;
            }
            //asignamos el valor a la partición que resulte de la comparación
            particiones[k][tamPart[k]++] = valor;
        }
    }
    //liberamos memoria again
    delete[] buffer;
    delete[] pivotes;

    //---------------------------------------------------------------------------------------------------------------------------------
    // Guardamos las particiones al archivo

    //posición actual
    int64_t actual = inicio;
    int64_t* inicioPart = new int64_t[a]; //inicio partición en disco
    int64_t* finPart = new int64_t[a];    //fin partición en disco

    for (int64_t k = 0; k < a; ++k) { 
        inicioPart[k] = actual; //inicio en posición actual

        if (tamPart[k] == 0) {
            //partición vacía
            finPart[k] = actual;
            continue;
        }
        //Escribimos cada partición de vuelta al archivo
        guardarEnDisco(particiones[k], tamPart[k], B, actual);
        //Actualizamos las posiciones
        //tomamos el nuevo rango de la partición
        finPart[k] = actual + tamPart[k];
        actual += tamPart[k];
        //liberamos memoria
        delete[] particiones[k];
    }

    delete[] particiones;
    delete[] tamPart;
    delete[] capPart;

    //Llamamos recursivamente a quicksort para cada partición creada
    for (int64_t k = 0; k < a; ++k)
        quicksortExterno(inicioPart[k], finPart[k], M, B, a);

    delete[] inicioPart;
    delete[] finPart;
}


void MergeSort() {
  const int64_t M = 50 * 1024 * 1024; // Memoria principal (50MB)
  const int64_t B = 4096;             // Tamaño del bloque (4KB)
  const int64_t N = 64; // Número total de elementos pequeños para prueba rápida

  // Re-generar los datos para asegurar condiciones iniciales iguales
  int64_t arreglo[N];
  std::mt19937_64 rng(std::random_device{}());
  for (int64_t i = 0; i < N; ++i)
    arreglo[i] = i;
  std::shuffle(arreglo, arreglo + N, rng);

  // Determinar la mejor aridad
  int mejorAridad = encontrarMejorAridad(arreglo, N, B, M);

  // imprimir mejor aridad usada
  // printear la mejor aridad 
  std::cout << "Mejor aridad encontrada: " << mejorAridad << std::endl;



  // Re-escribir los datos
  guardarEnDisco(arreglo, N, B);

  // Ejecutar mergesort externo
  mergesortExterno(N, B, M, mejorAridad);
}

void QuickSort() {
    const int64_t M = 50 * 1024 * 1024; // 50MB
    const int64_t B = 4096;             // 4KB
    const int64_t N = 64;               // número total de elementos

    // Re-generar datos
    int64_t arreglo[N];
    std::mt19937_64 rng(std::random_device{}());
    for (int64_t i = 0; i < N; ++i) arreglo[i] = i;
    std::shuffle(arreglo, arreglo + N, rng);

    // Guardar el arreglo desordenado en disco
    guardarEnDisco(arreglo, N, B);

    // Definir número de particiones (igual que mejor aridad en mergesort)
    int64_t elementosPorBloque = B / sizeof(int64_t);
    int64_t a = elementosPorBloque; // máximo pivotes que caben en un bloque

    quicksortExterno(0, N, M / sizeof(int64_t), B, a);
}



// implementaciones de ordenamiento que tendremos 
void MergeSort();
void QuickSort();

int main() {
    const int MB = 1024 * 1024;
    const int M = 50 * MB;
    const int B = 4096;
    const int64_t tamañoEntero = sizeof(int64_t);

    // Crear encabezado para resultados.csv
    std::ofstream resultados("resultados.csv");
    resultados << "Tamaño_MB,Secuencia,Algoritmo,Tiempo_segundos,Accesos_disco\n";
    resultados.close();

    // Crear encabezado para promedios.csv
    std::ofstream encabezado("promedios.csv");
    encabezado << "N,Algoritmo,Promedio_Tiempo,Promedio_Accesos\n";
    encabezado.close();

    std::mt19937_64 rng(std::random_device{}());

    int64_t* arregloQuickTiempo = new int64_t[5];
    int64_t* arregloAccesoDiscoQuick = new int64_t[5];
    int64_t* arregloAccesoDiscoMerge = new int64_t[5];
    int64_t* arregloMergeTiempo = new int64_t[5];

    for (int mult = 4; mult <= 60; mult += 4) {
        const int64_t N = (int64_t(mult) * M) / tamañoEntero;

        for (int secuencia = 1; secuencia <= 5; ++secuencia) {
            std::cout << "\n-------------------------------\n";
            std::cout << ">> Tamaño: " << mult << "MB | Secuencia #" << secuencia << "\n";

            // Crear y desordenar arreglo
            int64_t* arreglo = new int64_t[N];
            for (int64_t i = 0; i < N; ++i)
                arreglo[i] = i;
            std::shuffle(arreglo, arreglo + N, rng);

            // Guardar en disco
            auto t1 = std::chrono::high_resolution_clock::now();
            guardarEnDisco(arreglo, N, B);
            auto t2 = std::chrono::high_resolution_clock::now();
            std::cout << "Escritura a disco: " << std::chrono::duration<double>(t2 - t1).count() << " s\n";
            delete[] arreglo;

            // MERGE SORT EXTERNO
            contadorEscritura_Lectura.contadorLectura = 0;
            contadorEscritura_Lectura.contadorEscritura = 0;
            auto t3 = std::chrono::high_resolution_clock::now();
            MergeSort();
            auto t4 = std::chrono::high_resolution_clock::now();
            double tiempoMerge = std::chrono::duration<double>(t4 - t3).count();
            int accesosMerge = contadorEscritura_Lectura.contadorLectura;

            arregloMergeTiempo[secuencia - 1] = tiempoMerge;
            arregloAccesoDiscoMerge[secuencia - 1] = accesosMerge;

            {
                std::ofstream resultados("resultados.csv", std::ios::app);
                resultados << mult << "," << secuencia << ",Merge," << tiempoMerge << "," << accesosMerge << "\n";
                resultados.flush();
            }

            // QUICK SORT EXTERNO
            contadorEscritura_Lectura.contadorLectura = 0;
            contadorEscritura_Lectura.contadorEscritura = 0;
            auto t5 = std::chrono::high_resolution_clock::now();
            QuickSort();
            auto t6 = std::chrono::high_resolution_clock::now();
            double tiempoQuick = std::chrono::duration<double>(t6 - t5).count();
            int accesosQuick = contadorEscritura_Lectura.contadorLectura;

            arregloQuickTiempo[secuencia - 1] = tiempoQuick;
            arregloAccesoDiscoQuick[secuencia - 1] = accesosQuick;

            {
                std::ofstream resultados("resultados.csv", std::ios::app);
                resultados << mult << "," << secuencia << ",Quick," << tiempoQuick << "," << accesosQuick << "\n";
                resultados.flush();
            }

            std::cout << "Merge y Quick completados para " << mult << "MB - secuencia " << secuencia << "\n";
        }

        // Calcular y guardar promedios en tiempo real
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

        std::ofstream resumen("promedios.csv", std::ios::app);
        resumen << N << ",Merge," << promedioTiempoMerge << "," << promedioAccesosMerge << "\n";
        resumen << N << ",Quick," << promedioTiempoQuick << "," << promedioAccesosQuick << "\n";
        resumen.flush();
        resumen.close();

        std::cout << "[+] Promedios guardados para " << mult << "MB\n";
    }

    delete[] arregloQuickTiempo;
    delete[] arregloAccesoDiscoQuick;
    delete[] arregloAccesoDiscoMerge;
    delete[] arregloMergeTiempo;

    std::cout << "\nExperimentos completados. Resultados en 'resultados.csv' y 'promedios.csv'\n";
    return 0;
}
