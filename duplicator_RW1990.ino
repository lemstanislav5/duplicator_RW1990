// Для BluetoothSerial --------------------------
#include "BluetoothSerial.h"
String device_name = "BTMonitor";
// Check if Bluetooth is available
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
// Check Serial Port Profile
#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Port Profile for Bluetooth is not available or not enabled. It is only available for the ESP32 chip.
#endif
BluetoothSerial SerialBT;
// ----------------------------------------------

// Для OneWire ----------------------------------
#include <OneWire.h>                                
#define pin 13                                    // D10: пин данных (центральный) для подлючения лузы iButton (зелёный провод у лузы DS9092)
OneWire ibutton(pin);                  

// ----------------------------------------------

// СОЗДАЕМ КЛАСС И ДАЛЕЕ РАБОТАЕМ С ЕГО ОБЪЕКТОМ, В КЛАСС СПРЯМЕМ ВСЕ ОБРАБОТЧИКИ ДАННЫХ

byte addr[8];
byte ReadID[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }; // Массив байт с нулевыми значениями

const int buttonPin = 2;                          // Назначаем пин D2 на кнопку
int buttonState = 0;                              // Переменная, в которой хранится состояние кнопки
int writeflag = 0;                                // Не пишем содержимое массива readID в таблетку, когда флаг равен 0
int readflag = 0;                                 // Вытаскиваем из таблетки номер в массив readID, если этот флаг равен 0

void setup() {  
  pinMode(buttonPin, OUTPUT);
  Serial.begin(115200);                           // Инициализируем монитор порта на скорости 9600
  SerialBT.begin(device_name);                    // Bluetooth device name
  Serial.println("Инициализация выполнена");      // Отправляем тестовую строку в монитор порта
}

void readKey() {
  Serial.println("Считан ключ");
  delay(50);
  Serial.print("Read: ");
  for (byte x = 0; x < 8; x++) {                  // ...и начинаем выводить на экран и в монитор порта что считали из ключа
    if (readflag == 0) {                          // Пишем байт в массив ReadID, если не стоит флаг запрета
      ReadID[x] = (addr[x]);
    }
    Serial.print(addr[x], HEX);                   // Пишем байт в порт
    Serial.print(":");                            // Пишем в порт разделитель байтов, если байт не последний
  }
  Serial.println();                              // Красивости: отправляем пустую строку-разделитель в монитор порта, когда закончили с выводом считанного байта.
  sendKey(ReadID);
}

void readyToWriteKey() {
  Serial.println(buttonState);
  Serial.println ("Режим записи");
  readflag = 1;                                   // Ставим флаг, чтобы не читать из таблетки в массив ReadID перед записью
  writeflag = 1;                                  // Ставим флаг, чтобы писать в таблетку содержимое масива readID
                                                  
  Serial.print("Записан: ");                      // отчитываемся в монитор порта о том, какой номер будем записывать в ключ
  for (byte x = 0; x < 8; x++) {
    Serial.print(ReadID[x], HEX);
    Serial.print(':');
  }
    Serial.println();  
    delay(250);
}

void theKeyIsWrittenDown() {
    Serial.println(writeflag);  
    ibutton.skip(); 
    ibutton.reset(); 
    ibutton.write(0x33);
  
    Serial.print("Предидущий ключ: ");              // пишем в порт что на ключе было до записи
    for (byte x = 0; x < 8; x++) {
      Serial.print(ibutton.read(), HEX);            // Считываем и побайтно в цикле пишем в порт текущий ключ
      Serial.print(':');     
    }
    Serial.println();
    
    Serial.println ("-----------");
        
    // send reset
    ibutton.skip();
    ibutton.reset();
    // send 0xD1
    ibutton.write(0xD1);
    // send logical 0
    digitalWrite(pin, LOW); pinMode(pin, OUTPUT); delayMicroseconds(60);
    pinMode(pin, INPUT); digitalWrite(pin, HIGH); delay(10);
    byte newID[8] = { (ReadID[0]), (ReadID[1]), (ReadID[2]), (ReadID[3]), (ReadID[4]), (ReadID[5]), (ReadID[6]), (ReadID[7]) };
    ibutton.skip();
    ibutton.reset();
    ibutton.write(0xD5);
    Serial.println();
    Serial.print("Начало записи: ");                // Отчитываемся в порт о начале записи ключа
    for (byte x = 0; x < 8; x++) {
      writeByte(newID[x]);                          // В цикле вызываем функцию побайтовой записи ключа
      Serial.print(newID[x], HEX);                  // Параллельно с таблеткой пишем байты в порт
      Serial.print(':');                            // Пишем в порт разделитель
    }
    Serial.println();
    
    ibutton.reset();
    // send 0xD1
    ibutton.write(0xD1);
    //send logical 1
    digitalWrite(pin, LOW); pinMode(pin, OUTPUT); delayMicroseconds(10);
    pinMode(pin, INPUT); digitalWrite(pin, HIGH); delay(10);
    writeflag = 0;                                  // Сбрасываем флаги
    readflag = 0;
    Serial.println ("Запись ключа закончена.");
    SerialBT.print("&");                            // Отправляем сигнал # о готовности записи
    delay(2000);
}

