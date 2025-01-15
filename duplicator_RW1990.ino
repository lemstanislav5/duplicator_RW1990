#include <OneWire.h>                                // Библиотека
// Для Bluetooth HC05 ------------------------------------------------
#define pin 10                                      // D10: пин данных (центральный) для подлючения лузы iButton (зелёный провод у лузы DS9092)
OneWire ibutton(pin);                  
byte addr[8];
byte ReadID[8] = { 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x2F }; // "Универсальный" ключ. Прошивается если не приложить исходный ключ.
// -------------------------------------------------------------------
// Для Bluetooth HC05 ------------------------------------------------
#include <SoftwareSerial.h>
SoftwareSerial bluetooth(4, 5); // указываем пины rx и tx соответственно
// -------------------------------------------------------------------

const int buttonPin = 2;                          // Назначаем пин D2 на кнопку
int buttonState = 0;                              // Переменная, в которой хранится состояние кнопки
int writeflag = 0;                                // Не пишем содержимое массива readID в таблетку, когда флаг равен 0
int readflag = 0;                                 // Вытаскиваем из таблетки номер в массив readID, если этот флаг равен 0

void setup() {  
  pinMode(buttonPin, OUTPUT);
  Serial.begin(9600);                             // Инициализируем монитор порта на скорости 9600
  bluetooth.begin(9600);
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
  Serial.println ();                              // Красивости: отправляем пустую строку-разделитель в монитор порта, когда закончили с выводом считанного байта.
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
      Serial.print(ibutton.read(), HEX);              // Считываем и побайтно в цикле пишем в порт текущий ключ
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
      Serial.print(':');                   // Пишем в порт разделитель
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
    delay(2000);
}

void loop() {
  // Для Bluetooth HC05 ------------------------------------------------
  if (bluetooth.available()) {
    char c = bluetooth.read(); // читаем из software-порта
    Serial.print(c); // пишем в hardware-порт
    bluetooth.write('hello'); // пишем в software-порт
  }
  if (Serial.available()) {
    char c = Serial.read(); // читаем из hardware-порта
    bluetooth.write(c); // пишем в software-порт
  }
  // Для Bluetooth HC05 ------------------------------------------------


  buttonState = digitalRead(buttonPin);             // Читаем состояние кнопки
  if (buttonState == HIGH) readyToWriteKey();                         // Если кнопка нажата, то приступаем к записи из массива в таблетку

  if (!ibutton.search(addr)) {                     // Проверка: а может таблетка не обнаружена?
    ibutton.reset_search();
    delay(50);
    return;                                         // если нет таблетки, то выходим из loop
  }

  readKey(); // функция постоянного чтения

  if (writeflag == 1) theKeyIsWrittenDown(); // Если установлен флаг на запись в таблетку
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
    data = data >> 1;                                 // Битшифт или побитовый сдвиг вправо
  }
  return 0;
}
