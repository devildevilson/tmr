#include "Variable.h"

#include <cstdlib>
#include "Globals.h"

// Var1* Var1::findVar(const std::string &name) {
//   auto itr = vars.find(name);
//   if (itr == vars.end()) return nullptr;
//   
//   return itr->second;
// }
// 
// std::vector<Var1*> Var1::findVarsWithFlags(const uint32_t &flags) {
//   std::vector<Var1*> ret;
//   
//   for (const auto &pair : vars) {
//     if (pair.second->getFlags() & flags) ret.push_back(pair.second);
//   }
//   
//   return ret;
// }
// 
// float Var1::getValueF(const std::string &name) {
//   auto itr = vars.find(name);
//   if (itr == vars.end()) return 0.0f;
//   
//   return itr->second->getValueF();
// }
// 
// std::string Var1::getValue(const std::string &name) {
//   auto itr = vars.find(name);
//   if (itr == vars.end()) return "";
//   
//   return itr->second->getValue();
// }
// 
// bool Var1::reset(const std::string &name) {
//   auto itr = vars.find(name);
//   if (itr == vars.end()) return false;
//   
//   itr->second->setDefault();
//   
//   return true;
// }
// 
// bool Var1::set(const std::string &name, const float &valueF) {
//   auto itr = vars.find(name);
//   if (itr == vars.end()) return false;
//   
//   itr->second->set(valueF);
//   
//   return true;
// }
// 
// bool Var1::set(const std::string &name, const std::string &value) {
//   auto itr = vars.find(name);
//   if (itr == vars.end()) return false;
//   
//   itr->second->set(value);
//   
//   return true;
// }
// 
// Var1* Var1::add(const std::string &name, const std::string &defaultVal, const uint32_t &flags) {
//   auto itr = vars.find(name);
//   if (itr != vars.end()) return itr->second;
//   
//   Var1* ret = varPool.newElement(name, defaultVal, flags);
//   vars[name] = ret;
//   
//   return ret;
// }
// 
// void Var1::get_f() {
//   const std::string &varName = Global::commands()->getArg(1);
//   
//   if (varName.empty()) {
//     Global::console()->print("Usage: get VARNAME");
//     return;
//   }
//   
//   Var1* tmp = findVar(varName);
//   if (tmp == nullptr) {
//     Global::console()->printf("Could not find var %s", varName.c_str());
//     return;
//   }
//   
//   Global::console()->printf("%s equals %s\n", varName, tmp->value);
// }
// 
// void Var1::set_f() {
//   const std::string &varName = Global::commands()->getArg(1);
//   const std::string &value = Global::commands()->getArg(2);
//   
//   if (varName.empty() || value.empty()) {
//     Global::console()->print("Usage: set VARNAME VALUE");
//     return;
//   }
//   
//   Var1* tmp = findVar(varName);
//   if (tmp == nullptr) {
//     Global::console()->printf("Could not find var %s", varName.c_str());
//     return;
//   }
//   
//   tmp->set(value);
//   
//   Global::console()->printf("%s now %s\n", varName, value);
// }
// 
// void Var1::init() {
//   Global::commands()->addCommand("get", std::bind(&Var1::get_f));
//   Global::commands()->addCommand("set", std::bind(&Var1::set_f));
// }
// 
// std::string Var1::getName() {
//   return name;
// }
// 
// std::string Var1::getValue() {
//   return value;
// }
// 
// float Var1::getValueF() {
//   return valueF;
// }
// 
// uint32_t Var1::getFlags() {
//   return flags;
// }
// 
// void Var1::setDefault() {
//   set(defaultValue);
// }
// 
// bool Var1::set(const float &valueF) {
// //   if (!(flags & static_cast<uint32_t>(VAR_REGISTER))) {
// //     Global::console()->printf("Could not set variable %s", name.c_str());
// //     return false;
// //   }
//   
//   this->valueF = valueF;
//   this->value = std::to_string(valueF);
//   
//   if (defaultValue.compare(value) != 0) {
//     flags = flags | static_cast<uint32_t>(VAR_CHANGED);
//   } else {
//     flags = flags & ~(static_cast<uint32_t>(VAR_CHANGED));
//   }
//   
//   return true;
// }
// 
// bool Var1::set(const std::string &value) {
// //   if (!(flags & static_cast<uint32_t>(VAR_REGISTER))) {
// //     Global::console()->printf("Could not set variable %s", name.c_str());
// //     return false;
// //   }
//   
//   this->value = value;
//   this->valueF = atof(value.c_str());
//   
//   if (defaultValue.compare(value) != 0) {
//     flags = flags | static_cast<uint32_t>(VAR_CHANGED);
//   } else {
//     flags = flags & ~(static_cast<uint32_t>(VAR_CHANGED));
//   }
//   
//   return true;
// }
// 
// Var1::Var1() {}
// 
// Var1::Var1(const std::string &name, const std::string &defaultVal, const uint32_t &flags) {
//   this->name = name;
//   this->defaultValue = defaultVal;
//   this->flags = flags;
//   
//   setDefault();
// }
// 
// MemoryPool<Var1, 104*10> Var1::varPool;
// std::unordered_map<std::string, Var1*> Var1::vars;

