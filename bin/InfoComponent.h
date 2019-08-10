#ifndef INFO_COMPONENT_H
#define INFO_COMPONENT_H

#include "Type.h"
#include "UserDataComponent.h"

class InfoComponent {
public:
  struct CreateInfo {
    Type type;
    UserDataComponent* userData;
  };
  InfoComponent(const CreateInfo &info);
  ~InfoComponent();

  void edit();

  std::string getType() const;
private:
  Type type;
  UserDataComponent* userData;
};

#endif //INFO_COMPONENT_H
