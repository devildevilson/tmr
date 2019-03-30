/* --------------------------------------------------------------------------- */
// Command Manager
// Этот класс содержит в себе все консольные команды. Чтобы выполнить команду
// необходимо вызвать execute(). Чтобы добавить команду - addCommand().
// Для того чтобы добавить новый класс, который может иметь консольные команды   }
// необходимо добавить в структуру Command указатель, добавить новую ф-цию       }
// addCommand() и добавить вызов в execute().                                    } устаревшая информация
// ВСЕ ВЫЗЫВАЕМЫЕ ТАКИМ ОБРАЗОМ Ф-ЦИИ ДОЛЖНЫ ВОЗВРАЩАТЬ И ПРИНИМАТЬ void         }
//
// Теперь я использую std function
/* --------------------------------------------------------------------------- */

#ifndef CMDMGR_H
#define CMDMGR_H

class CmdMgr;

#include <functional>
#include <unordered_map>
#include <vector>

std::vector<std::string> splitCommand(const std::string &command, const char &sep = ' ');

struct Command {
  std::string name;
  uint32_t    flags = 0; // не знаю, будут ли использоваться
  std::function<void()> f;
};

class CmdMgr {
public:
              CmdMgr();
              ~CmdMgr();
  void        init();
  void        addCommand(const std::string &name, const std::function<void()> &f);
  void        addAlias(const std::string &commandName, const std::string &alias);
  void        execute(const std::string &command);
  std::string getArg(const uint32_t &i);

  void help_f();
  void addAlias_f();
private:
  Command* findCommand(const std::string &name);
  std::vector<std::string> currentCommand;
  std::unordered_map<std::string, Command> commands;
};

#endif
