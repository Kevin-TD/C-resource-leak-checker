#include "ProgramRepresentation/ProgramVariable.h"
#include "Debug.h"

ProgramVariable::ProgramVariable() {

}

ProgramVariable::ProgramVariable(std::string cleanedName) {
  this->cleanedName = cleanedName;
}

ProgramVariable::ProgramVariable(Value *value) {
  this->value = value;
  this->rawName = dataflow::variable(value);
  this->cleanedName = this->rawName;
  this->fixNameAndIdentifier();
}

ProgramVariable::ProgramVariable(Value *value, MethodsSet methods) {
  this->value = value;
  this->rawName = dataflow::variable(value);
  this->cleanedName = this->rawName;
  this->fixNameAndIdentifier();
  this->methods = MethodsSet::copy(methods); 
}


ProgramVariable::ProgramVariable(Value *value, int index) {
  this->value = value;
  this->rawName = dataflow::variable(value) + "." + std::to_string(index);
  this->cleanedName = this->rawName;
  this->fixNameAndIdentifier();
}

void ProgramVariable::fixNameAndIdentifier() {
  this->varIsIdentifier = true; 
  if (this->cleanedName[0] == '%' || this->cleanedName[0] == '@') {
    this->cleanedName.erase(0, 1);
  } else {
    this->varIsIdentifier = false;
  }
}

std::string ProgramVariable::getRawName() { return this->rawName; }

std::string ProgramVariable::getCleanedName() { return this->cleanedName; }

Value *ProgramVariable::getValue() { return this->value; }

bool ProgramVariable::hasProgramName() { return this->value->hasName(); }

bool ProgramVariable::isIdentifier() { return this->varIsIdentifier; }

bool ProgramVariable::equalsValue(Value *otherValue) {
  return this->value == otherValue;
}

bool ProgramVariable::equalsCleanedName(std::string otherName) {
  return this->cleanedName.compare(otherName) == 0;
}

bool ProgramVariable::equalsRawName(std::string otherRawName) {
  return this->rawName.compare(otherRawName) == 0;
}

void ProgramVariable::addAlias(ProgramVariable pv) {
  this->aliases.push_back(pv);
}

std::set<std::string> ProgramVariable::getNamedAliases(bool cleanNames) {
  std::set<std::string> namedAliases;

  if (this->hasProgramName()) {
    if (cleanNames) {
      namedAliases.insert(this->cleanedName);
    } else {
      namedAliases.insert(this->rawName);
    }
  }

  for (ProgramVariable pv : this->aliases) {
    if (pv.hasProgramName()) {
      if (cleanNames) {
        namedAliases.insert(pv.getCleanedName());
      } else {
        namedAliases.insert(pv.getRawName());
      }
    }
  }
  return namedAliases;
}

std::set<std::string> ProgramVariable::getAllAliases(bool cleanNames) {
  std::set<std::string> allAliases;

  if (cleanNames) {
    allAliases.insert(this->cleanedName);
  } else {
    allAliases.insert(this->rawName);
  }

  for (ProgramVariable pv : this->aliases) {
    if (cleanNames) {
      allAliases.insert(pv.getCleanedName());
    } else {
      allAliases.insert(pv.getRawName());
    }
  }
  return allAliases;
}

MethodsSet ProgramVariable::getMethodsSet() {
  return this->methods;
}

MethodsSet* ProgramVariable::getMethodsSetRef() {
  return &this->methods;
}

std::list<ProgramVariable> ProgramVariable::getPValiases() {
  return this->aliases; 
}

std::list<ProgramVariable>* ProgramVariable::getPValiasesRef() {
  return &this->aliases; 
}

std::list<ProgramVariable*> ProgramVariable::generatePVptrAliases() {
  std::list<ProgramVariable*> result;
  for (ProgramVariable& pv : this->aliases) {
    result.push_back(&pv);
  }
  return result; 
}

void ProgramVariable::setMethodsSet(MethodsSet methods) {
  this->methods = methods; 
}

void ProgramVariable::setAliases(std::list<ProgramVariable> aliases) {
  this->aliases = aliases; 
}

ProgramVariable ProgramVariable::copy(ProgramVariable programVariable) {
  std::list<ProgramVariable> aliasesClone(programVariable.getPValiases());
  Value* valueClone = programVariable.getValue();
  ProgramVariable pvClone = ProgramVariable(valueClone);
  pvClone.setAliases(aliasesClone);
  return pvClone;
}