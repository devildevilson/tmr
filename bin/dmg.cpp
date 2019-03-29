#include "AttributesComponent.h"
#include "Type.h"

static Type physAType = Type::get("physicalDamage");
static Type physDef = Type::get("physicalDefence");
static Type physResist = Type::get("physicalResistance");
static Type healthType = Type::get("health");

void damageCompute(const Attributes* attackerAttrib, Attributes* attackedAttrib) {
  Attribute* physAttack = attackerAttrib->get(physAType);
  Attribute* physDefence = attackedAttrib->get(physDef);
  Attribute* physRes = attackedAttrib->get(physResist);
  
  float dmg = physAttack->max();
  float def = physDefence->max();
  float res = physRes->max();
  
  float finalDmg = (dmg - def) / res;
  
  Attribute* health = attackedAttrib->get(healthType);
  
  health->sub(finalDmg);
  if (health->isCurrentEqMin()) {
    // пизда
  }
}
