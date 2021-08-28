/// identificador de la red wi-fi local
#define netWork_SSD  "" // hola mundo //consolelog
// contraseña de la red wi-fi local
#define netWork_SSD_password  "" // 484AC7C3B2671DA0 // piercethevie

// variables globales para la pantalla OLED

#define screen_width 128 // ancho de la pantalla OLED

#define screen_heighyt 64 // alto de la pantalla OLED

#define oled_reset_pin 4 // pin para resetear la pantalla OLED

#define brokerSececreto ""
#define LED_RED 7 // red rojo 
#define LED_YELLOW 6 // red amarillo
#define LED_GREEN 4 // led verde
#define CHIP_SELECT 4 // pin de la memoria SD
#define SENSOR_1 A3 // sensor CT #1
#define RELAY_PIN1 3 // CONEXION PARA EL CT 1
#define mvVerAmp 66 // usar 100 para sensores CT DE 20A, 66 para sensores CT de 30A
#define DEVICE_READING ""


const char awsCertificate[] =  R""
/*Esta variable es para habilitar el Log del Serial Arduino*/
#define DEBUG true
/*El voltaje del hogar del usuario*/
#define home_voltage 220.0
#define device_name "ArduinoDevice01" // NOMBRE DEL DISPOSITIVO
#define user_Name "" // USUARIO ADMINISTRADOR
//colores de pantallas
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
//pines de pantallas LCD
#define TFT_DC 7 //PIN TFT DC DE LA PANTALLA LCD ""2.8
#define TFT_CS 6 //pin tft de la pantalla LCD ""2.8
#define libName "Proyecto final de tesis -> Claudio Brito MATRICULA 16-1769" // Nombre del proyecto
#define testFrequency 60
#define relayOnePin 3 //PIN RELE #1
#define relayTwoPin 2 //PIN RELE #2
#define relayThreePin 1 //PIN RELE #3
#define printPeriod 1000
