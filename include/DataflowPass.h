#ifndef DATAFLOW_PASS_H
#define DATAFLOW_PASS_H

#include "CFG.h"
#include "ProgramVariablesHandler.h"
#include "RunAnalysis.h"
#include "Utils.h"

#include <fstream>
#include <set>

/*
considering an alternative design:

Base class ProgramNamedKeyword
Derived class ProgramVariable : ProgramNamedKeyword
Derived class ProgramFunction : ProgramNamedKeyword
Derived class ProgramAnnotation : ProgramNamedKeyword
*/

struct MaybeUninitMethodsSet {
  std::set<std::string> methodsSet;
  bool setInitialized;
};

// mapping for an entire program; first string is branch name, second string is
// var name
typedef std::map<std::string, std::map<std::string, MaybeUninitMethodsSet>>
    MappedMethods;

class DataflowPass {
protected:
  ProgramVariablesHandler programVariables;
  std::string testName;
  CFG *cfg;
  MappedMethods expectedResult;

  void analyzeCFG(CFG *cfg, MappedMethods &PreMappedMethods,
                  MappedMethods &PostMappedMethods, std::string priorBranch);
  virtual void leastUpperBound(MaybeUninitMethodsSet &preMethods,
                               MaybeUninitMethodsSet &curMethods,
                               std::set<std::string> &result) = 0;

  void transfer(Instruction *instruction, SetVector<Instruction *> workSet,
                std::map<std::string, MaybeUninitMethodsSet> &inputMethodsSet);
  virtual void onAllocationFunctionCall(MaybeUninitMethodsSet &input,
                                        std::string &fnName) = 0;
  virtual void onDeallocationFunctionCall(MaybeUninitMethodsSet &input,
                                          std::string &fnName) = 0;
  virtual void onUnknownFunctionCall(MaybeUninitMethodsSet &input) = 0;
  virtual void onReallocFunctionCall(MaybeUninitMethodsSet &input,
                                     std::string &fnName) = 0;
  virtual void onSafeFunctionCall(MaybeUninitMethodsSet &input,
                                  std::string &fnName) = 0;

public:
  void setFunctions(std::set<std::string> safeFunctions,
                    std::set<std::string> reallocFunctions,
                    std::map<std::string, std::string> memoryFunctions);

  std::set<std::string> safeFunctions;
  std::set<std::string> reallocFunctions;
  std::map<std::string, std::string> memoryFunctions;
  std::string passName;

  MappedMethods generatePassResults();

  void setCFG(CFG *cfg);
  void setExpectedResult(MappedMethods expectedResult);
  void setProgramVariables(ProgramVariablesHandler programVariables);

  MappedMethods getExpectedResult();
};

#endif