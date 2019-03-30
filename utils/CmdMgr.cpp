#include "CmdMgr.h"

#include "Globals.h"

std::vector<std::string> splitCommand(const std::string &command, const char &sep) {
  std::vector<std::string> tokens;
  size_t start = command.find_first_not_of(sep), end = 0;

  while ((end = command.find_first_of(sep, start)) != std::string::npos) {
    tokens.push_back(command.substr(start, end - start));
    start = command.find_first_not_of(sep, end);
  }

  if(start != std::string::npos) tokens.push_back(command.substr(start));

  return tokens;
}

CmdMgr::CmdMgr() {}

CmdMgr::~CmdMgr() {
  //std::cout << "CmdMgr destruction" << std::endl;
  currentCommand.clear();
  commands.clear();
}

void CmdMgr::init() {
  // здесь что-то должно быть еще

  addCommand("help", std::bind(&CmdMgr::help_f, this));
  addAlias("help", "HELP");
  addAlias("help", "commands");
  addCommand("alias", std::bind(&CmdMgr::addAlias_f, this));
}

void CmdMgr::addCommand(const std::string &name, const std::function<void()> &f) {
  Command* cmd = findCommand(name);
  if (cmd) {
    Global::console()->printf("Command %s already exist\n", name.c_str());
    return;
  }

  Command tmp = {};
  tmp.name = name;
  tmp.f = f;
  //commands.push_back(tmp);
  commands[name] = tmp;
}

void CmdMgr::addAlias(const std::string &commandName, const std::string &alias) {
  Command* cmd = findCommand(commandName);
  if (cmd == nullptr) {
    Global::console()->printf("%s command not found\n", commandName.c_str());
    return;
  }

  Command* aliasCmd = findCommand(alias);
  if (aliasCmd) {
    Global::console()->printf("Command %s already exist\n", alias.c_str());
    return;
  }

  Command tmp = {};
  tmp.name = alias;
  tmp.f = cmd->f;
  //commands.push_back(tmp);
  commands[alias] = tmp;
}

void CmdMgr::execute(const std::string &command) {
  // как ко мне будет приходить команда? "] " + command или просто command
  Global::console()->print("] " + command);
  currentCommand = splitCommand(command);

  Global::console()->addToHistory(command);

  Command* tmp = findCommand(currentCommand[0]);
  if (tmp == nullptr) {
    Global::console()->printf("%s command not found", currentCommand[0].c_str());
    return;
  }

  // дальше идет вызов необходимой функции
  // сначала мы превращаем void* в необходимый класс (obj = (Game*)tmp->data;)
  // затем вызываем ф-цию класса (obj->fG();)
  // можно ли как-то переделать? static?

  // есть же std function!
  tmp->f();
}

std::string CmdMgr::getArg(const uint32_t &i) {
  if (i >= currentCommand.size()) {
    return "";
  }

  return currentCommand[i];
}

void CmdMgr::help_f() {
  Global::console()->print("Available commands: ");
//   for (size_t i = 0; i < commands.size(); i = i + 3) {
//     if (i + 2 < commands.size()) {
//       Global::console()->printf("%s %s %s", commands[i].name.c_str(), commands[i+1].name.c_str(), commands[i+2].name.c_str());
//     } else if (i + 1 < commands.size()) {
//       Global::console()->printf("%s %s",    commands[i].name.c_str(), commands[i+1].name.c_str());
//     } else {
//       Global::console()->printf("%s",       commands[i].name.c_str());
//     }
//   }

  for (auto itr = commands.begin(); itr != commands.end(); itr++) {
    Global::console()->print(itr->second.name);
  }
}

void CmdMgr::addAlias_f() {
  std::string commandName = getArg(1);
  if (commandName.empty()) {
    Global::console()->print("Bind another name to the command\n");
    Global::console()->print("Usage: alias COMMANDNAME ALIASNAME");
    return;
  }

  std::string alias = getArg(2);
  if (alias.empty()) {
    Global::console()->print("New command name is missed\n");
    return;
  }

  addAlias(commandName, alias);
}

Command* CmdMgr::findCommand(const std::string &name) {
  auto itr = commands.find(name);
  if (itr == commands.end()) return nullptr;

  return &(itr->second);
}
