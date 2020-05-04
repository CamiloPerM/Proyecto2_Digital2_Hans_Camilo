//***************************************************************************************************************************************
/* 
   Proyecto 2
   Electronica Digital 1
   Camilo Perafan Montoya - 17092
   Hans Burmester - 17022
   Seccion 12 
    
   Librería para el uso de la pantalla ILI9341 en modo 8 bits
   Basado en el código de martinayotte - https://www.stm32duino.com/viewtopic.php?t=637
   Adaptación, migración y creación de nuevas funciones: Pablo Mazariegos y José Morales
   Con ayuda de: José Guerra
   IE3027: Electrónica Digital 2 - 2019
*/
//***************************************************************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"

#define LCD_RST PD_0
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1
int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};

#define SALTAR_J2 PF_4
#define AGACHARSE_J2 PF_0

#define SALTAR_J1 PA_7
#define AGACHARSE_J1 PA_6

//***************************************************************************************************************************************
// Functions Prototypes
//***************************************************************************************************************************************
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);

void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset);

void BalaDer(uint8_t modulo, uint8_t y);
void BalaIzq(uint8_t modulo, uint8_t y);
void Plataforma1Der(uint8_t modulo, uint8_t y);
void Plataforma1Izq(uint8_t modulo, uint8_t y);
void Plataforma2Der(uint8_t modulo, uint8_t y);
void Plataforma2Izq(uint8_t modulo, uint8_t y);
void Plataforma3Der(uint8_t modulo, uint8_t y);
void Plataforma3Izq(uint8_t modulo, uint8_t y);

void Init(void);
void GameOver1(void);
void GameOver2(void);

//***************************************************************************************************************************************
// Variables globales
//***************************************************************************************************************************************
uint8_t AparicionBloque1Der = 0;           //Bandera bloque 1 hacia la derecha
uint8_t AparicionBloque2Der = 0;           //Bandera bloque 2 hacia la derecha
uint8_t AparicionBloque3Der = 0;           //Bandera bloque 3 hacia la derecha


uint8_t AparicionBloque1Izq = 0;           //Bandera bloque 1 hacia la izquierda
uint8_t AparicionBloque2Izq = 0;           //Bandera bloque 2 hacia la izquierda
uint8_t AparicionBloque3Izq = 0;           //Bandera bloque 2 hacia la izquierda


uint8_t AparicionBalaDer = 0;             //Bandera bala hacia la derecha
uint8_t AparicionBalaIzq = 0;             //Bandera bala hacia la izquierda

uint8_t x1 = 0;                           //Variable para el movimiento para la derecha bloques
uint16_t x2 = 297;                        //Variable para el movimiento para la izquireda bloques
uint8_t x1_bala = 0;                      //Variable para el movimiento para la derecha bala
uint16_t x2_bala = 297;                    //Variable para el movimiento para la izquireda bala

uint8_t CONTADOR = 0;                     //Contador para los sprites

uint8_t cont = 0;                         //contador para las rutinas del J1
uint8_t cont2 = 0;                        //Contador para las rutinas del J2

uint8_t flag1 = 0; //random(0,2);              //Bandera para las rutinas del J1
uint8_t flag2 = 4; //random(0,2);            //Bandera para las rutinas del J2

uint8_t Saltar2 = 1;                       //Bandera para determinar rutina de salto J2
uint8_t Boton2 = 1;                        //Bandera par que solo se ejcute una vez el salto

uint8_t posy_J2 = 180;                    //Posicion original en y del personaje 2
uint8_t posy_secJ2 = 180;                 //Posicion en y del personaje 2 que se usará para aumentarla
uint8_t posy_AgJ2 = posy_J2 + 8;          //Posicion en y para el personaje 2 agachado

uint8_t Saltar1 = 1;                       //Bandera para determinar rutina de salto J1
uint8_t Boton1 = 1;                        //Bandera par que solo se ejcute una vez el salto

uint8_t posy_J1 = 180;                    //Posicion original en y del personaje 1
uint8_t posy_secJ1 = 180;                 //Posicion en y del personaje 1 que se usará para aumentarla
uint8_t posy_AgJ1 = posy_J1 + 8;          //Posicion en y para el personaje 1 agachado

bool Estado = false;                      //Bandera para definir es el estado del juego

uint8_t pres = 0;                         //Bandera para antirebote boton 1
uint8_t pres2 = 0;                        //Bandera para antirebote boton 2

uint8_t perd_J1 = 0;                      //Bandera para definir si perdio el Jugador 1
uint8_t perd_J2 = 0;                      //Bandera para definri si perdio el Jugador 2

//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************