var::var(const std::string &name, const std::string &defaultVal, const uint32_t &flags) 
  : nameVal(name), 
    flagsVal(flags), 
    value(defaultVal), 
    valueF(atof(defaultVal.c_str())), 
    defaultValue(value) {
  flagsVal = flagsVal & (~VAR_STRING);
  flagsVal = flagsVal | VAR_STRING;
}

var::var(const std::string &name, const float &defaultVal, const uint32_t &flags) 
  : nameVal(name), 
    flagsVal(flags), 
    value(std::to_string(defaultVal)), 
    valueF(defaultVal), 
    defaultValue(value) {
  flagsVal = flagsVal & (~VAR_STRING);
}

// template<>
// std::string & var::get() {
//   if ((flagsVal & VAR_STRING) == VAR_STRING) {
//     return stringVal;
//   } 
//   
//   throw std::runtime_error("Trying to get a wrong type variable");
// }
// 
// template<>
// int64_t & var::get() {
//   if ((flagsVal & VAR_INT) == VAR_INT) {
//     return intVal;
//   } 
//   
//   throw std::runtime_error("Trying to get a wrong type variable");
// }
// 
// template<>
// float & var::get() {
//   if ((flagsVal & VAR_FLOAT) == VAR_FLOAT) {
//     return floatVal;
//   } 
//   
//   throw std::runtime_error("Trying to get a wrong type variable");
// }
// 
// template<>
// const std::string & var::get() const {
//   if ((flagsVal & VAR_STRING) == VAR_STRING) {
//     return stringVal;
//   } 
//   
//   throw std::runtime_error("Trying to get a wrong type variable");
// }
// 
// template<>
// const int64_t & var::get() const {
//   if ((flagsVal & VAR_INT) == VAR_INT) {
//     return intVal;
//   } 
//   
//   throw std::runtime_error("Trying to get a wrong type variable");
// }
// 
// template<>
// const float & var::get() const {
//   if ((flagsVal & VAR_FLOAT) == VAR_FLOAT) {
//     return floatVal;
//   } 
//   
//   throw std::runtime_error("Trying to get a wrong type variable");
// }
// 
// template<>
// void var::set(const std::string &value) {
//   if ((flagsVal & VAR_STRING) == VAR_STRING) stringVal = value; 
//   
//   throw std::runtime_error("Trying to set a wrong type variable");
// }
// 
// template<>
// void var::set(const int64_t &value) {
//   if ((flagsVal & VAR_INT) == VAR_INT) intVal = value;
//   
//   throw std::runtime_error("Trying to set a wrong type variable");
// }
// 
// template<>
// void var::set(const float &value) {
//   if ((flagsVal & VAR_FLOAT) == VAR_FLOAT) floatVal = value; 
//   
//   throw std::runtime_error("Trying to set a wrong type variable");
// }

std::string & var::get() {
  return value;
}

const std::string & var::get() const {
  return value;
}

float & var::getFloat() {
  return valueF;
}

const float & var::getFloat() const {
  return valueF;
}

void var::set(const std::string &value) {
  this->value = value;
  this->valueF = atof(value.c_str());
}

void var::set(const float &value) {
  this->valueF = value;
  this->value = std::to_string(value);
}

void var::reset() {
//   if ((flagsVal & VAR_STRING) == VAR_STRING) stringVal = defaultStringVal; 
//   else if ((flagsVal & VAR_INT) == VAR_INT) intVal = defaultIntVal;
//   else floatVal = defaultFloatVal;
  value = defaultValue;
  valueF = atof(value.c_str());
}

