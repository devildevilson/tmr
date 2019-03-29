#include "Console.h"

std::string trim(const std::string& str) {
  size_t first = str.find_first_not_of(' ');
  if (std::string::npos == first) return str;
  size_t last = str.find_last_not_of(' ');
  return str.substr(first, (last - first + 1));
}

Console::Console() {}

Console::~Console() {
  //std::cout << "Console destruction" << std::endl;
//   Notify* tmp = notifyRoot;
//   while (notifyRoot) {
//     tmp = tmp->next;
//     delete notifyRoot;
//     notifyRoot = tmp;
//   }
  
  history.clear();
}

// void Console::init() {
//   clearCommandBuffer();
//   
//   // здесь будет регистрация переменных и добавление консольных команд
//   // а также загрузка заднего фона (если будет)
//   // вычисление высоты и ширины линий
// }
// 
// void Console::setConsoleLines(uint32_t widht, uint32_t height) {
//   this->widht = widht;
//   this->height = height/2;
//   lineWidht = widht - 16;
//   conLinesCount = this->height/lineHeight;
//   if (history.size() < conLinesCount) conLinesCount = history.size();
// }
// 
// TextData Console::getNextConsoleLine(bool input, uint32_t deltaTime) {
//   // СНАЧАЛА РИСУЕМ БЕКГРАУНД (бекграунд рисуем не здесь)
//   
//   // ЗАТЕМ НАЧИНАЕМ РИСОВАТЬ СИМВОЛЫ С КОНЦА
//   uint16_t line = conLinesCount - currentLine;
//   TextData data = {};
//   if (currentLine == 0) { // 1. версия и какая-нибудь доп инфа
//     data.text = "testing app v. 0.1.3";
//     data.x = 8 + lineWidht;
//     // ЗДЕСЬ МОЖЕТ БЫТЬ ОШИБКА
//     data.y = height - lineHeight * line;
//     data.align = TextAlign::RIGHT;
//   } else if (currentLine == 1) { // 2. линия ввода (] 123# что-то такое)
//     if (input) {
//       data = drawInput(height - lineHeight * line, deltaTime);
//     } else {
//       currentLine++;
//     }
//   } else { // 3. история сообщений консоли
//     data.text = history[currentLine-2];
//     data.x = 8;
//     data.y = height - lineHeight * line;
//     data.align = TextAlign::LEFT;
//   }
//   
//   currentLine++;
//   
//   return data;
// }
// 
// bool Console::isNextLineVisible() {
//   return currentLine <= conLinesCount;
// }
// 
// void Console::resetCurrentConsoleLine() {
//   currentLine = 0;
// }
// 
// 
// TextData Console::getNextNotify(uint32_t deltaTime) {
//   TextData data = {};
//   float tmpF = 0.0f; // чтобы не смешивать разные нотификации 
//   
//   data.text = currentNotify->string;
//   if (currentNotify->x < 0.0001) data.x = 8;
//   else data.x = currentNotify->x;
//   if (currentNotify->y < 0.0001) {
//     tmpF = tmpF + 8;
//     data.y = tmpF;
//   } else data.y = currentNotify->y;
//   data.align = currentNotify->align;
//   
//   bool deleted = false;
//   currentNotify->time = currentNotify->time + deltaTime;
//   if (currentNotify->time > NOTIFY_TIME) {
//     Notify* tmp;
//     tmp = currentNotify->next;
//     delete currentNotify;
//     currentNotify = tmp;
//     if (prev) prev->next = currentNotify;
//     deleted = true;
//   }
//   
//   if (!deleted) {
//     prev = currentNotify;
//     if (currentNotify) currentNotify = currentNotify->next;
//   }
//   
//   return data;
// }
// 
// bool Console::isNextNotifyExist() {
//   if (currentNotify->next) {
//     return true;
//   }
//   
//   return false;
// }
// 
// void Console::resetNotifyPointer() {
//   currentNotify = notifyRoot;
//   prev = nullptr;
// }

// void Console::drawConsole(int lines, bool input, uint32_t deltaTime, const char userInput) {
//   // СНАЧАЛА РИСУЕМ БЕКГРАУНД (бекграунд рисуем не здесь)
//   
//   // ЗАТЕМ НАЧИНАЕМ РИСОВАТЬ СИМВОЛЫ С КОНЦА
//   
//   // 1. версия и какая-нибудь доп инфа
//   // будет ли у нас динамическое окно?
//   TextData data = {};
//   data.text = "testing app v. 0.1.3";
//   data.x = lineWidht;
//   data.y = height - lineHeight * lines;
//   data.align = TextAlign::RIGHT;
//   
//   //uint32_t currentLine = height - lineHeight * lines;
//   
//   app->addText(data);
//   
//   // 2. линия ввода (] 123# что-то такое)
//   if (input) {
//     data.y = data.y - 8;
//     drawInput(data.y, deltaTime, userInput);
//   }
//   
//   // 3. история сообщений консоли
//   //data.y = data.y - 8;
//   data.align = TextAlign::LEFT;
//   for (long int i = history.size()-1; i >= 0; i--) {
//     data.text = history[i];
//     data.y = data.y - 8;
//     
//     app->addText(data);
//   }
// }
// 
// void Console::drawNotify(uint32_t deltaTime) {
//   // проверяем осталось ли чтонибудь в истории нотификации
//   if (notifyRoot == nullptr) {
//     return;
//   } else {
//     Notify* prev = nullptr;
//     Notify* current = notifyRoot;
//     TextData data = {};
//     float tmpF = 0.0f; // чтобы не смешивать разные нотификации 
//     while (current) {
//       data.text = current->string;
//       if (current->x < 0.0001) data.x = 8;
//       else data.x = current->x;
//       if (current->y < 0.0001) {
//         tmpF = tmpF + 8;
//         data.y = tmpF;
//       } else data.y = current->y;
//       data.align = current->align;
//       app->addText(data);
//       
//       current->time = current->time + deltaTime;
//       if (current->time > NOTIFY_TIME) {
//         Notify* tmp;
//         tmp = current->next;
//         delete current;
//         current = tmp;
//         if (prev) prev->next = current;
//       }
//       
//       prev = current;
//       if (current) current = current->next;
//     }
//   }
// }