void setup() {
  SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
  Serial.begin(9600);
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  LCD_Init();

  //Definicion de entradas y salidas
  pinMode(SALTAR_J2, INPUT_PULLUP);
  pinMode(AGACHARSE_J2, INPUT_PULLUP);
  pinMode(SALTAR_J1, INPUT);
  pinMode(AGACHARSE_J1, INPUT);

  //Pantalla Incial al Encender el Juego
  LCD_Clear(0x14F4);
  LCD_Bitmap(140, 110, 44, 26, Start);
  LCD_Print("Mario Jump", 80, 50, 2, 0x1186, 0x14F4);
  LCD_Print("Presione Saltar Para Comenzar", 50, 90, 1, 0x1186, 0x14F4);
  LCD_Print("Hans Burmester - 17022", 73, 150, 1, 0x1186, 0x14F4);
  LCD_Print("Camilo Perafan M. - 17092", 63, 180, 1, 0x1186, 0x14F4);

  //Hasta que se presione el boton de saltar de cualquier jugador, no se inicia el juego
  while (!Estado) {
    if (!digitalRead(SALTAR_J1)) {
      pres = 1;
      delay(3);
      if (digitalRead(SALTAR_J1) && pres == 1) {
        pres = 0;
        Estado = true;
        Init();
      }
    }

    if (!digitalRead(SALTAR_J2)) {
      pres2 = 1;
      delay(3);
      if (digitalRead(SALTAR_J2) && pres2 == 1) {
        pres2 = 0;
        Estado = true;
        Init();
      }
    }
  }
}
//***************************************************************************************************************************************
// Loop Infinito
//***************************************************************************************************************************************
void loop() {

//***************************************************************************************************************************************
//Estado de Juego Perdido
//***************************************************************************************************************************************  
  if (Estado == false) {
    
    //Si perdio el Jugador 1, se muestra una cuenta regresiva de 5 segundos para volver a comenzar
    //el juego, reiniciando todas las variables al terminar la cuenta. 
    if (perd_J1 == 1) {
      LCD_Clear(0x14F4);
      LCD_Print("Perdio J1", 90, 80, 2, 0x0000, 0x14F4);
      LCD_Bitmap(110, 40, 100, 29, Fin);
      LCD_Print("El Juego", 95, 120, 2, 0x0000, 0x14F4);
      LCD_Print("Se Reiniciara...", 45, 140, 2, 0x0000, 0x14F4);
      LCD_Print("5", 150, 160, 2, 0x0000, 0x14F4);
      delay(1000);
      LCD_Print("4", 150, 160, 2, 0x0000, 0x14F4);
      delay(1000);
      LCD_Print("3", 150, 160, 2, 0x0000, 0x14F4);
      delay(1000);
      LCD_Print("2", 150, 160, 2, 0x0000, 0x14F4);
      delay(1000);
      LCD_Print("1", 150, 160, 2, 0x0000, 0x14F4);
      delay(1000);
      posy_J1 = 180;
      posy_secJ1 = 180;
      posy_AgJ1 = posy_J1 + 8;
      posy_J2 = 180;
      posy_secJ2 = 180;
      posy_AgJ2 = posy_J1 + 8;
      CONTADOR = 0;
      cont = 0;
      x1 = 0;                           
      x2 = 297;                        
      x1_bala = 0;                      
      x2_bala = 297;
      flag1 = random(0, 2);
      flag2 = random(0,2);
      perd_J1 = 0;
      Init();
      Estado = true;
    }

    //Si perdio el Jugador 2, se muestra una cuenta regresiva de 5 segundos para volver a comenzar
    //el juego, reiniciando todas las variables al terminar la cuenta.
    if (perd_J2 == 1) {
      LCD_Clear(0x14F4);
      LCD_Print("Perdio J2", 90, 80, 2, 0x0000, 0x14F4);
      LCD_Bitmap(110, 40, 100, 29, Fin);
      LCD_Print("El Juego", 95, 120, 2, 0x0000, 0x14F4);
      LCD_Print("Se Reiniciara...", 45, 140, 2, 0x0000, 0x14F4);
      LCD_Print("5", 150, 160, 2, 0x0000, 0x14F4);
      delay(1000);
      LCD_Print("4", 150, 160, 2, 0x0000, 0x14F4);
      delay(1000);
      LCD_Print("3", 150, 160, 2, 0x0000, 0x14F4);
      delay(1000);
      LCD_Print("2", 150, 160, 2, 0x0000, 0x14F4);
      delay(1000);
      LCD_Print("1", 150, 160, 2, 0x0000, 0x14F4);
      delay(1000);
      posy_J1 = 180;
      posy_secJ1 = 180;
      posy_AgJ1 = posy_J1 + 8;
      posy_J2 = 180;
      posy_secJ2 = 180;
      posy_AgJ2 = posy_J1 + 8;
      CONTADOR = 0;
      cont = 0;
      x1 = 0;                           
      x2 = 297;                        
      x1_bala = 0;                      
      x2_bala = 297;
      flag1 = random(0, 2);
      flag2 = random(0,2);
      perd_J2 = 0;
      Init();
      Estado = true;
    }
  }

//***************************************************************************************************************************************
//Estado del Juego
//***************************************************************************************************************************************
  if (Estado == true) {
    CONTADOR++;
    LCD_Sprite(150, 186, 16, 21, planta, 4, (CONTADOR / 60) % 4, 0, 0);       //Sprite para el movimiento de la planta

//***************************************************************************************************************************************
//Comparaciones J1
//***************************************************************************************************************************************
    //Revisa si se esta realizando la rutina 1 del Jugador 1
    if (flag1 == 0) {
      //Comparacion para determinar si el jugador no pudo saltar y por ende perdio el juego
      //Si el bloque toco al jugador
      if (x1 > 43) {

        //Si la posicion en y del jugador es 180
        if (posy_J1 == 180) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (180 - posy_secJ1 < 12) {
            GameOver1();
          }
        }

        //Si la posicion en y del jugador es 154 y cont2 esta en 1
        if (posy_J1 == 154 && cont == 2) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (154 - posy_secJ1 < 12) {
            GameOver1();
          }
        }

        //Si la posicion en y del jugador es 128 y cont2 esta en 2
        if (posy_J1 == 128 && cont == 5) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (128 - posy_secJ1 < 12) {
            GameOver1();
          }
        }

        //Si la posicion en y del jugador es 102 y cont2 esta en 3
        if (posy_J1 == 102 && cont == 6) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (102 - posy_secJ1 < 12) {
            GameOver1();
          }
        }

        //Si la posicion en y del jugador es 76 y cont2 esta en 4
        if (posy_J1 == 76 && cont == 7) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (76 - posy_secJ1 < 12) {
            GameOver1();
          }
        }

        //Si la posicion en y del jugador es 50 y cont2 esta en 6
        if (posy_J1 == 50 && cont == 8) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (50 - posy_secJ1 < 12) {
            GameOver1();
          }
        }
      }
    }

    //Revisa si se esta realizando la rutina 2 del Jugador 2
    if (flag1 == 1) {
      //Comparacion para determinar si el jugador no pudo saltar y por ende perdio el juego
      //Si el bloque toco al jugador
      if (x1 > 43) {

        //Si la posicion en y del jugador es 180
        if (posy_J1 == 180 && cont == 1) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (180 - posy_secJ1 < 12) {
            GameOver1();
          }
        }

        //Si la posicion en y del jugador es 154 y cont2 esta en 1
        if (posy_J1 == 154 && cont == 2) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (154 - posy_secJ1 < 12) {
            GameOver1();
          }
        }

        //Si la posicion en y del jugador es 128 y cont2 esta en 2
        if (posy_J1 == 128 && cont == 4) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (128 - posy_secJ1 < 12) {
            GameOver1();
          }
        }

        //Si la posicion en y del jugador es 102 y cont2 esta en 3
        if (posy_J1 == 102 && cont == 5) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (102 - posy_secJ1 < 12) {
            GameOver1();
          }
        }

        //Si la posicion en y del jugador es 76 y cont2 esta en 4
        if (posy_J1 == 76 && cont == 6) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (76 - posy_secJ1 < 12) {
            GameOver1();
          }
        }

        //Si la posicion en y del jugador es 50 y cont2 esta en 6
        if (posy_J1 == 50 && cont == 7) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (50 - posy_secJ1 < 12) {
            GameOver1();
          }
        }
      }
    }

    //Revisa si se esta realizando la rutina 3 del Jugador 2
    if (flag1 == 2) {
      //Comparacion para determinar si el jugador no pudo saltar y por ende perdio el juego
      //Si el bloque toco al jugador
      if (x1 > 43) {

        //Si la posicion en y del jugador es 180
        if (posy_J1 == 180) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (180 - posy_secJ1 < 12) {
            GameOver1();
          }
        }

        //Si la posicion en y del jugador es 154 y cont2 esta en 1
        if (posy_J1 == 154 && cont == 1) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (154 - posy_secJ1 < 12) {
            GameOver1();
          }
        }

        //Si la posicion en y del jugador es 128 y cont2 esta en 2
        if (posy_J1 == 128 && cont == 2) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (128 - posy_secJ1 < 12) {
            GameOver1();
          }
        }

        //Si la posicion en y del jugador es 102 y cont2 esta en 3
        if (posy_J1 == 102 && cont == 5) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (102 - posy_secJ1 < 12) {
            GameOver1();
          }
        }

        //Si la posicion en y del jugador es 76 y cont2 esta en 4
        if (posy_J1 == 76 && cont == 6) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (76 - posy_secJ1 < 12) {
            GameOver1();
          }
        }

        //Si la posicion en y del jugador es 50 y cont2 esta en 6
        if (posy_J1 == 50 && cont == 8) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (50 - posy_secJ1 < 12) {
            GameOver1();
          }
        }
      }
    }

    //Comparacion para determinar si el jugador no se pudo agachar y por ende perdio el juego
    if (x1_bala > 43) {
      if (digitalRead(AGACHARSE_J1)) {
        GameOver1();
      }
    }

