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
#include <SPI.h>
// Keypad
#include <Key.h>
#include <Keypad.h>
#include <Password.h>
// Pantalla LCD con I2C
#include <LiquidCrystal_I2C.h>
/************************************** Librerías usadas **************************************/


/************************** Configurando pines para el Arduino MEGA ***************************/

/************************************** Servomotor **************************************/
// Pines para el Servomotor
Servo puerta;  // Creamos un objeto de tipo Servo
int pinServo = 5;
int puertaBloqueada = 100; // Por defecto los 100° son para tener bloqueada la puerta
int pueraDesbloqueada = -180; // Por defecto los -180° son para tener desbloqueada la puerta
int tiempoEspera = 10000; // El tiempo de espera para que el servomotor cambie de ángulo, 10000 son 10 segundos.

/************************************** Botón **************************************/
// Pines para el botón
int pinBoton = 6;
int estadoBoton = 0;

/************************************** RFID **************************************/
// Pines para el RFID
int pinSDA = 10;
int pinSCK = 13;
int pinMOSI = 11;
int pinMISO = 12;
int pinRST = 9;
MFRC522 mfrc522(pinSDA, pinRST); // Creamos el objeto para el RC522

/************************************** Display LCD **************************************/
// Crear el objeto para el LCD Display con I2C y lo almacena en la dirección 0x3F
LiquidCrystal_I2C lcd(0x3F, 20, 4);

/************************************** Keypad 4x4 **************************************/
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
Keypad keypad = Keypad(makeKeymap(keys), pinFilaKeyPad, pinColumnaKeyPad, FILAS, COLUMNAS);

/************************************** Contraseña **************************************/
String nuevaContrasenaString; // Mantener la contraseña
char nuevaContrasena[6]; // Caracter por caracter de nuevaContrasenaString
// Inicializar la contraseña a 3645
Password password = Password("3645");
byte longitudMaximaPassword = 6; // La cantidad máxima de caracteres para la contraseña
byte longitudActualPassword = 0; // Cantidad actual de caracteres ingresados (por defecto 0)


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
  delay(500);
  lcd.clear();
  lcd.print("Contrasena: ");
  /******************************** LCD *************************************/

  /******************************** SERVO ************************************/
  puerta.attach(pinServo);     // El pin al que estará conectado
  puerta.write(puertaBloqueada); // Pone el servomotor en los grados que le especificamos
  pinMode(pinServo, OUTPUT); // Establecemos que el pin será de salida
  /******************************** SERVO ************************************/

  /******************************** RFID ************************************/
  SPI.begin(); // Iniciamos el bus SPI
  mfrc522.PCD_Init(); // Iniciamos el MFRC522
  /******************************** RFID ************************************/
  

}

void loop() {
  // put your main code here, to run repeatedly:

  /***************************** Por si pulsamos el botón **************************/
  // Obtenemos la tecla pulsada
  char key = keypad.getKey();  
  // Almacenamos el estado del botón
  estadoBoton = digitalRead(pinBoton);
  // Preguntamos si está presionado, para accionar el servomotor
  if (estadoBoton == HIGH) {
    // Desbloqueamos la puerta
    puerta.write(pueraDesbloqueada);
    delay(tiempoEspera);
    puerta.write(puertaBloqueada);
    setup(); // Volver a inicializar los componentes, por si acaso
  }
  /***************************** Por si pulsamos el botón **************************/

  /***************************** Pedir la contraseña **************************/
  // Obtenemos la tecla pulsada
  char key = keypad.getKey();

  // Condición que revisa si pulsamos la tecla
  if ( key != NO_KEY ) {
    delay(60);  // Espera de 60ms para que no se pulse tan rápido las teclas
    switch( key ) {
      case 'A': break; // Si pulsamos la tecla 'A' se cancela
      case 'B': break; // Si pulsamos la tecla 'B' se cancela
      case 'C': break; // Si pulsamos la tecla 'C' se cancela
      case '#': verificarPassword(); break; // Si pulsamos la tecla '#' verifica la contraseña
      case '*': resetPassword(); break; // Si pulsamos la tecla '*' se borra todo lo que ingresaste
      default: manejarEntradaPassword( key ); // Caso por defecto, si pulsamos cualquier otra tecla
    }
  }
  /***************************** Pedir la contraseña **************************/

}

/*
 * 
 * verificarPassword
 * 
 * Verifica la contraseña ingresada con la previamente establecida.
 * 
 * Si la contraseña hace match, pasa a llamar a una función que pide
 * pasar la tarjeta RFID sobre su sensor.
 * 
 * returns void;
 */
void verificarPassword() {

  // Si la contraseña es correcta
  if ( password.evaluate() ) {
    lcd.clear(); // Limpia el LCD
    lcd.print("Contraseña correcta");
    delay(1000); // Espera 1 segundo
    lcd.clear();
    lcd.print("Pasa la tarjeta de acceso");
    // ToDo: Llamar una función que lea la tarjeta rfid
  }
  else {
    lcd.clear();
    lcd.print("Acceso denegado");
    delay(10000);
    setup(); // Llama setup para posteriormente comenzar el loop de nuevo
  }
  
}

/*
 *
 * resetPassword
 * 
 * Resetea la contraseña, la longitud actual de la misma y limpia
 * el LCD.
 * 
 * returns void;
 *
 */
void resetPassword() {
  password.reset(); // Reset de la contraseña que se está ingresando
  longitudActualPassword = 0; // Restablecemos la longitud actual a 0
  lcd.clear(); // Limpiamos la pantalla
  
}

/*
 * 
 * manejarEntradaPassword
 * 
 * Maneja la entrada de las teclas
 * 
 * retunrs void;
 */
void manejarEntradaPassword( char key ) {
  longitudActualPassword++; // Aumentar en 1 la longitud
  password.append( key ); // Agregar la key a la contraseña para comparar
  lcd.print("*"); // Podríamos imprimir lo que ingresa, pero mejor el * para mayor seguridad

  // Si se llega al limite de caracteres pasar a verificar en 
  // lugar de pulsar '#' para hacerlo
  if ( longitudActualPassword == longitudMaximaPassword ) {
    verificarPassword(); // Invocamos la función
  }
  
}
