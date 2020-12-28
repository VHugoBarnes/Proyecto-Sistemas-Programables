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
 *                    2. Pedir la tarjeta.
 *                    3. Accionar el servomotor para quitar el seguro de la puerta si la tarjeta es correcta
 *                    4. Mostrar en el display que la puerta ha sido desbloqueada
 *                    5. Al cabo de 20segs se volverá a bloquear el seguro
 *                    6. Si esta dentro del cuarto, y si pulsa el botón, desbloquear puerta
***********************************************************************************************/

/************************************** Librerías usadas **************************************/
// Servomotor
#include <Servo.h>
// RFID
#include <MFRC522.h>
#include <MFRC522Extended.h>
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
int pinServo = 6;
int puertaBloqueada = 90; // Por defecto los 100° son para tener bloqueada la puerta
int pueraDesbloqueada = 180; // Por defecto los -180° son para tener desbloqueada la puerta
int tiempoEspera = 10000; // El tiempo de espera para que el servomotor cambie de ángulo, 10000 son 10 segundos.

/************************************** Botón **************************************/
// Pines para el botón
int pinBoton = 7;
//int estadoBoton = 0;

/************************************** RFID **************************************/
// Pines para el RFID
int pinRST = 5; // Amarilla
int pinSDA = 53; // Verde
int pinMOSI = 51; // Naranja
int pinMISO = 50; // Morada
int pinSCK = 52; // Azul
MFRC522 mfrc522(pinSDA, pinRST); // Creamos el objeto para el RC522
// Identificaciones
byte actualUID[4]; // Almacenará el código del tag leído
byte usuario1[4] = {0xE5, 0xE7, 0x26, 0x23}; // Tarjeta
byte usuario2[4] = {0xA9, 0x61, 0xDD, 0xA2}; // Llavero

/************************************** Display LCD **************************************/
// Crear el objeto para el LCD Display con I2C y lo almacena en la dirección 0x3F
LiquidCrystal_I2C lcd(0x27, 20, 4);

/************************************** Keypad 4x4 **************************************/
// Configuración para el Keypad 4x4
const byte FILAS = 4;
const byte COLUMNAS = 4;
// Arreglo bidimensional para implementar el teclado
char keys[FILAS][COLUMNAS] = {
  {'1','2','3','*'},

  {'4','5','6','#'},

  {'7','8','9','0'},

  {'-','.',',','|'}
};
byte pinFilaKeyPad[FILAS] = {34,35,36,37}; // Pines para el Keypad 4x4
byte pinColumnaKeyPad[COLUMNAS] = {30,31,32,33}; // Pines para el Keypad 4x4
Keypad keypad = Keypad(makeKeymap(keys), pinFilaKeyPad, pinColumnaKeyPad, FILAS, COLUMNAS);

/************************************** Contraseña **************************************/
String nuevaContrasenaString; // Mantener la contraseña
char nuevaContrasena[6]; // Caracter por caracter de nuevaContrasenaString
// Inicializar la contraseña a 3645
Password password = Password("3645");
byte longitudMaximaPassword = 6; // La cantidad máxima de caracteres para la contraseña
byte longitudActualPassword = 0; // Cantidad actual de caracteres ingresados (por defecto 0)
int intentosPassword = 0; // Cuantos intentos de ingresar la contraseña ha hecho
int intentosMaximosPassword = 4;

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
  Serial.begin(9600);
  /******************************** SERVO ************************************/
  puerta.attach(pinServo);     // El pin al que estará conectado
  puerta.write(puertaBloqueada); // Pone el servomotor en los grados que le especificamos
  pinMode(pinServo, OUTPUT); // Establecemos que el pin será de salida
  /******************************** SERVO ************************************/

  /******************************** BOTON ************************************/
  pinMode(pinBoton, INPUT_PULLUP);
  /******************************** BOTON ************************************/

  /******************************** RFID ************************************/
  SPI.begin(); // Iniciamos el bus SPI
  mfrc522.PCD_Init(); // Iniciamos el MFRC522
  /******************************** RFID ************************************/

}