//***************************************************************************************************************************************
//Comparaciones J2
//***************************************************************************************************************************************
    //Revisa si se esta realizando la rutina 1 del Jugador 2
    if (flag2 == 0) {
      //Comparacion para determinar si el jugador no pudo saltar y por ende perdio el juego
      //Si el bloque toco al jugador
      if (x2 < 256) {

        //Si la posicion en y del jugador es 180
        if (posy_J2 == 180) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (180 - posy_secJ2 < 12) {
            GameOver2();
          }
        }

        //Si la posicion en y del jugador es 154 y cont2 esta en 1
        if (posy_J2 == 154 && cont2 == 1) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (154 - posy_secJ2 < 12) {
            GameOver2();
          }
        }

        //Si la posicion en y del jugador es 128 y cont2 esta en 2
        if (posy_J2 == 128 && cont2 == 2) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (128 - posy_secJ2 < 12) {
            GameOver2();
          }
        }

        //Si la posicion en y del jugador es 102 y cont2 esta en 3
        if (posy_J2 == 102 && cont2 == 3) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (102 - posy_secJ2 < 12) {
            GameOver2();
          }
        }

        //Si la posicion en y del jugador es 76 y cont2 esta en 4
        if (posy_J2 == 76 && cont2 == 4) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (76 - posy_secJ2 < 12) {
            GameOver2();
          }
        }

        //Si la posicion en y del jugador es 50 y cont2 esta en 6
        if (posy_J2 == 50 && cont2 == 6) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (50 - posy_secJ2 < 12) {
            GameOver2();
          }
        }
      }
    }

    //Revisa si se esta realizando la rutina 2 del Jugador 2
    if (flag2 == 1) {
      //Comparacion para determinar si el jugador no pudo saltar y por ende perdio el juego
      //Si el bloque toco al jugador
      if (x2 < 256) {

        //Si la posicion en y del jugador es 180
        if (posy_J2 == 180) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (180 - posy_secJ2 < 12) {
            GameOver2();
          }
        }

        //Si la posicion en y del jugador es 154 y cont2 esta en 1
        if (posy_J2 == 154 && cont2 == 3) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (154 - posy_secJ2 < 12) {
            GameOver2();
          }
        }

        //Si la posicion en y del jugador es 128 y cont2 esta en 2
        if (posy_J2 == 128 && cont2 == 4) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (128 - posy_secJ2 < 12) {
            GameOver2();
          }
        }

        //Si la posicion en y del jugador es 102 y cont2 esta en 3
        if (posy_J2 == 102 && cont2 == 5) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (102 - posy_secJ2 < 12) {
            GameOver2();
          }
        }

        //Si la posicion en y del jugador es 76 y cont2 esta en 4
        if (posy_J2 == 76 && cont2 == 7) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (76 - posy_secJ2 < 12) {
            GameOver2();
          }
        }

        //Si la posicion en y del jugador es 50 y cont2 esta en 6
        if (posy_J2 == 50 && cont2 == 8) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (50 - posy_secJ2 < 12) {
            GameOver2();
          }
        }
      }
    }

    //Revisa si se esta realizando la rutina 3 del Jugador 2
    if (flag2 == 2) {
      //Comparacion para determinar si el jugador no pudo saltar y por ende perdio el juego
      //Si el bloque toco al jugador
      if (x2 < 256) {

        //Si la posicion en y del jugador es 180
        if (posy_J2 == 180 && cont2 == 2) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (180 - posy_secJ2 < 12) {
            GameOver2();
          }
        }

        //Si la posicion en y del jugador es 154 y cont2 esta en 1
        if (posy_J2 == 154 && cont2 == 3) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (154 - posy_secJ2 < 12) {
            GameOver2();
          }
        }

        //Si la posicion en y del jugador es 128 y cont2 esta en 2
        if (posy_J2 == 128 && cont2 == 4) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (128 - posy_secJ2 < 12) {
            GameOver2();
          }
        }

        //Si la posicion en y del jugador es 102 y cont2 esta en 3
        if (posy_J2 == 102 && cont2 == 5) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (102 - posy_secJ2 < 12) {
            GameOver2();
          }
        }

        //Si la posicion en y del jugador es 76 y cont2 esta en 4
        if (posy_J2 == 76 && cont2 == 6) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (76 - posy_secJ2 < 12) {
            GameOver2();
          }
        }

        //Si la posicion en y del jugador es 50 y cont2 esta en 6
        if (posy_J2 == 50 && cont2 == 8) {

          //Si la posicion en y del jugador esta en el rango del bloque, el jugador pierde
          if (50 - posy_secJ2 < 12) {
            GameOver2();
          }
        }
      }
    }

    //Comparacion para determinar si el jugador no se pudo agachar y por ende perdio el juego
    if (x2_bala < 256) {
      if (digitalRead(AGACHARSE_J2)) {
        GameOver2();
      }
    }