// void Console::printNotify(std::string s, float x, float y, TextAlign align) {
//   Notify* newNotify = new Notify();
//   newNotify->string = s;
//   newNotify->x = x;
//   newNotify->y = y;
//   newNotify->align = align;
//   
//   Notify* tmp = notifyRoot;
//   while (tmp->next) {
//     tmp = tmp->next;
//   }
//   
//   tmp->next = newNotify;
// }


void Console::print(const std::stringstream &ss) {
  print(ss.str());
}

void Console::print(const std::string &s) {
  ConsoleLine cl = {};
  cl.type = INFO;
  cl.string = s;
  cl.string = trim(cl.string);
  items.push_back(cl);
  //if (history.back().at(history.back().length()-1) == '\n') history.back().erase(history.back().length()-1);
  fmt::print(cl.string);
  if (cl.string.at(cl.string.length()-1) != '\n') fmt::print("\n");
}

void Console::printW(const std::stringstream &ss) {
  printW(ss.str());
}

void Console::printW(const std::string &s) {
  ConsoleLine cl = {};
  cl.type = WARNING;
  cl.string = s;
  cl.string = trim(cl.string);
  items.push_back(cl);
  //if (history.back().at(history.back().length()-1) == '\n') history.back().erase(history.back().length()-1);
  fmt::print(cl.string);
  if (cl.string.at(cl.string.length()-1) != '\n') fmt::print("\n");
}

void Console::printE(const std::stringstream &ss) {
  printE(ss.str());
}

void Console::printE(const std::string &s) {
  ConsoleLine cl = {};
  cl.type = ERROR;
  cl.string = s;
  cl.string = trim(cl.string);
  items.push_back(cl);
  //if (history.back().at(history.back().length()-1) == '\n') history.back().erase(history.back().length()-1);
  fmt::print(cl.string);
  if (cl.string.at(cl.string.length()-1) != '\n') fmt::print("\n");
}

// сюда будут выводиться MOTD и игровые сообщения
// void Console::centerPrintf(float textHeight, const char* formatMessage, ...) {
//   va_list argptr;
//   char *buffer = nullptr; 
//   uint32_t size = 4096; // возможная длина (в какой-то момент нужно увеличить буфер в TextRender)
//   va_start(argptr, formatMessage);
//   vsnprintf(buffer, size, formatMessage, argptr);
//   va_end(argptr);
//   
//   // ПЕРЕПИСАТЬ
//   uint32_t count = 0;
//   char c = buffer[0];
//   while (c != '\0') {
//     if (c == '\n') {
//       count++;
//     }
//     
//     buffer++;
//     c = *buffer;
//   }
//   
//   textHeight = textHeight - (count/2)*lineHeight;
//   std::string buf = buffer;
//   int j = 0;
//   size_t oldPoint = buf.length();
//   
//   for (long int i = buf.length()-1; i >= 0; i--) {
//     if (buf.at(i) == '\n') {
//       printNotify(buf.substr(i, oldPoint), widht/2, textHeight+lineHeight*j, TextAlign::CENTER);
//       j++;
//       oldPoint = i;
//     }
//   }
//   
//   std::cout << buffer;
// }

void Console::addToHistory(const std::string &s) {
  history.push_back(s);
}

void Console::logWriteToFile() {
  FileTools::createLogFile(items);
}

std::vector<ConsoleLine> Console::getHistory() {
  return items;
}

size_t Console::getHistorySize() {
  return items.size();
}

// TextData Console::drawInput(float inputHeight, uint32_t deltaTime) {
//   static uint32_t time = 0;
//   
//   //commandBuffer = commandBuffer + userInput;
//   time = time + deltaTime;
//   if (time > INPUT_CURSOR_TIME) {
//     time = 0;
//     inputCursor = !inputCursor;
//   }
//   
//   TextData data = {};
//   if (inputCursor) {
//     data.text = commandBuffer + '#';
//     //textRender->addText(commandBuffer + '#', 8, inputHeight, TextAlign::LEFT);
//   } else {
//     data.text = commandBuffer;
//     //textRender->addText(commandBuffer, 8, inputHeight, TextAlign::LEFT);
//   }
//   data.x = 8;
//   data.y = inputHeight;
//   data.align = TextAlign::LEFT;
//   
//   return data;
// }
// 
// void Console::clearCommandBuffer() {
//   commandBuffer = "] ";
// }