std::string var::name() const {
  return nameVal;
}

uint32_t var::flags() const {
  return flagsVal;
}

std::string var::toString() const {
  return value;
}

cvar cvar::get(const std::string &name) {
  auto itr = vars.find(name);
  if (itr == vars.end()) return cvar(nullptr);
  
  return cvar(itr->second);
}

bool cvar::has(const std::string &name) {
  auto itr = vars.find(name);
  return itr != vars.end();
}

void cvar::reset(const std::string &name) {
  auto itr = vars.find(name);
  if (itr == vars.end()) return;
  
  itr->second->reset();
}

void cvar::get_f() {
  const std::string &varName = Global::commands()->getArg(1);
  
  if (varName.empty()) {
    Global::console()->print("Usage: get VARNAME");
    return;
  }
  
  cvar tmp = get(varName);
  if (tmp.valid()) {
    Global::console()->printf("Could not find var %s", varName);
    return;
  }
  
  Global::console()->printf("%s equals %s\n", varName, tmp.toString());
}

// template<>
// void cvar::set(const std::string &value);

void cvar::set_f() {
  const std::string &varName = Global::commands()->getArg(1);
  const std::string &value = Global::commands()->getArg(2);
  
  if (varName.empty() || value.empty()) {
    Global::console()->print("Usage: set VARNAME VALUE");
    return;
  }
  
  cvar tmp = get(varName);
  if (tmp.valid()) {
    Global::console()->printf("Could not find var %s", varName);
    return;
  }
  
  if ((tmp.flags() & VAR_STRING) == VAR_STRING) tmp.set(value);
  else tmp.set(atof(value.c_str()));
  
  Global::console()->printf("%s now %s\n", varName, value);
}

void cvar::destroy() {
  for (const auto &variable : vars) {
    varPool.deleteElement(variable.second);
  }
  
  vars.clear();
}

cvar::cvar(const std::string &name) {
  auto tmp = get(name);
  variable = tmp.variable;
}

cvar::cvar(const std::string &name, const std::string &defaultVal, const uint32_t &flags) {
  if (has(name)) {
    auto tmp = get(name);
    variable = tmp.variable;
    return;
  }
  
  variable = varPool.newElement(name, defaultVal, flags);
  vars.insert(std::make_pair(name, variable));
}

cvar::cvar(const std::string &name, const float &defaultVal, const uint32_t &flags) {
  if (has(name)) {
    auto tmp = get(name);
    variable = tmp.variable;
    return;
  }
  
  variable = varPool.newElement(name, defaultVal, flags);
  vars.insert(std::make_pair(name, variable));
}

bool cvar::valid() const {
  return variable != nullptr;
}

// template<>
// int64_t & cvar::get() {
//   return variable->get<int64_t>();
// }
// 
// template<>
// float & cvar::get() {
//   return variable->get<float>();
// }
// 
// template<>
// std::string & cvar::get() {
//   return variable->get<std::string>();
// }
// 
// template<>
// const int64_t & cvar::get() const {
//   return variable->get<int64_t>();
// }
// 
// template<>
// const float & cvar::get() const {
//   return variable->get<float>();
// }
// 
// template<>
// const std::string & cvar::get() const {
//   return variable->get<std::string>();
// }
// 
// template<>
// void cvar::set(const std::string &value) {
//   variable->set<std::string>(value);
// }
// 
// template<>
// void cvar::set(const int64_t &value) {
//   variable->set<int64_t>(value);
// }
// 
// template<>
// void cvar::set(const float &value) {
//   variable->set<float>(value);
// }

std::string & cvar::get() {
  return variable->get();
}

const std::string & cvar::get() const {
  return variable->get();
}

float & cvar::getFloat() {
  return variable->getFloat();
}

const float & cvar::getFloat() const {
  return variable->getFloat();
}

void cvar::set(const std::string &value) {
  variable->set(value);
}

void cvar::set(const float &value) {
  variable->set(value);
}

void cvar::reset() {
  variable->reset();
}

std::string cvar::name() const {
  return variable->name();
}

uint32_t cvar::flags() const {
  return variable->flags();
}

std::string cvar::toString() const {
  return variable->toString();
}

cvar::cvar(var* variable) : variable(variable) {}

MemoryPool<var, sizeof(var)*20> cvar::varPool;
std::unordered_map<std::string, var*> cvar::vars;
