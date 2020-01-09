#ifndef INFO_COMPONENT_H
#define INFO_COMPONENT_H

#include "Type.h"
#include "UserDataComponent.h"

namespace yacs {
  class entity;
}

class InfoComponent {
public:
  struct CreateInfo {
    Type type;
    yacs::entity* ent;
  };
  InfoComponent(const CreateInfo &info);
  ~InfoComponent();

  void edit();

  Type type() const;
private:
  Type m_type;
  yacs::entity* m_ent;
};

#endif //INFO_COMPONENT_H
