// этот подход плох тем что мы задаем все аттрибуты в коде
// и я не очень понимаю как задавать их тогда через json
// в таком случае нужно будет придумывать способ положить 
// несколько компонентов одного типа в энтити
// а это не нужно

#ifndef HEALTH_H
#define HEALTH_H

#include "Attributes.h"

class Health : public Attribute<INT_ATTRIBUTE_TYPE> {
public:
  
};

#endif
