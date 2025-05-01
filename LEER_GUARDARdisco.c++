#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>

const char* FILENAME = "datos.bin";

struct ContadorEscritura_Lectura{
    int contadorEscritura = 0;
    int contadorLectura = 0;
}
contadorEscritura_Lectura;

// valores de i = 0, y i < N
void guardarEnDisco(int* arreglo, int N, int B, int i = 0) {
    // modo escribir binario
    FILE* archivo = fopen(FILENAME, "wb");
    if (!archivo) {
        perror("Error abriendo archivo");
        exit(1);
    }
    // si escogemos un indice fuera de rango 
    if (i < 0 || i >= N) {
        fprintf(stderr, "Error: el índice inicial i=%d está fuera de rango (0 <= i < %d)\n", i, N);
        fclose(archivo);
        return;
    }

    int tamañoEntero = sizeof(int);
    // tamaño del bloque en bytes
    int elementosPorBloque = B / sizeof(int);

    // buffer aux, para poder poder escribir bloque por bloque, es  = B pues ints * sizeint = B
    int* buffer = new int[elementosPorBloque];

    while (i < N) {
        // si cabe todo el bloque
        int elementosPorCopiar = 0;
        if ((i + elementosPorBloque <= N)) {
            elementosPorCopiar = elementosPorBloque;
        }
        // los restantes (N-i))
        else {
            elementosPorCopiar = N - i;
        }

        // memcy = mem = memory cpy = copiar en memoria el buffer(bloque) usando el arreglo 
        // void* memcpy(void* dest, const void* src, size_t n);
        // destination: puntero donde queremos guardar
        // source: puntero a la memo donde queremos guardar
        // num: The number of bytes to copy. elementos * size = bytes
        memcpy(buffer, &arreglo[i], elementosPorCopiar * sizeof(int));

        // verifica que no se llenó
        if (elementosPorCopiar < elementosPorBloque) {
            // desde el indice que queda por copiar
            memset(&buffer[elementosPorCopiar], 0,
                   (elementosPorBloque - elementosPorCopiar) * sizeof(int));
        }

        // fseek = mover el puntero del archivo
        // SEEK_SET: desde el inicio del archivo
        // i * sizeof(int): posición en bytes donde escribir
        fseek(archivo, i * sizeof(int), SEEK_SET);

        // fwrite = write file = escribir en el archivo
        // void fwrite(const void* ptr, size_t size, size_t count, FILE* stream);
        fwrite(buffer, sizeof(int), elementosPorBloque, archivo);

        // contador de escritura, la estructura definida anteriormente
        contadorEscritura_Lectura.contadorEscritura++;

        // saltos de elementosPorBloque
        i += elementosPorBloque;
    }

    delete[] buffer;
    fclose(archivo);
}


void leerDesdeDisco(int* arreglo, int N, int B, int i =0) {
    // modo leer binario
    FILE* archivo = fopen(FILENAME, "rb");
    if (!archivo) {
        perror("Error abriendo archivo");
        exit(1);
    }

    // si escogemos un indice fuera de rango 
    if (i < 0 || i >= N) {
        fprintf(stderr, "Error: el índice inicial i=%d está fuera de rango (0 <= i < %d)\n", i, N);
        fclose(archivo);
        return;
    }

    // tamaño del bloque en bytes
    int elementosPorBloque = B / sizeof(int);
    // buffer aux, para poder poder escribir bloque por bloque, es  = B pues ints * sizeint = B
    int* buffer = new int[elementosPorBloque];

    for (i; i < N; i += elementosPorBloque) {
        int offsetBytes = i * sizeof(int);

        fseek(archivo, offsetBytes, SEEK_SET);

        // size_t fwrite(const void* ptr, size_t size, size_t count, FILE* stream);

        int cantidad = (i + elementosPorBloque <= N) ? elementosPorBloque : N - i;

        fread(buffer, sizeof(int), elementosPorBloque, archivo);

        memcpy(&arreglo[i], buffer, cantidad * sizeof(int));

        // contador de lectura, la estructura definida anteriormente
        contadorEscritura_Lectura.contadorLectura++;
    }

    delete[] buffer;
    fclose(archivo);
}