void sendKey(byte *arr){                           // Функци отправки данных в формате char 1:211:200:167:1:0:0:56:
  Serial.println("Начало отправки: ");
  String key = "[";
  for (byte i = 0; i < 8; i++) {
    key += String((int) arr[i]);                                                    
    if(i < 7) key += ",";                            
  }
  key += "]";
  SerialBT.print(key);
}

String json = "";


// char **pointer, *stringVar;
//   stringVar = "b9";
//   byte Var = strtol(stringVar,pointer,16);

void convertWriteKeyToReadID()
{
  int count = 0;
  byte arr[8];
  String item = "";
  int size = sizeof(ReadID) / sizeof(ReadID[0]);
  char **pointer;
  Serial.println(size);
  for (int i = 0; i < json.length(); i++){ 
      if(json[i] == ','){
        byte num = strtol(item.c_str(),pointer,16);
        ReadID[count] = num;
        count++;
        item = "";
      } else {
        item += json[i];
      }
  }
  readflag = 1;                                   // Ставим флаг, чтобы не читать из таблетки в массив ReadID перед записью
  writeflag = 1;                                  // Ставим флаг, чтобы писать в таблетку содержимое масива readID
  SerialBT.print("#");                            // Отправляем сигнал # о готовности записи
}

void loop() {
  // Для Bluetooth HC05 ------------------------------------------------
  if (SerialBT.available()) {
    char c = SerialBT.read();                       // читаем из software-порта
    json += String(c);
  }
  
  // Проверяем наличие во входящих данных ковычек [] 
  if(json !="" && json.length() > 0) {
    if(json.indexOf("[") > -1 && json.indexOf("]") > -1){
      int length = json.length();
      json.remove(json.indexOf("["), 1);
      json.remove(json.indexOf("]"), 1);
      json += ",";
      Serial.println(json);
      convertWriteKeyToReadID();
      json = "";
    }
  }
  // if (Serial.available()) {
  //   char c = Serial.read();                         // читаем из hardware-порта
  //   SerialBT.write(c);                              // пишем в software-порт
  // }
  // Для Bluetooth HC05 ------------------------------------------------


  // buttonState = digitalRead(buttonPin);          // Читаем состояние кнопки
  if (buttonState == HIGH) readyToWriteKey();       // Если кнопка нажата, то приступаем к записи из массива в таблетку

  if (!ibutton.search(addr)) {                      // Проверка: а может таблетка не обнаружена?
    ibutton.reset_search();
    delay(50);
    return;                                         // если нет таблетки, то выходим из loop
  }

  if(readflag != 1) readKey();                                        // функция постоянного чтения

  if (writeflag == 1) theKeyIsWrittenDown();        // Если установлен флаг на запись в таблетку
}

int writeByte(byte data) {                          // собственно, функция записи байта в таблетку
  int data_bit;
    for (data_bit = 0; data_bit < 8; data_bit++) {  // влетает байт, пишем его в ключ побитово
    if (data & 1) {
      digitalWrite(pin, LOW); pinMode(pin, OUTPUT);
      delayMicroseconds(60);
      pinMode(pin, INPUT); digitalWrite(pin, HIGH);
      delay(10);
    } else {
      digitalWrite(pin, LOW); pinMode(pin, OUTPUT);
      pinMode(pin, INPUT); digitalWrite(pin, HIGH);
      delay(10);
    }
    data = data >> 1;                               // Битшифт или побитовый сдвиг вправо
  }
  return 0;
}
