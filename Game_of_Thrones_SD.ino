/*
  Camilo Perafan Montoya
  Carne 17092
  Musica del Juego
*/

//Se incluyen las librerias de SPI y SD
#include <SPI.h>
#include <SD.h>

//Se definen las variables a utilizar
File myFile1;
File myFile2;
int UltimaPosicion1 = 0;
int UltimaPosicion2 = 0;
int buzzer = 40;

const int chipSelect = 4;

//Se define una funcion que reproduce en el buzzer una nota con una duracion determinada
void beep(int note, int duration)
{
  tone(buzzer, note, duration * 0.9);
  delay(duration);
  noTone(buzzer);
}

//Se realizan las configuraciones iniciales
void setup() {

  // Configuración de puerto serial y módulo SPI
  Serial.begin(9600);
  SPI.setModule(0);

  Serial.print("Inicializando Memoria SD...");

  //Activación del SS
  pinMode(10, OUTPUT);

  //Inicialización de la memoria SD
  if (!SD.begin(32)) {
    Serial.println("Inicialización Fallida");
    return;
  }
  Serial.println("Inicialización Terminada");

  pinMode(buzzer, OUTPUT);

}

//Funcion que lee el valor de las notas y su duracion de la memoria SD y reproduce la cancion correspondiente
void OST(void) {

  //Se abre el archivo que contiene las notas de la cancion
  myFile1 = SD.open("Notas.txt");

  //Se define el size del archivo
  int totalBytes1 = myFile1.size();

  //Se define la variable interna donde se guarda el valor de las notas
  String cadena1 = "";

  if (myFile1) {

    //Se va a la siguiente posicion del archivo despues de guardar una nota
    if (UltimaPosicion1 >= totalBytes1)  UltimaPosicion1 = 0;
    myFile1.seek(UltimaPosicion1);

    //Leer el archivo escogido hasta terminar
    while (myFile1.available()) {

      //Se guarda el valor de la nota en las variables mostradas
      char caracter1 = myFile1.read();
      cadena1 = cadena1 + caracter1;

      //Se determina la posicion actual en el archivo
      UltimaPosicion1 = myFile1.position();

      //Si se da una nueva linea en el archivo salir de la rutina
      if (caracter1 == 10) {
        break;
      }
    }
    // Cerrar el archivo:
    myFile1.close();

    //Se abre el archivo que contiene la duracion de las notas
    myFile2 = SD.open("Duracion.txt");

    //Se define el size del archivo
    int totalBytes2 = myFile2.size();

    //Se define la variable donde se guardaran el valor de la duracion de las notas
    String cadena2 = "";

    if (myFile2) {

      //Se va a la siguiente posicion del archivo despues de guardar una nota
      if (UltimaPosicion2 >= totalBytes2)  UltimaPosicion2 = 0;
      myFile2.seek(UltimaPosicion2);

      //Leer el archivo escogido hasta terminar
      while (myFile2.available()) {

        //Se guarda el valor de la nota en las variables mostradas
        char caracter2 = myFile2.read();
        cadena2 = cadena2 + caracter2;

        //Se obtiene la posicion actual en el archivo
        UltimaPosicion2 = myFile2.position();

        //Si se da una nueva linea en el archivo salir de la rutina
        if (caracter2 == 10) {
          break;
        }
      }
      // Cerrar el archivo:
      myFile2.close();
    }

    //Conversion de las notas y duraciones de string a int
    int notas = cadena1.toInt();
    int duracion = cadena2.toInt();

    //Se reproduce la cancion
    beep(notas, duracion);

  } else {
    //Si el archivo no se puede abrir, imprimir este mensaje
    Serial.println("error opening test.txt");
  }

}

void loop() {
  OST();
}
