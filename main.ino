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
String messages[10] = { "1. Запустите Serial Bluetooth Terminal", "2. Подключитесь к E-va и отправьте .bin", "Составить сообщения в eva-card.ru", "4. Наслаждайтесь!" };
int messageCount = 4; // Количество сохраненных сообщений
int currentMessageIndex = 0; // Индекс текущего сообщения

// Переменные для работы с кнопкой
bool lastTouchState = LOW;
unsigned long pressStartTime = 0;
bool autoScrollActive = false;
unsigned long lastScrollTime = 0;
const unsigned long autoScrollInterval = 10000; // Интервал автоперелистывания 10 сек
const unsigned long longPressDuration = 3000; // Длительность удержания для активации автоперелистывания 3 сек

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);

  pinMode(touchPin, INPUT); // Настраиваем пин кнопки как вход
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  oled.init();
  oled.clear();
  displaySplashScreen();
  displayStaticText(messages[currentMessageIndex]);
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

  handleTouchButton();

  if (autoScrollActive && millis() - lastScrollTime >= autoScrollInterval) {
    lastScrollTime = millis();
    changeMessage();
  }
}

void handleTouchButton() {
  bool touchState = digitalRead(touchPin);

  if (touchState == HIGH && lastTouchState == LOW) {
    // Кнопка нажата
    pressStartTime = millis();
  } else if (touchState == LOW && lastTouchState == HIGH) {
    // Кнопка отпущена
    if (millis() - pressStartTime < longPressDuration) {
      // Короткое нажатие
      if (!autoScrollActive) {
        changeMessage();
      }
    } else {
      // Длинное нажатие: переключение режима
      autoScrollActive = !autoScrollActive;
      blinkRGB();
      lastScrollTime = millis();
    }
  }

  lastTouchState = touchState;
}

void handleCommand(String command) {
  if (command.startsWith("T")) {
    String textToDisplay = command.substring(1); // Убираем "T"
    if (messageCount < 10) { // Ограничение на 10 сообщений
      messages[messageCount++] = textToDisplay; // Добавляем сообщение в массив
    }
    currentMessageIndex = 0; // Сбрасываем индекс на начало
    displayStaticText(messages[currentMessageIndex]);
    Serial.println("Message added: " + textToDisplay);
    return;
  }
}

void processFileData() {
  String message = "";
  messageCount = 0; // Очистка старых сообщений

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
  currentMessageIndex = 0; // Сбрасываем индекс на начало
  displayStaticText(messages[currentMessageIndex]);
}

void changeMessage() {
  if (messageCount == 0) {
    return; // Если сообщений нет, ничего не делаем
  }
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

void displaySplashScreen() {
  oled.setScale(3);
  oled.setCursor(0, 2);
  oled.print("E-va! 2");
  oled.update();
  delay(3000);
  oled.clear();
}

void blinkRGB() {
  for (int i = 0; i < 3; i++) {
    analogWrite(redPin, 255);
    analogWrite(greenPin, 255);
    analogWrite(bluePin, 255);
    delay(200);
    analogWrite(redPin, 0);
    analogWrite(greenPin, 0);
    analogWrite(bluePin, 0);
    delay(200);
  }
}
