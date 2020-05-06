//***************************************************************************************************************************************
/*
   Proyecto 2
   Electronica Digital 1
   Camilo Perafan Montoya - 17092
   Hans Burmester - 17022
   Seccion 12
*/
//***************************************************************************************************************************************

//Se incluyen las librerias de SPI y SD
#include <SPI.h>
#include <SD.h>

#define Musica_Juego PA_7
#define Musica_Perdio PA_6

//***************************************************************************************************************************************
// Functions Prototypes
//***************************************************************************************************************************************
void beep(int note, int duration);
void OST_Juego(void);
void OST_Perdio(void);

//***************************************************************************************************************************************
// Variables globales
//***************************************************************************************************************************************
File myFile1;
File myFile2;
uint16_t UltimaPosicion1 = 0;
uint16_t UltimaPosicion2 = 0;
uint16_t buzzer = 40;
const uint16_t chipSelect = 4;

//Se realizan las configuraciones iniciales
void setup() {

  // Configuración de puerto serial y módulo SPI
  Serial.begin(9600);
  SPI.setModule(0);

  pinMode(Musica_Juego, INPUT);
  pinMode(Musica_Perdio, INPUT);

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

void loop() {

  //Mientras esté activada la entrada que habilita la música del juego, reproduzca la melodia
  //del juego
  while (digitalRead(Musica_Juego)) {
    OST_Juego();
    
    //Si se activa la entrada que habilita la musica cuando el jugador pierde, reproduzca la melodia
    // y reinicie todas las variables de la memoria SD
    if (digitalRead(Musica_Perdio)) {
      OST_Perdio();
      myFile1.close();
      myFile2.close();
      UltimaPosicion1 = 0;
      UltimaPosicion2 = 0;
      break;
    }
  }
}

//Se define una funcion que reproduce en el buzzer una nota con una duracion determinada
void beep(int note, int duration)
{
  tone(buzzer, note, duration * 0.9);
  delay(duration);
  noTone(buzzer);
}

//Funcion que lee el valor de las notas y su duracion de la memoria SD y reproduce la cancion correspondiente
void OST_Juego(void) {

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

//Función que reproduce la melodia cuando el jugador pierde
void OST_Perdio(void) {
  beep(294, 562);
  beep(392, 187);
  beep(523, 1125);
  beep(494, 375);
  beep(392, 281);
  beep(330, 281);
  beep(440, 281);
  beep(587, 1500);
}