void loop() {
  // put your main code here, to run repeatedly:

  /***************************** Por si pulsamos el botón **************************/
  // Almacenamos el estado del botón
  int estadoBoton = digitalRead(pinBoton);
  // Preguntamos si está presionado, para accionar el servomotor
  if (estadoBoton == LOW) {
    Serial.println("LOW");
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
    Serial.println(key);
    delay(60);  // Espera de 60ms para que no se pulse tan rápido las teclas
    switch( key ) {
      case 'A': break; // Si pulsamos la tecla 'A' se cancela
      case 'B': break; // Si pulsamos la tecla 'B' se cancela
      case 'C': break; // Si pulsamos la tecla 'C' se cancela
      case '#': Serial.println(key);verificarPassword(); break; // Si pulsamos la tecla '#' verifica la contraseña
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
    longitudActualPassword = 0;
    lcd.clear(); // Limpia el LCD
    lcd.print("Contrasena correcta");
    delay(1000); // Espera 1 segundo
    lcd.clear();
    lcd.print("Pasa la tarjeta de acceso");
    password.reset(); // Reset de la contraseña que se está ingresando
    // Llamar una función que lea la tarjeta rfid
    verificarRfid();
  }
  // Si la contraseña es incorrecta
  else {
    if (intentosPassword != intentosMaximosPassword) {
        longitudActualPassword = 0;
        password.reset(); // Reset de la contraseña que se está ingresando
        lcd.clear();
        lcd.print("Contrasena Incorrecta");
        delay(10000);
        intentosPassword++; // Incrementamos en 1 los intentos
        setup(); // Llama setup para posteriormente comenzar el loop de nuevo
    } else { // Quiere decir que ya uso sus 4 intentos
      longitudActualPassword = 0;
      password.reset(); // Reset de la contraseña que se está ingresando
      lcd.clear();
      lcd.print("Sistema bloqueado temporalmente");
      delay(30000); // Se bloquea por 30 segs
      intentosPassword = 0;
    }
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
 *
 */
void resetPassword() {
  password.reset(); // Reset de la contraseña que se está ingresando
  longitudActualPassword = 0; // Restablecemos la longitud actual a 0
  lcd.clear(); // Limpiamos la pantalla
  lcd.print("Contrasena: ");
}

/*
 * 
 * manejarEntradaPassword
 * 
 * Maneja la entrada de las teclas
 * 
 * 
 * retunrs void;
 */
void manejarEntradaPassword( char key ) {
  
  longitudActualPassword++; // Aumentar en 1 la longitud
  password.append( key ); // Agregar la key a la contraseña para comparar
  lcd.print(key); // Podríamos imprimir lo que ingresa, pero mejor el * para mayor seguridad

  // Si se llega al limite de caracteres pasar a verificar en 
  // lugar de pulsar '#' para hacerlo
  Serial.print("longitud actual: ");
  Serial.println(longitudActualPassword);
  if ( longitudActualPassword == longitudMaximaPassword ) {
    verificarPassword(); // Invocamos la función
  }
  
}

/*
 * verificarRfid
 * 
 * Maneja la entrada de tarjetas RFID y acciona el servomotor si 
 * es correcta.
 * 
 * returns void;
 */
void verificarRfid() {
  // Revisamos si hay nuevas tarjetas presentes
  Serial.println("Tarjeta RFID");
  boolean hayTarjeta = false;
  boolean estaLeyendo = false;

  while ( !hayTarjeta ) { // Estara en el bucle mientras no haya una tarjeta

    if ( mfrc522.PICC_IsNewCardPresent() ) {
      hayTarjeta = true;
      // Seleccionamos una tarjeta
      Serial.println("RFID presente");

      while ( !estaLeyendo ) {

        if ( mfrc522.PICC_ReadCardSerial() ) {
          estaLeyendo = true;
          Serial.println("Seleccionando RFID");
          mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
            // Comparamos los UID para determinar si es uno de los usuarios
            if ( isEqualArray(mfrc522.uid.uidByte, usuario1, 4) ) {
              Serial.println("Comparando Arrays");
                lcd.clear();
                lcd.print("Acceso Concedido, puerta desbloqueada");
                // Desbloqueamos la puerta
                puerta.write(pueraDesbloqueada);
                delay(tiempoEspera);
                puerta.write(puertaBloqueada);
                setup(); // Volver a inicializar los componentes, por si acaso
            }
            else if ( isEqualArray(mfrc522.uid.uidByte, usuario2, 4) ) {
              Serial.println("Comparando Arrays");
                lcd.clear();
                lcd.print("Acceso Concedido, puerta desbloqueada");
                // Desbloqueamos la puerta
                puerta.write(pueraDesbloqueada);
                delay(tiempoEspera);
                puerta.write(puertaBloqueada);
                setup(); // Volver a inicializar los componentes, por si acaso
            }
            else {
                lcd.clear();
                lcd.print("Acceso denegado, credencial incorrecta");
                delay(tiempoEspera);
                setup();
            }
        }
        
      }
      
    }
    
  }
}

/*
 * compararArray
 * 
 * Su nombre lo dice, compara los contenidos de un array de 4.
 * Usado para comparas las uid de tarjetas MIFARE
 * 
 * returns booblean;
 */
bool isEqualArray(byte* arrayA, byte* arrayB, int length)
{
  for (int index = 0; index < length; index++)
  {
    if (arrayA[index] != arrayB[index]) return false;
  }
  return true;
}