//***************************************************************************************************************************************
//Saltar J2
//***************************************************************************************************************************************
    //Si el boton de saltar es presionado
    if (!digitalRead(SALTAR_J2)) {

      //Si la posicion en x del bloque es menor a 270
      if (x2 > 259) {

        //Si la bandera esta encendida
        if (Boton2 == 1) {
          if (((CONTADOR / 2) % 5 == 0)) {                                                                    //Cambiar la velocidad

            //Si la bandera esta en 1, realice el movimiento hacia arriba del jugador
            if (Saltar2 == 1) {
              posy_secJ2--;
              LCD_Bitmap(240, posy_secJ2, 16, 26, Salto);
              H_line(240, posy_secJ2 + 26, 16, 0x0255);
              posy_AgJ2 = posy_secJ2 + 8 ;
            }

            //Si se subieron 40 pixeles con respecto a la posicion original, apague la bandera
            if ((posy_J2 - posy_secJ2 == 40)) {                                                                 //Puedo modificar
              Saltar2 = 0;
            }

            //Si la bandera esta en 0, realice el movimiento hacia abajo del jugador
            if (Saltar2 == 0) {
              posy_secJ2++;
              LCD_Bitmap(240, posy_secJ2, 16, 26, Salto);
              H_line(240, posy_secJ2 - 1, 16, 0x0255);
              posy_AgJ2 = posy_secJ2 + 8;
            }

            //Si se volvio a la posicion original, vuelva a prender la bandera
            //Simulando asi una gravedad. Ademas apague la bandera del boton,
            //Logrando asi que si se mantien oprimido el boton de saltar, solo se
            //realice un salto
            if (posy_J2 - posy_secJ2 == 0) {
              Saltar2 = 1;
              Boton2 = 0;
              LCD_Bitmap(240, posy_secJ2, 16, 26, Mario);
            }
          }
        }

        //Si el jugador llega al borde de arriba de la pantalla, reinice la posicion del jugador
        if (posy_secJ2 == 0) {
          posy_J2 = 180;
          posy_secJ2 = 180;
          FillRect(240, 0, 16, 26, 0x0255);
        }
      }

      //Si la posicion en x del bloque es menor a 270
      if (x2 < 259) {

        //Si la bandera del boton esta encendida
        if (Boton2 == 1) {
          if (((CONTADOR / 2) % 5 == 0)) {

            //Si la bandera esta en 1, realice el movimiento hacia arriba del jugador
            if (Saltar2 == 1) {
              posy_secJ2--;
              LCD_Bitmap(240, posy_secJ2, 16, 26, Salto);
              H_line(240, posy_secJ2 + 26, 16, 0x0255);
              posy_AgJ2 = posy_secJ2 + 8 ;
              LCD_Bitmap(240, posy_secJ2, 16, 26, Mario);
            }

            //Si se subieron 40 pixeles con respecto a la posicion original, apague la bandera
            if ((posy_J2 - posy_secJ2 == 40)) {
              Saltar2 = 0;
            }

            //Si la bandera esta en 0, realice el movimiento hacia abajo del jugador
            if (Saltar2 == 0) {
              posy_secJ2++;
              LCD_Bitmap(240, posy_secJ2, 16, 26, Salto);
              H_line(240, posy_secJ2 - 1, 16, 0x0255);
              posy_AgJ2 = posy_secJ2 + 8;
            }

            //Si se llego al borde del bloque, vuelva a prender la bandera
            //Simulando asi una gravedad. Ademas apague la bandera del boton,
            //Logrando asi que si se mantien oprimido el boton de saltar, solo se
            //realice un salto
            if (posy_J2 - posy_secJ2 == 26) {
              Saltar2 = 1;
              Boton2 = 0;
              LCD_Bitmap(240, posy_secJ2, 16, 26, Mario);
            }
          }
        }

        //Si el jugador llega al borde de arriba de la pantalla, reinice la posicion del jugador
        if (posy_secJ2 == 0) {
          posy_J2 = 180;
          posy_secJ2 = 180;
          FillRect(240, 0, 16, 26, 0x0255);
        }
      }
    }

//***************************************************************************************************************************************
//Agacharse J2
//***************************************************************************************************************************************
    //Si no esta presionado el boton de agacharse o se suelta
    if (digitalRead(SALTAR_J2)) {

      //Mantenga la bandera en 1
      Boton2 = 1;

      //Actualice el valor de la posicion en y del jugador 1
      posy_J2 = posy_secJ2;

      //Si el Jugador 1 presiona el boton de agacharse
      if (!digitalRead(AGACHARSE_J2)) {
        if ((CONTADOR / 2) % 10 == 0) {
          //Se muestra el bitmap del Jugador 1 agachado
          LCD_Bitmap(240, posy_AgJ2, 16, 18, Agacharse);

          //Dependiendo si el valor en x del objeto es menor a 270 o no
          //Use diferentes posiciones para borrar el bitmap anterior

          if (x2_bala < 258) {
            FillRect(260, posy_J2 + 5, 16, 6, 0x0255);
          } else {
            FillRect(240, posy_J2, 16, 8, 0x0255);
          }

        }
      }

      //Si se suelta el boton de agacharse, se muestra el bitmap del Jugador 1 esparando saltar
      if (digitalRead(AGACHARSE_J2)) {
        LCD_Bitmap(240, posy_J2, 16, 26, Mario);
      }
    }

//***************************************************************************************************************************************
//Agacharse J1
//***************************************************************************************************************************************
    //Si no esta presionado el boton de agacharse o se suelta
    if (digitalRead(SALTAR_J1)) {

      //Mantenga la bandera en 1
      Boton1 = 1;

      //Actualice el valor de la posicion en y del jugador 1
      posy_J1 = posy_secJ1;

      //Si el Jugador 1 presiona el boton de agacharse
      if (!digitalRead(AGACHARSE_J1)) {
        if ((CONTADOR / 2) % 10 == 0) {
          //Se muestra el bitmap del Jugador 1 agachado
          LCD_Bitmap(67, posy_AgJ1, 16, 18, Agacharse2);

          //Dependiendo si el valor en x del objeto es menor a 270 o no
          //Use diferentes posiciones para borrar el bitmap anterior

          if (x1_bala > 60) {
            FillRect(47, posy_J1 + 5, 16, 6, 0x0255);
          } else {
            FillRect(67, posy_J1, 16, 8, 0x0255);
          }

        }
      }

      //Si se suelta el boton de agacharse, se muestra el bitmap del Jugador 1 esparando saltar
      if (digitalRead(AGACHARSE_J1)) {
        LCD_Bitmap(67, posy_J1, 16, 26, Mario2);
      }
    }

