/// identificador de la red wi-fi local
#define netWork_SSD  "Hola mundo" // hola mundo //consolelog
// contraseña de la red wi-fi local
#define netWork_SSD_password  "claudio10" // 484AC7C3B2671DA0 // piercethevie

// variables globales para la pantalla OLED

#define screen_width 128 // ancho de la pantalla OLED

#define screen_heighyt 64 // alto de la pantalla OLED

#define oled_reset_pin 4 // pin para resetear la pantalla OLED

#define brokerSececreto "a3grg8s0qkek3y-ats.iot.us-west-2.amazonaws.com"
#define LED_RED 7 // red rojo 
#define LED_YELLOW 6 // red amarillo
#define LED_GREEN 4 // led verde
#define CHIP_SELECT 4 // pin de la memoria SD
#define SENSOR_1 A3 // sensor CT #1
#define RELAY_PIN1 3 // CONEXION PARA EL CT 1
#define mvVerAmp 66 // usar 100 para sensores CT DE 20A, 66 para sensores CT de 30A
#define DEVICE_READING "deviceReadings"


const char awsCertificate[] =  R"(
-----BEGIN CERTIFICATE-----
MIIC0jCCAbqgAwIBAgIVAO84pJ/GpWXw6lWyHPZSNNW8lkKeMA0GCSqGSIb3DQEB
CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t
IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yMTAzMjUwMjE4
MzVaFw00OTEyMzEyMzU5NTlaMGExCzAJBgNVBAYTAkRPMRYwFAYDVQQIEw1TYW50
byBEb21pbmdvMRYwFAYDVQQHEw1TYW50byBEb21pbmdvMQ4wDAYDVQQKEwVUZXNp
czESMBAGA1UEAxMJTXlNS1IxMDAwMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE
LYOW0Hef3ECy2aLWXdaX8z5YD2ewZzH4ZgAd66u7EK0wuh6R6fts5wGiSQmqaOva
E8I6YQdSRPARkVO5BWu3t6NgMF4wHwYDVR0jBBgwFoAU0mbHgFhry38ha07uoLqL
l0JAiVowHQYDVR0OBBYEFDINjPxy17AoIli4LEv9hQ0bCkQfMAwGA1UdEwEB/wQC
MAAwDgYDVR0PAQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQBDVGiCatdt702q
Vc6DHM/Z9/aRQ44Tu9K5GnG/lgBuqgKfiiF1ykSMqXxmHEEfx6NNtYo35YPPRyXq
4AyTwof9Z55cgkiZe/I6dl/4nOzGHPNuShXcIvqQvbfJHyeYb6mPU1yX89cbmQUc
uoGH0WNXNroS21ZQdMDu90rEA9ftGEcgyy1BitUhYxYL4IFvidYK47Q64JlLjK5m
a79GNGXc1++7wvOsX21Z+jRdWgau6iaLgBc3wemvWBFRmba7B3KzmEFFza8Xrrnc
aLa0nm68wrb3rUFrfw7uftlV0yE3K+K6CjTG7VR2Aobae1OO99AfDgHtpqfV7bEm
3Ukt5Qnj
-----END CERTIFICATE----
)";
/*Esta variable es para habilitar el Log del Serial Arduino*/
#define DEBUG true
/*El voltaje del hogar del usuario*/
#define home_voltage 220.0
#define device_name "ArduinoDevice01" // NOMBRE DEL DISPOSITIVO
#define user_Name "claudioraulmercedes@gmail.com" // USUARIO 
