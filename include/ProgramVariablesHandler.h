#ifndef PROGRAM_VARIABLES_HANDLER_H
#define PROGRAM_VARIABLES_HANDLER_H

#include "ProgramVariable.h"

class ProgramVariablesHandler {
private:
  std::list<ProgramVariable> programVariables;

public:
  void addAlias(ProgramVariable receiving, ProgramVariable receiver);

  void addVariable(ProgramVariable programVar);

  std::list<ProgramVariable> getProgramVariables();

  std::set<std::string> findVarAndNamedAliases(std::string cleanedName);
};

#endif