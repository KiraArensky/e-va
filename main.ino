#include <GyverOLED.h>
#include <Wire.h>
#include <SoftwareSerial.h> // Для работы с Bluetooth

// Инициализация OLED-дисплея
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;

// Пины для сенсорной кнопки и RGB-светодиода
const int touchPin = 5; // Пин для сенсорной кнопки
const int redPin = 9;
const int greenPin = 10;
const int bluePin = 11;

// Настройка Bluetooth (пины RX и TX для модуля HC-05/HC-06)
SoftwareSerial bluetooth(2, 3); // RX, TX

// Буфер для хранения сообщений
#define BUFFER_SIZE 256
char fileBuffer[BUFFER_SIZE];
int bufferIndex = 0;
bool receivingFile = false;

// Массив для хранения текстовых сообщений
String messages[10]; // Массив на 10 сообщений
int messageCount = 0; // Количество сохраненных сообщений
int currentMessageIndex = 0; // Индекс текущего сообщения

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);

  pinMode(touchPin, INPUT); // Настраиваем пин кнопки как вход
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  oled.init();
  oled.clear();
  oled.setScale(3); // Увеличенный размер шрифта
  oled.setCursor(0, 2.5);
  oled.print("E-va! 2");
  oled.update();
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    handleCommand(command);
  }

  while (bluetooth.available()) {
    char received = bluetooth.read();
    if (received == '<') {
      receivingFile = true;
      bufferIndex = 0;
      continue;
    }
    if (received == '>') {
      receivingFile = false;
      fileBuffer[bufferIndex] = '\0';
      processFileData();
      continue;
    }
    if (receivingFile && bufferIndex < BUFFER_SIZE - 1) {
      fileBuffer[bufferIndex++] = received;
    }
  }

  // Проверяем нажатие сенсорной кнопки
  if (digitalRead(touchPin) == HIGH) {
    changeMessage(); // Переход к следующему сообщению
    delay(100); // Задержка для устранения дребезга
  }
}

void handleCommand(String command) {
  if (command.startsWith("T")) {
    String textToDisplay = command.substring(1); // Убираем "T"
    if (messageCount < 10) { // Ограничение на 10 сообщений
      messages[messageCount++] = textToDisplay; // Добавляем сообщение в массив
    }
    displayStaticText(messages[currentMessageIndex]);
    Serial.println("Message added: " + textToDisplay);
    return;
  }

  char color = command.charAt(0);
  int value = command.substring(1).toInt();
  String colorName;

  switch (color) {
    oled.setScale(2); // Увеличенный размер шрифта
    case 'R':
      analogWrite(redPin, value);
      colorName = "Red";
      break;
    case 'G':
      analogWrite(greenPin, value);
      colorName = "Green";
      break;
    case 'B':
      analogWrite(bluePin, value);
      colorName = "Blue";
      break;
    default:
      colorName = "Unknown";
      break;
  }

  oled.clear();
  oled.setScale(2); // Увеличенный размер шрифта
  oled.setCursor(0, 0);
  oled.print("Color:");
  oled.setCursor(0, 2);
  oled.print(colorName);
  oled.update();
}

void processFileData() {
  String message = "";
  for (int i = 0; i < bufferIndex; i++) {
    if (fileBuffer[i] == '\n') {
      if (messageCount < 10) { // Ограничение на 10 сообщений
        messages[messageCount++] = message;
      }
      message = "";
    } else {
      message += fileBuffer[i];
    }
  }
  if (message.length() > 0 && messageCount < 10) {
    messages[messageCount++] = message;
  }
  displayStaticText(messages[currentMessageIndex]);
}

void changeMessage() {
  if (messageCount == 0){
    
  }; // Если сообщений нет, ничего не делаем
  currentMessageIndex = (currentMessageIndex + 1) % messageCount; // Переход к следующему сообщению
  displayStaticText(messages[currentMessageIndex]);
}

void displayStaticText(String text) {
  oled.clear();

  const int maxCharsPerLine = 10; // Максимум символов на строку
  const int maxLines = 4;         // Количество строк на экране
  int line = 0;
  int cursorY = 0;

  for (int i = 0; i < text.length(); i += maxCharsPerLine) {
    if (line >= maxLines) break; // Ограничиваем количество строк на экране

    String lineText = text.substring(i, i + maxCharsPerLine);
    oled.setScale(2); // Увеличенный размер шрифта
    oled.setCursor(0, cursorY); // Устанавливаем позицию для строки
    oled.print(lineText);
    cursorY += 2.6; // Переход на следующую строку
    line++;
  }

  oled.update();
}