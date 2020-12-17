/**********************************************************************************************
 *                 Protección de un cuarto con Arduino MEGA, RFID y passcode
 *
 * Proyecto creado por:
 *                      Nicole Rodríguez González
 *                      Víctor Hugo Vázquez Gómez
 * 
 * Componentes usados:
 *                    - Servomotor
 *                    - Botón
 *                    - Keypad 4x4
 *                    - Pantalla LCD 20x4 (con el módulo I2C)
 *                    - RFID
 * Funcionamiento:
 *                    1. Pedir la contraseña
 *                        a. Si la contraseña es correcta, pasa al paso 2
 *                        b. Si es incorrecta pasa al paso 1
 *                        c. Si falla 4 veces se bloquea la entrada por 1 minuto, así hasta bloquearlo por 10 hrs
 *                    2. Pedir la tarjeta.
 *                        a. Dar un lapso de 1 minuto después de introducir la contraseña para pasar la tarjeta.
 *                        b. Si pasa un minuto, pedir el llavero también.
 *                            - Si se tarda un minuto en poner el llavero también, se bloquea el sistema por 20 minutos.
 *                        c. Si pasa la tarjeta bien, antes del minuto, acciona el servomotor
 *                    3. Accionar el servomotor para quitar el seguro de la puerta
 *                    4. Mostrar en el display que la puerta ha sido desbloqueada
 *                    5. Al cabo de 20segs se volverá a bloquear el seguro
***********************************************************************************************/

/************************************** Librerías usadas **************************************/
// Servomotor
#include <Servo.h>
// RFID
#include <MFRC522.h>
#include <deprecated.h>
#include <MFRC522Extended.h>
#include <require_cpp11.h>
// Keypad
#include <Key.h>
#include <Keypad.h>
// Pantalla LCD con I2C
#include <LiquidCrystal_I2C.h>
/************************************** Librerías usadas **************************************/


/************************** Configurando pines para el Arduino MEGA ***************************/

// Pines para el Servomotor
int pinServo = 5;

// Pines para el botón
int pinBoton = 6;

// Pines para el RFID
int pinSDA = 10;
int pinSCK = 13;
int pinMOSI = 11;
int pinMISO = 12;
int pinRST = 9;

// Crear el objeto para el LCD Display con I2C y lo almacena en la dirección 0x3F
LiquidCrystal_I2C lcd(0x3F, 20, 4);

// Configuración para el Keypad 4x4
const byte FILAS = 4;
const byte COLUMNAS = 4;
// Arreglo bidimensional para implementar el teclado
char keys[FILAS][COLUMNAS] = {
  {'1','2','3','A'},

  {'4','5','6','B'},

  {'7','8','9','C'},

  {'*','0','#','D'}
};
byte pinFilaKeyPad[FILAS] = {30, 31, 32, 33}; // Pines para el Keypad 4x4
byte pinColumnaKeyPad[COLUMNAS] = {34, 35, 36, 37,}; // Pines para el Keypad 4x4


/************************** Configurando pines para el Arduino MEGA ***************************/


void setup() {
  // put your setup code here, to run once:

  /******************************** LCD *************************************/
  // Iniciar el LCD.
  lcd.init();
  // Enciende la luz de fondo.
  lcd.backlight();
  // Escribe en pantalla el saludo de bienvenida.
  lcd.print("Bienvenido");

  

}

void loop() {
  // put your main code here, to run repeatedly:

}
