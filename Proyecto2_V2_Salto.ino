//***************************************************************************************************************************************
/* Librería para el uso de la pantalla ILI9341 en modo 8 bits
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

#define SALTAR PF_4
#define AGACHARSE PF_0
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


//***************************************************************************************************************************************
// Variables globales
//***************************************************************************************************************************************
uint8_t AparicionBloqueDer = 1;           //Bandera
uint8_t AparicionBloqueIzq = 1;           //Bandera
uint8_t AparicionBalaDer = 1;             //Bandera
uint8_t AparicionBalaIzq = 1;             //Bandera
uint8_t Saltar = 1;                       //Bandera
uint8_t Boton = 1;                        //Bandera

uint8_t x1 = 0;                           //Variable para el movimiento para la derecha
uint16_t x2 = 297;                        //Variable para el movimiento para la izquireda

uint8_t CONTADOR = 0;                     //Contador para los sprites y animaciones 

uint8_t posy_J1 = 180;                    //Posicion original én y del personaje 1
uint8_t posy_secJ1 = 180;                 //Posicion en y del personaje 1 que se usará para aumentarla
uint8_t posy_AgJ1 = posy_J1 + 8;          //Posicion en y para el personaje 1 agachado

uint8_t posy_bala1 = 170;                 //Posición en y de la bala



//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************

void setup() {
  SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
  Serial.begin(9600);
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  LCD_Init();
  LCD_Clear(0x00);
  FillRect(0, 0, 319, 206, 0x0255);

  pinMode(PUSH1, INPUT_PULLUP);
  pinMode(PUSH2, INPUT_PULLUP);


  //Elementos estáticos de la arena
  LCD_Bitmap(180, 15, 16, 16, sun);
  LCD_Bitmap(125, 134, 16, 17, boton);
  LCD_Bitmap(120, 30, 18, 16, nube);
  LCD_Bitmap(150, 70, 18, 16, nube);
  LCD_Bitmap(220, 10, 18, 16, nube);

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
  LCD_Bitmap(260, posy_J1, 16, 26, Mario);

}
//***************************************************************************************************************************************
// Loop Infinito
//***************************************************************************************************************************************
void loop() {

  CONTADOR++;
  LCD_Sprite(150, 186, 16, 21, planta, 4, (CONTADOR / 60) % 4, 0, 0);       //Sprite para el movimiento de la planta

  //Si no esta presionado el boton de agacharse o se suelta
  if (digitalRead(SALTAR)) {
    
    //Mantenga la bandera en 1
    Boton = 1;

    //Actualice el valor de la posicion en y del jugador 1
    posy_J1 = posy_secJ1;
    
    //Si el Jugador 1 presiona el boton de agacharse
    if (!digitalRead(AGACHARSE)) {
      
      //Se muestra el bitmap del Jugador 1 agachado
      LCD_Bitmap(260, posy_AgJ1, 16, 18, Agacharse);

      //Dependiendo si el valor en x del objeto es menor a 270 o no
      //Use diferentes posiciones para borrar el bitmap anterior
      if (x2 < 270) {
        FillRect(260, posy_secJ1 + 5, 16, 6, 0x0255);
      } else {
        FillRect(260, posy_J1, 16, 8, 0x0255);
      }
    }

    //Si se suelta el boton de agacharse, se muestra el bitmap del Jugador 1 esparando saltar
    if(digitalRead(AGACHARSE)){
      LCD_Bitmap(260, posy_J1, 16, 26, Mario);
    }
  }

  //Si el boton de saltar es presionado
  if (!digitalRead(SALTAR)) {

    //Si la posicion en x del bloque es menor a 270
    if (x2 > 270) {

      //Si la bandera esta encendida
      if (Boton == 1) {
        if (((CONTADOR / 2) % 5 == 0)) {

          //Si la bandera esta en 1, realice el movimiento hacia arriba del jugador
          if (Saltar == 1) {
            posy_secJ1--;
            LCD_Bitmap(260, posy_secJ1, 16, 26, Salto);
            H_line(260, posy_secJ1 + 26, 16, 0x0255);
            posy_AgJ1 = posy_secJ1 + 8 ;
          }

          //Si se subieron 40 pixeles con respecto a la posicion original, apague la bandera
          if ((posy_J1 - posy_secJ1 == 40)) {
            Saltar = 0;
          }

          //Si la bandera esta en 0, realice el movimiento hacia abajo del jugador
          if (Saltar == 0) {
            posy_secJ1++;
            LCD_Bitmap(260, posy_secJ1, 16, 26, Salto);
            H_line(260, posy_secJ1 - 1, 16, 0x0255);
            posy_AgJ1 = posy_secJ1 + 8;
          }

          //Si se volvio a la posicion original, vuelva a prender la bandera
          //Simulando asi una gravedad. Ademas apague la bandera del boton,
          //Logrando asi que si se mantien oprimido el boton de saltar, solo se
          //realice un salto
          if (posy_J1 - posy_secJ1 == 0) {
            Saltar = 1;
            Boton = 0;
            LCD_Bitmap(260, posy_secJ1, 16, 26, Mario);
          }
        }
      }

      //Si el jugador llega al borde de arriba de la pantalla, reinice la posicion del jugador
      if (posy_secJ1 == 0) {
        posy_J1 = 180;
        posy_secJ1 = 180;
        FillRect(260, 0, 16, 26, 0x0255);
      }
    }

    //Si la posicion en x del bloque es menor a 270
    if (x2 < 270) {

      //Si la bandera del boton esta encendida
      if (Boton == 1) {
        if (((CONTADOR / 2) % 5 == 0)) {

          //Si la bandera esta en 1, realice el movimiento hacia arriba del jugador
          if (Saltar == 1) {
            posy_secJ1--;
            LCD_Bitmap(260, posy_secJ1, 16, 26, Salto);
            H_line(260, posy_secJ1 + 26, 16, 0x0255);
            posy_AgJ1 = posy_secJ1 + 8 ;
            LCD_Bitmap(260, posy_secJ1, 16, 26, Mario);
          }

          //Si se subieron 40 pixeles con respecto a la posicion original, apague la bandera
          if ((posy_J1 - posy_secJ1 == 40)) {
            Saltar = 0;
          }

          //Si la bandera esta en 0, realice el movimiento hacia abajo del jugador
          if (Saltar == 0) {
            posy_secJ1++;
            LCD_Bitmap(260, posy_secJ1, 16, 26, Salto);
            H_line(260, posy_secJ1 - 1, 16, 0x0255);
            posy_AgJ1 = posy_secJ1 + 8;
          }

          //Si se llego al borde del bloque, vuelva a prender la bandera
          //Simulando asi una gravedad. Ademas apague la bandera del boton,
          //Logrando asi que si se mantien oprimido el boton de saltar, solo se
          //realice un salto
          if (posy_J1 - posy_secJ1 == 11) {
            Saltar = 1;
            Boton = 0;
            LCD_Bitmap(260, posy_secJ1, 16, 26, Mario);
          }
        }
      }

      //Si el jugador llega al borde de arriba de la pantalla, reinice la posicion del jugador
      if (posy_secJ1 == 0) {
        posy_J1 = 180;
        posy_secJ1 = 180;
        FillRect(260, 0, 16, 26, 0x0255);
      }
    }
  }

  //BalaDer(10, 100);
  //BalaDer(10, 80);
  //BalaDer(10, posy_bala1);
  //BalaIzq(10, 100);
  Plataforma3Der(25, 195);

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
void BalaIzq(uint8_t modulo, uint8_t y) {

  if (AparicionBalaDer == 1) {
    if ((CONTADOR / 2) % modulo == 0) {
      x1++;
      LCD_Sprite(x1, y, 23, 15, Bullet, 3, (CONTADOR / 100) % 3, 0, 0);
      V_line( x1 - 1, y, 23, 0x0255);

      if (x1 > 50) {
        x1 = 0;
        AparicionBalaDer = 0;
        FillRect(50, y, 23, 15, 0x0255);
      }
    }
  }

}

//***************************************************************************************************************************************
// Función aparición de bala con movimiento hacia la izquierda
//***************************************************************************************************************************************
void BalaDer(uint8_t modulo, uint8_t y) {

  if (AparicionBalaIzq == 1) {
    if ((CONTADOR / 2) % modulo == 0) {
      x2--;
      LCD_Sprite(x2, y, 23, 15, Bullet, 3, (CONTADOR / 30) % 3, 1, 0);
      V_line( x2 + 23, y, 23, 0x0255);

      if (x2 < 250) {
        x2 = 297;
        AparicionBalaIzq = 0;
        FillRect(249, y, 23, 15, 0x0255);
      }
    }
  }

}

//***************************************************************************************************************************************
// Función aparición de plataforma1 con movimiento hacia la derecha
//***************************************************************************************************************************************
void Plataforma1Izq(uint8_t modulo, uint8_t y) {

  if (AparicionBloqueDer == 1) {                                              //Verificación si la bandera se encuentra encendida
    if ((CONTADOR / 2) % modulo == 0) {
      x1++;                                                                 //Aumentar posición en x
      LCD_Sprite(x1, y, 26, 12, plataforma1, 3, (CONTADOR / 200) % 3, 0, 0); //Sprite
      V_line( x1 - 1, y, 26, 0x0255);

      if (x1 > 50) {                                                        //Revisión si se llego a la distancia requerida
        x1 = 0;
        AparicionBloqueDer = 0;
        FillRect(50, y, 26, 12, 0x0255);
      }
    }
  }

}

//***************************************************************************************************************************************
// Función aparición de plataforma1 con movimiento hacia la izquierda
//***************************************************************************************************************************************
void Plataforma1Der(uint8_t modulo, uint8_t y) {

  if (AparicionBloqueIzq == 1) {                                              //Verificación si la bandera se encuentra encendida
    if ((CONTADOR / 2) % modulo == 0) {
      x2--;                                                                 //Aumentar posición en x
      LCD_Sprite(x2, y, 26, 12, plataforma1, 3, (CONTADOR / 200) % 3, 0, 0);          //Sprite
      V_line( x2 + 26, y, 26, 0x0255);

      if (x2 < 258) {                                                        //Revisión si se llego a la distancia requerida
        x2 = 297;
        AparicionBloqueIzq = 0;
        FillRect(249, y, 26, 12, 0x0255);
      }
    }
  }

}

//***************************************************************************************************************************************
// Función aparición de plataforma2 con movimiento hacia la derecha
//***************************************************************************************************************************************
void Plataforma2Izq(uint8_t modulo, uint8_t y) {

  if (AparicionBloqueDer == 1) {                                              //Verificación si la bandera se encuentra encendida
    if ((CONTADOR / 2) % modulo == 0) {
      x1++;                                                                 //Aumentar posición en x
      LCD_Sprite(x1, y, 26, 12, plataforma2, 3, (CONTADOR / 200) % 3, 0, 0); //Sprite
      V_line( x1 - 1, y, 26, 0x0255);

      if (x1 > 50) {                                                        //Revisión si se llego a la distancia requerida
        x1 = 0;
        AparicionBloqueDer = 0;
        FillRect(50, y, 26, 12, 0x0255);
      }
    }
  }

}

//***************************************************************************************************************************************
// Función aparición de plataforma2 con movimiento hacia la izquierda
//***************************************************************************************************************************************
void Plataforma2Der(uint8_t modulo, uint8_t y) {

  if (AparicionBloqueIzq == 1) {                                              //Verificación si la bandera se encuentra encendida
    if ((CONTADOR / 2) % modulo == 0) {
      x2--;                                                                 //Aumentar posición en x
      LCD_Sprite(x2, y, 26, 12, plataforma2, 3, (CONTADOR / 200) % 3, 0, 0);          //Sprite
      V_line( x2 + 26, y, 26, 0x0255);

      if (x2 < 258) {                                                        //Revisión si se llego a la distancia requerida
        x2 = 297;
        AparicionBloqueIzq = 0;
        FillRect(249, y, 26, 12, 0x0255);
      }
    }
  }

}

//***************************************************************************************************************************************
// Función aparición de plataforma3 con movimiento hacia la derecha
//***************************************************************************************************************************************
void Plataforma3Izq(uint8_t modulo, uint8_t y) {

  if (AparicionBloqueDer == 1) {                                              //Verificación si la bandera se encuentra encendida
    if ((CONTADOR / 2) % modulo == 0) {
      x1++;                                                                 //Aumentar posición en x
      LCD_Sprite(x1, y, 26, 12, plataforma3, 3, (CONTADOR / 200) % 3, 0, 0); //Sprite
      V_line( x1 - 1, y, 26, 0x0255);

      if (x1 > 50) {                                                        //Revisión si se llego a la distancia requerida
        x1 = 0;
        AparicionBloqueDer = 0;
        FillRect(50, y, 26, 12, 0x0255);
      }
    }
  }

}

//***************************************************************************************************************************************
// Función aparición de plataforma3 con movimiento hacia la izquierda
//***************************************************************************************************************************************
void Plataforma3Der(uint8_t modulo, uint8_t y) {

  if (AparicionBloqueIzq == 1) {                                              //Verificación si la bandera se encuentra encendida
    if ((CONTADOR / 2) % modulo == 0) {
      x2--;                                                                 //Aumentar posición en x
      LCD_Sprite(x2, y, 26, 12, plataforma3, 3, (CONTADOR / 200) % 3, 0, 0);          //Sprite
      V_line( x2 + 26, y, 26, 0x0255);

      if (x2 < 258) {                                                        //Revisión si se llego a la distancia requerida
        x2 = 297;
        AparicionBloqueIzq = 0;
        //FillRect(249, y, 26, 12, 0x0255);
      }
    }
  }

}

//***************************************************************************************************************************************
// Rutinas 1
//***************************************************************************************************************************************