//***************************************************************************************************************************************
//Saltar J1
//***************************************************************************************************************************************
    //Si el boton de saltar es presionado
    if (!digitalRead(SALTAR_J1)) {

      //Si la posicion en x del bloque es menor a 270
      if (x1 < 42) {

        //Si la bandera esta encendida
        if (Boton1 == 1) {
          if (((CONTADOR / 2) % 5 == 0)) {                                                                    //Cambiar la velocidad

            //Si la bandera esta en 1, realice el movimiento hacia arriba del jugador
            if (Saltar1 == 1) {
              posy_secJ1--;
              LCD_Bitmap(67, posy_secJ1, 16, 26, Salto2);
              H_line(67, posy_secJ1 + 26, 16, 0x0255);
              posy_AgJ1 = posy_secJ1 + 8 ;
            }

            //Si se subieron 40 pixeles con respecto a la posicion original, apague la bandera
            if ((posy_J1 - posy_secJ1 == 40)) {                                                                 //Puedo modificar
              Saltar1 = 0;
            }

            //Si la bandera esta en 0, realice el movimiento hacia abajo del jugador
            if (Saltar1 == 0) {
              posy_secJ1++;
              LCD_Bitmap(67, posy_secJ1, 16, 26, Salto2);
              H_line(67, posy_secJ1 - 1, 16, 0x0255);
              posy_AgJ1 = posy_secJ1 + 8;
            }

            //Si se volvio a la posicion original, vuelva a prender la bandera
            //Simulando asi una gravedad. Ademas apague la bandera del boton,
            //Logrando asi que si se mantien oprimido el boton de saltar, solo se
            //realice un salto
            if (posy_J1 - posy_secJ1 == 0) {
              Saltar1 = 1;
              Boton1 = 0;
              LCD_Bitmap(67, posy_secJ1, 16, 26, Mario2);
            }
          }
        }

        //Si el jugador llega al borde de arriba de la pantalla, reinice la posicion del jugador
        if (posy_secJ1 == 0) {
          posy_J1 = 180;
          posy_secJ1 = 180;
          FillRect(67, 0, 16, 26, 0x0255);
        }
      }

      //Si la posicion en x del bloque es menor a 270
      if (x1 > 42) {

        //Si la bandera del boton esta encendida
        if (Boton1 == 1) {
          if (((CONTADOR / 2) % 5 == 0)) {

            //Si la bandera esta en 1, realice el movimiento hacia arriba del jugador
            if (Saltar1 == 1) {
              posy_secJ1--;
              LCD_Bitmap(67, posy_secJ1, 16, 26, Salto2);
              H_line(67, posy_secJ1 + 26, 16, 0x0255);
              posy_AgJ1 = posy_secJ1 + 8 ;
              LCD_Bitmap(67, posy_secJ1, 16, 26, Mario2);
            }

            //Si se subieron 40 pixeles con respecto a la posicion original, apague la bandera
            if ((posy_J1 - posy_secJ1 == 40)) {
              Saltar1 = 0;
            }

            //Si la bandera esta en 0, realice el movimiento hacia abajo del jugador
            if (Saltar1 == 0) {
              posy_secJ1++;
              LCD_Bitmap(67, posy_secJ1, 16, 26, Salto2);
              H_line(67, posy_secJ1 - 1, 16, 0x0255);
              posy_AgJ1 = posy_secJ1 + 8;
            }

            //Si se llego al borde del bloque, vuelva a prender la bandera
            //Simulando asi una gravedad. Ademas apague la bandera del boton,
            //Logrando asi que si se mantien oprimido el boton de saltar, solo se
            //realice un salto
            if (posy_J1 - posy_secJ1 == 26) {
              Saltar1 = 1;
              Boton1 = 0;
              LCD_Bitmap(67, posy_secJ1, 16, 26, Mario2);
            }
          }
        }

        //Si el jugador llega al borde de arriba de la pantalla, reinice la posicion del jugador
        if (posy_secJ1 == 0) {
          posy_J1 = 180;
          posy_secJ1 = 180;
          FillRect(67, 0, 16, 26, 0x0255);
        }
      }
    }

