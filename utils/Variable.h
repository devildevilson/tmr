/* 
 * Игровые переменные
 * Идея возникла после просмотра исходного кода Quake
 */

#ifndef VARIABLE_H
#define VARIABLE_H

#include <string>
#include <vector>
#include <unordered_map>

class VariableC;
#include "MemoryPool.h"

enum VarFlags : uint32_t {
  VAR_NONE       = 0,
  VAR_REGISTER   = (1<<0), // переменная готова к использованию (пригодится ли мне это?)
  VAR_ARCHIVE    = (1<<1), // переменная готова к архивации
  VAR_CHANGED    = (1<<2), // переменная изменена
  VAR_SERVERINFO = (1<<3), // переменная сервера отправляемая всем пользователям (пригодится ли?) (типа MOTD и тд)
  VAR_USERINFO   = (1<<4), // переменная пользователя отправляемая на сервер (пригодится ли?)
  VAR_STRING     = (1<<5)
};

// class Var1 {
//   friend MemoryPool<Var1, 104*10>;
// public:
//   static Var1*              findVar(const std::string &name);
//   static std::vector<Var1*> findVarsWithFlags(const uint32_t &flags);
//   static float              getValueF(const std::string &name);
//   static std::string        getValue(const std::string &name);
//   static bool               reset(const std::string &name);
//   static bool               set(const std::string &name, const float &valueF);
//   static bool               set(const std::string &name, const std::string &value);
//   static Var1*              add(const std::string &name, const std::string &defaultVal = "", const uint32_t &flags = 0);
//   
//   static void get_f();
//   static void set_f();
//   static void init();
//   
//   std::string getName();
//   std::string getValue();
//   float       getValueF();
//   uint32_t    getFlags();
//   void        setDefault();
//   bool set(const float &valueF);
//   bool set(const std::string &value);
// private:
//   Var1();
//   Var1(const std::string &name, const std::string &defaultVal, const uint32_t &flags = 0);
//   
//   std::string name;
//   std::string value;
//   float       valueF = 0.0f;
//   uint32_t    flags  = 0;
//   std::string defaultValue;
//   
//   static MemoryPool<Var1, 104*10> varPool;
//   static std::unordered_map<std::string, Var1*> vars;
// };

// в анрыле используется наследование
// а для того чтобы получить значение используются функции getInt getFloat и тд
// которые виртуальные, система такая себе на самом деле
// мне нужно хранить строки, инты и флоаты, как это сделать? union?

class var {
public:
  var(const std::string &name, const std::string &defaultVal, const uint32_t &flags);
  var(const std::string &name, const float &defaultVal, const uint32_t &flags);
  
  std::string & get();
  const std::string & get() const;
  float & getFloat();
  const float & getFloat() const;
  
  void set(const std::string &value);
  void set(const float &value);
  
  void reset();
  
  std::string name() const;
  uint32_t flags() const;
  std::string toString() const;
private:
  std::string nameVal;
  uint32_t flagsVal;
  
  std::string value;
  float valueF;
  std::string defaultValue;
};

class cvar {
public:
  static cvar get(const std::string &name);
  static bool has(const std::string &name);
  static void reset(const std::string &name);
  static void get_f();
  static void set_f();
  
  static void destroy();
  
  cvar(const std::string &name);
  cvar(const std::string &name, const std::string &defaultVal, const uint32_t &flags);
  cvar(const std::string &name, const float &defaultVal, const uint32_t &flags);
  
  bool valid() const;
  
  std::string & get();
  const std::string & get() const;
  float & getFloat();
  const float & getFloat() const;
  
  void set(const std::string &value);
  void set(const float &value);
  
  void reset();
  
  std::string name() const;
  uint32_t flags() const;
  std::string toString() const;
private:
  cvar(var* variable);
  
  var* variable;
  
  static MemoryPool<var, sizeof(var)*20> varPool;
  static std::unordered_map<std::string, var*> vars;
};

// struct Var {
//   std::string name;
//   std::string value;
//   float       valueF = 0.0;
//   uint32_t    flags  = 0;
//   std::string defaultValue;
// };
// 
// class VariableC {
// public:
//                          VariableC();
//                          ~VariableC();
//   
//   void                   init();
//   Var*                   findVar(const std::string &name);
//   std::vector<Var*>      findVarWithFlags(const uint32_t &flags);
//   float                  varValueF(const std::string &name);
//   std::string            varValue(const std::string &name);
//   bool                   resetVar(const std::string &name);
//   bool                   setVar(const std::string &name, const float &valueF);
//   bool                   setVar(const std::string &name, const std::string &value);
//   bool                   addVar(const std::string &name, const std::string &defaultVal, const uint32_t &flags = 0);
//   
//   void get_f();
// private:
//   void                   setVar(Var* var, const float &valueF);
//   void                   setVar(Var* var, const std::string &value);
//   
//   std::unordered_map<std::string, Var> vars;
//   
//   //Console* console = nullptr;
// };

#endif // VARIABLE_H
