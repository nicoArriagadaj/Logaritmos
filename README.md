# DISEÑO Y ANALISIS DE ALGORITMOS

### para correr el codigo normalmente, debes
1) compilar g++ main.c++ -o main

2) ejecutar ./main


### Ejecución dentro de Docker (límite de 50 MB de RAM), debes:

1) Descargar la imagen -> docker pull pabloskewes/cc4102-cpp-env


2) Arrancar el contenedor y ejecutar 
docker run --rm -it -m 50m -v ${PWD}:/workspace -w /workspace pabloskewes/cc4102-cpp-env bash -c "g++ main.c++ -o main && ./main"

Limitando a 50MB 

recuerde usar otros nombres o borrar los archivos, pues, si existen nombres iguales, no compilará


### Links a csv con información:

- [promedios.csv](promedios.csv)
- [resultados.csv](resultados.csv)

#### Saludos Cordiales
![Gato](images/gatopro.webp)

Ante cualquier duda, contactar en ucursos.
Saludos Cordiales.