//***************************************************************************************************************************************
//Rutinas J1
//***************************************************************************************************************************************
    if (flag1 == 0) {                                                         //RUTINA 1
      if (AparicionBloque1Der == 0 & cont == 0) {                             //REVISIÓN DE BANDERA Y CONTADOR
        Plataforma1Der(8, 180);                                              //EJECUCIÓN DE LA FUNCIÓN DEL SPRITE
        if (AparicionBloque1Der == 1) {                                       //REVISIÓN SI LA FUNCIÓN HA TERMINADO
          cont = 1;                                                           //AUMENTAR EL CONTADOR
        }
      } if (cont == 1) {                                                      //REVISIÓN DEL CONTADOR
        AparicionBloque1Der = 0;                                            //APAGAR LA BANDERA DE LA FUNCIÓN EJECUTADA EN EL VALOR ANTERIOR DEL CONTADOR
        BalaDer(5, 144);                                                   //EJECUCIÓN DE LA FUNCIÓN DEL SPRITE
        if (AparicionBalaDer == 1) {                                        //REVISIÓN SI LA FUNCIÓN HA TERMINADO
          cont = 2;                                                         //AUMENTAR EL CONTADOR
        }
      } if (cont == 2) {
        AparicionBalaDer = 0;
        Plataforma2Der(6, 154);
        if (AparicionBloque2Der == 1) {
          cont = 3;
        }
      } if (cont == 3) {
        AparicionBloque2Der = 0;
        BalaDer(5, 118);
        if (AparicionBalaDer == 1) {
          cont = 4;
        }
      } if (cont == 4) {
        AparicionBalaDer = 0;
        BalaDer(5, 118);
        if (AparicionBalaDer == 1) {
          cont = 5;
        }
      } if (cont == 5) {
        AparicionBalaDer = 0;
        Plataforma2Der(6, 128);
        if (AparicionBloque2Der == 1) {
          cont = 6;
        }
      } if (cont == 6) {
        AparicionBloque2Der = 0;
        Plataforma1Der(8, 102);
        if (AparicionBloque1Der == 1) {
          cont = 7;
        }
      } if (cont == 7) {
        AparicionBloque1Der = 0;
        Plataforma3Der(10, 76);
        if (AparicionBloque3Der == 1) {
          cont = 8;
        }
      } if (cont == 8) {
        AparicionBloque3Der = 0;
        Plataforma3Der(10, 50);
        if (AparicionBloque3Der == 1) {
          cont = 9;
        }
      } if (cont == 9) {
        AparicionBloque3Der = 0;
        FillRect(65, 0, 26, 200, 0x0255);
        posy_J1 = 180;
        posy_secJ1 = 180;
        posy_AgJ1 = posy_J1 + 8;
        cont = 0;
        flag1 = random(0, 2);
      }
    }

    if (flag1 == 1) {                                                         //RUTINA 2
      if (AparicionBalaDer == 0 & cont == 0) {
        BalaDer(5, 170);
        if (AparicionBalaDer == 1) {
          cont = 1;
        }
      } if (cont == 1) {
        AparicionBalaDer = 0;
        Plataforma3Der(10, 180);
        if (AparicionBloque3Der == 1) {
          cont = 2;
        }
      } if (cont == 2) {
        AparicionBloque3Der = 0;
        Plataforma2Der(6, 154);
        if (AparicionBloque2Der == 1) {
          cont = 3;
        }
      } if (cont == 3) {
        AparicionBloque2Der = 0;
        BalaDer(5, 118);
        if (AparicionBalaDer == 1) {
          cont = 4;
        }
      } if (cont == 4) {
        AparicionBalaDer = 0;
        Plataforma1Der(8, 128);
        if (AparicionBloque1Der == 1) {
          cont = 5;
        }
      } if (cont == 5) {
        AparicionBloque1Der = 0;
        Plataforma1Der(8, 102);
        if (AparicionBloque1Der == 1) {
          cont = 6;
        }
      } if (cont == 6) {
        AparicionBloque1Der = 0;
        Plataforma3Der(10, 76);
        if (AparicionBloque3Der == 1) {
          cont = 7;
        }
      } if (cont == 7) {
        AparicionBloque3Der = 0;
        Plataforma2Der(6, 50);
        if (AparicionBloque2Der == 1) {
          cont = 8;
        }
      } if (cont == 8) {
        AparicionBloque2Der = 0;
        BalaDer(5, 14);
        if (AparicionBalaDer == 1) {
          cont = 9;
        }
      } if (cont == 9) {
        AparicionBalaDer = 0;
        FillRect(65, 0, 26, 200, 0x0255);
        posy_J1 = 180;
        posy_secJ1 = 180;
        posy_AgJ1 = posy_J1 + 8;
        cont = 0;
        flag1 = random(0, 2);
      }
    }

    if (flag1 == 2) {
      if (AparicionBloque2Der == 0 & cont == 0) {
        Plataforma2Der(6, 180);
        if (AparicionBloque2Der == 1) {
          cont = 1;
        }
      } if (cont == 1) {
        AparicionBloque2Der = 0;
        Plataforma3Der(10, 154);
        if (AparicionBloque3Der == 1) {
          cont = 2;
        }
      } if (cont == 2) {
        AparicionBloque3Der = 0;
        Plataforma1Der(8, 128);
        if (AparicionBloque1Der == 1) {
          cont = 3;
        }
      } if (cont == 3) {
        AparicionBloque1Der = 0;
        BalaDer(5, 92);
        if (AparicionBalaDer == 1) {
          cont = 4;
        }
      } if (cont == 4) {
        AparicionBalaDer = 0;
        BalaDer(5, 92);
        if (AparicionBalaDer == 1) {
          cont = 5;
        }
      } if (cont == 5) {
        AparicionBalaDer = 0;
        Plataforma2Der(6, 102);
        if (AparicionBloque2Der == 1) {
          cont = 6;
        }
      } if (cont == 6) {
        AparicionBloque2Der = 0;
        Plataforma3Der(10, 76);
        if (AparicionBloque3Der == 1) {
          cont = 7;
        }
      } if (cont == 7) {
        AparicionBloque3Der = 0;
        BalaDer(5, 40);
        if (AparicionBalaDer == 1) {
          cont = 8;
        }
      } if (cont == 8) {
        AparicionBalaDer = 0;
        Plataforma2Der(6, 50);
        if (AparicionBloque2Der == 1) {
          cont = 9;
        }
      } if (cont == 9) {
        AparicionBloque2Der = 0;
        FillRect(65, 0, 26, 200, 0x0255);
        posy_J1 = 180;
        posy_secJ1 = 180;
        posy_AgJ1 = posy_J1 + 8;
        cont = 0;
        flag1 = random(0, 2);

      }
    }

