#define _LCD_TYPE 1
#include <OneWire.h>                                // Библиотека
#include <OLED_I2C.h> 
OLED  myOLED(16, 2); // SDA - 8pin, SCL - 9pin
extern uint8_t RusFont[]; // Русский шрифт

#define pin 10                                      // D10: пин данных (центральный) для подлючения лузы iButton (зелёный провод у лузы DS9092)
OneWire ibutton(pin);                  
byte addr[8];
byte ReadID[8] = { 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x2F }; // "Универсальный" ключ. Прошивается если не приложить исходный ключ.

// Для Bluetooth HC05 ------------------------------------------------
#include <SoftwareSerial.h>
SoftwareSerial bluetooth(4, 5); // указываем пины rx и tx соответственно
// Для Bluetooth HC05 ------------------------------------------------
byte ReadID01[8] = {0x01, 0xBE, 0x40, 0x11, 0x00, 0x00, 0x00, 0x77}; // - проверен работает



const int buttonPin = 2;                            // Назначаем пин D2 на кнопку
int buttonState = 0;                                // Переменная, в которой хранится состояние кнопки
int writeflag = 0;                                  // Не пишем содержимое массива readID в таблетку, когда флаг равен 0
int readflag = 0;                                   // Вытаскиваем из таблетки номер в массив readID, если этот флаг равен 0




void setup() {  
  myOLED.begin();


                                    // Функция инициализации, выполняется однократно при запуске Ардуинки
  pinMode(buttonPin, OUTPUT);
  // lcd.init();                                       // Инициализируем дисплей 16x2
  // lcd.backlight();                                  // Включаем подсветку дисплея 16х2
  // lcd.setCursor(3,0);                               // Устанавливаем курсор в 4 символ 1 строки
  // lcd.print("ПPИCЛOHИTE");                          // Первая строка при включении
  // lcd.setCursor(6,1);                               // Устанавливаем курсор в 7 символ 2 строки
  // lcd.print("KЛЮЧ!");                               // Вторая строка при включении  
  Serial.begin(9600);                               // Инициализируем монитор порта на скорости 9600
  bluetooth.begin(9600);
  Serial.println("Initialization complete");        // Отправляем тестовую строку в монитор порта
}

void readKey() {
  delay(50);
  // lcd.clear();                                      // Очищаем экран и начинаем отображение информации о номере ключа
  // lcd.setCursor(0,0);
  // lcd.print("CЧИTAH: "); 
  Serial.print("Read: ");
  // lcd.setCursor(8,0);                               // Устанавливаем курсор на 9 символ первой строки...
  for (byte x = 0; x < 8; x++) {                    // ...и начинаем выводить на экран и в монитор порта что считали из ключа
    // if (x==3)  lcd.setCursor (0,1);                   // Если дошли до 4 байта, то переходим на вторую строку, т.к. в первой строке места больше нет
    if (readflag == 0) {                              // Пишем байт в массив ReadID, если не стоит флаг запрета
      ReadID[x] = (addr[x]);
    }
    Serial.print(addr[x], HEX);                       // Пишем байт в порт
    // lcd.print(addr[x], HEX);                          // Пишем байт на экран
    Serial.print(":");                              // Пишем в порт разделитель байтов, если байт не последний
    // lcd.print(":");                                 // Пишем на экран разделитель байтов, если байт не последний
  }
  Serial.println ();                                // Красивости: отправляем пустую строку-разделитель в монитор порта, когда закончили с выводом считанного байта.
}

void readyToWriteKey() {
  Serial.println(buttonState);
  Serial.println ("Write mode");
  readflag = 1;                                   // Ставим флаг, чтобы не читать из таблетки в массив ReadID перед записью
  writeflag = 1;                                  // Ставим флаг, чтобы писать в таблетку содержимое масива readID
  // lcd.clear();                                    // очищаем LCD и предлагаем приложить болванку
  // lcd.setCursor(3,0);
  // lcd.print("ПPИCЛOHИTE");
  // lcd.setCursor(4,1);
  // lcd.print("БOЛBAHKУ!");

  // отчитываемся в монитор порта о том, какой номер будем записывать в ключ
  Serial.print("Writing: ");
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
  
    Serial.print("Key ID before: ");          // пишем в порт что на ключе было до записи
    for (byte x = 0; x < 8; x++) {
      Serial.print(ibutton.read(), HEX);              // Считываем и побайтно в цикле пишем в порт текущий ключ
      Serial.print(':');     
    }
    Serial.println();
    
    // lcd.clear();                                    // Очищаем экран перед записью ключа
    // lcd.setCursor(4,0);
    // lcd.print("БOЛBAHKA");                          // Начало записи, интригуем пользователя и не даём оторвать таблетку
        
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
    Serial.print("Writing: ");                // Отчитываемся в порт о начале записи ключа
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
    // lcd.setCursor(4,1);
    // lcd.print("3AПИCAHA");                          // дописываем фразу для пользователя, намекая, что таблетку можно забирать
    delay(2000);
}

void loop() {
  myOLED.print("Heccrbq ihban", CENTER, 0); // Выводим надпись "Русский язык"



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