//***************************************************************************************************************************************
//Rutinas J2
//***************************************************************************************************************************************
    if (flag2 == 0) {                                                         //RUTINA 1
      if (AparicionBloque1Izq == 0 & cont2 == 0) {                            //REVISIÓN DE BANDERA Y CONTADOR
        Plataforma1Izq(8, 180);                                              //EJECUCIÓN DE LA FUNCIÓN DEL SPRITE
        if (AparicionBloque1Izq == 1) {                                       //REVISIÓN SI LA FUNCIÓN HA TERMINADO
          cont2 = 1;                                                           //AUMENTAR EL CONTADOR
        }
      } if (cont2 == 1) {                                                      //REVISIÓN DEL CONTADOR
        AparicionBloque1Izq = 0;                                            //APAGAR LA BANDERA DE LA FUNCIÓN EJECUTADA EN EL VALOR ANTERIOR DEL CONTADOR
        Plataforma2Izq(6, 154);                                             //EJECUCIÓN DE LA FUNCIÓN DEL SPRITE
        if (AparicionBloque2Izq == 1) {                                     //REVISIÓN SI LA FUNCIÓN HA TERMINADO
          cont2 = 2;                                                        //AUMENTAR EL CONTADOR
        }
      } if (cont2 == 2) {
        AparicionBloque2Izq = 0;
        Plataforma3Izq(10, 128);
        if (AparicionBloque3Izq == 1) {
          cont2 = 3;
        }
      } if (cont2 == 3) {
        AparicionBloque3Izq = 0;                                              //LIMPIAR LA BANDERA DE LA ULTIMA FUNCION EJECUTADA
        Plataforma2Izq(6, 102);
        if (AparicionBloque2Izq == 1) {
          cont2 = 4;
        }
      } if (cont2 == 4) {
        AparicionBloque2Izq = 0;
        Plataforma1Izq(8, 76);                                                //8 es la velocidad más rapida para los bloques
        if (AparicionBloque1Izq == 1) {
          cont2 = 5;
        }
      } if (cont2 == 5) {
        AparicionBloque1Izq = 0;
        BalaIzq(5, 40);
        if (AparicionBalaIzq == 1) {
          cont2 = 6;
        }
      } if (cont2 == 6) {
        AparicionBalaIzq = 0;
        Plataforma1Izq(8, 50);
        if (AparicionBloque1Izq == 1) {
          cont2 = 7;
        }
      } if (cont2 == 7) {
        AparicionBloque1Izq = 0;
        BalaIzq(5, 14);
        if (AparicionBalaIzq == 1) {
          cont2 = 8;
        }
      } if (cont2 == 8) {
        AparicionBalaIzq = 0;
        BalaIzq(5, 14);
        if (AparicionBalaIzq == 1) {
          cont2 = 9;
        }
      } if (cont2 == 9) {
        AparicionBalaIzq = 0;
        FillRect(237, 0, 26, 200, 0x0255);
        posy_J2 = 180;
        posy_secJ2 = 180;
        posy_AgJ2 = posy_J2 + 8;
        cont2 = 0;
        flag2 = random(0, 2);
      }
    }

    if (flag2 == 1) {                                                                   //RUTINA 2
      if (AparicionBalaIzq == 0 & cont2 == 0) {
        BalaIzq(5, 170);
        if (AparicionBalaIzq == 1) {
          cont2 = 1;
        }
      } if (cont2 == 1) {
        AparicionBalaIzq = 0;
        Plataforma2Izq(6, 180);
        if (AparicionBloque2Izq == 1) {
          cont2 = 2;
        }
      } if (cont2 == 2) {
        AparicionBloque2Izq = 0;
        BalaIzq(5, 144);
        if (AparicionBalaIzq == 1) {
          cont2 = 3;
        }
      } if (cont2 == 3) {
        AparicionBalaIzq = 0;
        Plataforma1Izq(8, 154);
        if (AparicionBloque1Izq == 1) {
          cont2 = 4;
        }
      } if (cont2 == 4) {
        AparicionBloque1Izq = 0;
        Plataforma3Izq(10, 128);
        if (AparicionBloque3Izq == 1) {
          cont2 = 5;
        }
      } if (cont2 == 5) {
        AparicionBloque3Izq = 0;
        Plataforma3Izq(10, 102);
        if (AparicionBloque3Izq == 1) {
          cont2 = 6;
        }
      } if (cont2 == 6) {
        AparicionBloque3Izq = 0;
        BalaIzq(5, 66);
        if (AparicionBalaIzq == 1) {
          cont2 = 7;
        }
      } if (cont2 == 7) {
        AparicionBalaIzq = 0;
        Plataforma2Izq(6, 76);
        if (AparicionBloque2Izq == 1) {
          cont2 = 8;
        }
      } if (cont2 == 8) {
        AparicionBloque2Izq = 0;
        Plataforma1Izq(8, 50);
        if (AparicionBloque1Izq == 1) {
          cont2 = 9;
        }
      } if (cont2 == 9) {
        AparicionBloque1Izq = 0;
        FillRect(237, 0, 26, 200, 0x0255);
        posy_J2 = 180;
        posy_secJ2 = 180;
        posy_AgJ2 = posy_J2 + 8;
        cont2 = 0;
        flag2 = random(0, 2);

      }
    }

    if (flag2 == 2) {
      if (AparicionBalaIzq == 0 & cont2 == 0) {
        BalaIzq(5, 170);
        if (AparicionBalaIzq == 1) {
          cont2 = 1;
        }
      } if (cont2 == 1) {
        AparicionBalaIzq = 0;
        BalaIzq(5, 170);
        if (AparicionBalaIzq == 1) {
          cont2 = 2;
        }
      } if (cont2 == 2) {
        AparicionBalaIzq = 0;
        Plataforma1Izq(8, 180);
        if (AparicionBloque1Izq == 1) {
          cont2 = 3;
        }
      } if (cont2 == 3) {
        AparicionBloque1Izq = 0;
        Plataforma2Izq(6, 154);
        if (AparicionBloque2Izq == 1) {
          cont2 = 4;
        }
      } if (cont2 == 4) {
        AparicionBloque2Izq = 0;
        Plataforma2Izq(6, 128);
        if (AparicionBloque2Izq == 1) {
          cont2 = 5;
        }
      } if (cont2 == 5) {
        AparicionBloque2Izq = 0;
        Plataforma3Izq(10, 102);
        if (AparicionBloque3Izq == 1) {
          cont2 = 6;
        }
      } if (cont2 == 6) {
        AparicionBloque3Izq = 0;
        Plataforma1Izq(8, 76);
        if (AparicionBloque1Izq == 1) {
          cont2 = 7;
        }
      } if (cont2 == 7) {
        AparicionBloque1Izq = 0;
        BalaIzq(5, 40);
        if (AparicionBalaIzq == 1) {
          cont2 = 8;
        }
      } if (cont2 == 8) {
        AparicionBalaIzq = 0;
        Plataforma3Izq(10, 50);
        if (AparicionBloque3Izq == 1) {
          cont2 = 9;
        }
      } if (cont2 == 9 ) {
        AparicionBloque3Izq = 0;
        FillRect(237, 0, 26, 200, 0x0255);
        posy_J2 = 180;
        posy_secJ2 = 180;
        posy_AgJ2 = posy_J2 + 8;
        cont2 = 0;
        flag2 = random(0, 2);
      }
    }
  }
}

//***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  for (uint8_t i = 0; i < 8; i++) {
    pinMode(DPINS[i], OUTPUT);
  }
  //****************************************
  // Secuencia de Inicialización
  //****************************************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER)
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40 | 0x80 | 0x20 | 0x08); // LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
  //  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = cmd;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = data;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);
  LCD_CMD(0x2c); // Write_memory_start
}
//***************************************************************************************************************************************
// Función para borrar la pantalla - parámetros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c) {
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c);
    }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
    LCD_DATA(c >> 8);
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8);
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y + h, w, c);
  V_line(x  , y  , h, c);
  V_line(x + w, y  , h, c);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  unsigned int i;
  for (i = 0; i < h; i++) {
    H_line(x  , y  , w, c);
    H_line(x  , y + i, w, c);
  }
}
//***************************************************************************************************************************************
// Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background)
//***************************************************************************************************************************************
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;

  if (fontSize == 1) {
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if (fontSize == 2) {
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }

  char charInput ;
  int cLength = text.length();
  Serial.println(cLength, DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength + 1];
  text.toCharArray(char_array, cLength + 1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1) {
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2) {
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}
//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 = x + width;
  y2 = y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k + 1]);
      //LCD_DATA(bitmap[k]);
      k = k + 2;
    }
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 =   x + width;
  y2 =    y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  int k = 0;
  int ancho = ((width * columns));
  if (flip) {
    for (int j = 0; j < height; j++) {
      k = (j * (ancho) + index * width - 1 - offset) * 2;
      k = k + width * 2;
      for (int i = 0; i < width; i++) {
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k + 1]);
        k = k - 2;
      }
    }
  } else {
    for (int j = 0; j < height; j++) {
      k = (j * (ancho) + index * width + 1 + offset) * 2;
      for (int i = 0; i < width; i++) {
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k + 1]);
        k = k + 2;
      }
    }


  }
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// Función aparición de bala con movimiento hacia la derecha
//***************************************************************************************************************************************
void BalaDer(uint8_t modulo, uint8_t y) {

  if ((CONTADOR / 2) % modulo == 0) {
    x1_bala++;
    LCD_Sprite(x1_bala, y, 23, 15, Bullet, 3, (CONTADOR / 100) % 3, 0, 0);
    V_line( x1_bala - 1, y, 15, 0x0255);

    if (x1_bala > 83) {
      x1_bala = 0;
      AparicionBalaDer = 1;
      FillRect(83, y, 23, 15, 0x0255);
    }
  }


}

//***************************************************************************************************************************************
// Función aparición de bala con movimiento hacia la izquierda
//***************************************************************************************************************************************
void BalaIzq(uint8_t modulo, uint8_t y) {


  if ((CONTADOR / 3) % modulo == 0) {
    x2_bala--;
    LCD_Sprite(x2_bala, y, 23, 15, Bullet, 3, (CONTADOR / 42) % 3, 1, 0);
    V_line( x2_bala + 23, y, 15, 0x0255);

    if (x2_bala < 225) {
      x2_bala = 297;
      AparicionBalaIzq = 1;
      FillRect(224, y, 23, 15, 0x0255);
    }
  }


}

//***************************************************************************************************************************************
// Función aparición de plataforma1 con movimiento hacia la derecha
//***************************************************************************************************************************************
void Plataforma1Der(uint8_t modulo, uint8_t y) {


  if ((CONTADOR / 2) % modulo == 0) {
    x1++;                                                                 //Aumentar posición en x
    LCD_Sprite(x1, y, 26, 12, plataforma1, 3, (CONTADOR / 200) % 3, 0, 0); //Sprite
    V_line( x1 - 1, y, 12, 0x0255);

    if (x1 > 65) {                                                        //Revisión si se llego a la distancia requerida
      x1 = 0;
      AparicionBloque1Der = 1;
      //FillRect(50, y, 26, 12, 0x0255);
    }
  }


}

//***************************************************************************************************************************************
// Función aparición de plataforma1 con movimiento hacia la izquierda
//***************************************************************************************************************************************
void Plataforma1Izq(uint8_t modulo, uint8_t y) {


  if ((CONTADOR / 2) % modulo == 0) {
    x2--;                                                                 //Aumentar posición en x
    LCD_Sprite(x2, y, 26, 12, plataforma1, 3, (CONTADOR / 200) % 3, 0, 0); //Sprite
    V_line( x2 + 26, y, 12, 0x0255);

    if (x2 < 238) {                                                        //Revisión si se llego a la distancia requerida
      x2 = 297;
      AparicionBloque1Izq = 1;
      //FillRect(249, y, 26, 12, 0x0255);
    }
  }


}

//***************************************************************************************************************************************
// Función aparición de plataforma2 con movimiento hacia la derecha
//***************************************************************************************************************************************
void Plataforma2Der(uint8_t modulo, uint8_t y) {



  if ((CONTADOR / 2) % modulo == 0) {
    x1++;                                                                 //Aumentar posición en x
    LCD_Sprite(x1, y, 26, 12, plataforma2, 3, (CONTADOR / 200) % 3, 0, 0); //Sprite
    V_line( x1 - 1, y, 12, 0x0255);

    if (x1 > 65) {                                                        //Revisión si se llego a la distancia requerida
      x1 = 0;
      AparicionBloque2Der = 1;
      //FillRect(50, y, 26, 12, 0x0255);
    }
  }


}

//***************************************************************************************************************************************
// Función aparición de plataforma2 con movimiento hacia la izquierda
//***************************************************************************************************************************************
void Plataforma2Izq(uint8_t modulo, uint8_t y) {


  if ((CONTADOR / 2) % modulo == 0) {
    x2--;                                                                 //Aumentar posición en x
    LCD_Sprite(x2, y, 26, 12, plataforma2, 3, (CONTADOR / 200) % 3, 0, 0); //Sprite
    V_line( x2 + 26, y, 12, 0x0255);

    if (x2 < 238) {                                                        //Revisión si se llego a la distancia requerida
      x2 = 297;
      AparicionBloque2Izq = 1;
      //FillRect(249, y, 26, 12, 0x0255);
    }
  }


}

//***************************************************************************************************************************************
// Función aparición de plataforma3 con movimiento hacia la derecha
//***************************************************************************************************************************************
void Plataforma3Der(uint8_t modulo, uint8_t y) {


  if ((CONTADOR / 2) % modulo == 0) {
    x1++;                                                                 //Aumentar posición en x
    LCD_Sprite(x1, y, 26, 12, plataforma3, 3, (CONTADOR / 200) % 3, 0, 0); //Sprite
    V_line( x1 - 1, y, 12, 0x0255);

    if (x1 > 65) {                                                        //Revisión si se llego a la distancia requerida
      x1 = 0;
      AparicionBloque3Der = 1;
      //FillRect(50, y, 26, 12, 0x0255);
    }
  }


}

//***************************************************************************************************************************************
// Función aparición de plataforma3 con movimiento hacia la izquierda
//***************************************************************************************************************************************
void Plataforma3Izq(uint8_t modulo, uint8_t y) {


  if ((CONTADOR / 2) % modulo == 0) {
    x2--;                                                                 //Aumentar posición en x
    LCD_Sprite(x2, y, 26, 12, plataforma3, 3, (CONTADOR / 200) % 3, 0, 0); //Sprite
    V_line( x2 + 26, y, 12, 0x0255);

    if (x2 < 238) {                                                        //Revisión si se llego a la distancia requerida
      x2 = 297;
      AparicionBloque3Izq = 1;
      //FillRect(249, y, 26, 12, 0x0255);
    }
  }


}

//***************************************************************************************************************************************
// Función que inicializa todos los fondos del juego
//***************************************************************************************************************************************

void Init(void) {
  LCD_Clear(0x14F4);
  FillRect(0, 0, 319, 206, 0x0255);

  //Elementos estáticos de la arena
  LCD_Bitmap(170, 15, 16, 16, sun);
  LCD_Bitmap(125, 134, 16, 17, boton);
  LCD_Bitmap(120, 30, 18, 16, nube);
  LCD_Bitmap(150, 70, 18, 16, nube);
  LCD_Bitmap(200, 10, 18, 16, nube);

  for (int x = 0; x < 319; x++) {
    LCD_Bitmap(x, 207, 16, 16, tile);
    LCD_Bitmap(x, 223, 16, 16, tile);
    x += 15;
  }

  for (uint8_t x = 125; x < 216; x++) {
    LCD_Bitmap(x, 150, 16, 16, tile2);
    x += 15;
  }

  for (uint8_t x = 157; x < 216; x++) {
    LCD_Bitmap(x, 134, 14, 16, coin);
    x += 14;
  }

  //Posicion inicial Jugador 1
  LCD_Bitmap(240, posy_J2, 16, 26, Mario);

  //Posicion inicial Jugador 1
  LCD_Bitmap(67, posy_J1, 16, 26, Mario2);

}

//***************************************************************************************************************************************
// Función de perdida del jugador 1
//***************************************************************************************************************************************
void GameOver1(void) {
  perd_J1 = 1;
  Estado = false;
}

//***************************************************************************************************************************************
// Función de perdida del jugador 2
//***************************************************************************************************************************************
void GameOver2(void) {
  perd_J2 = 1;
  Estado = false;
}
