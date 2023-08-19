#ifndef CALLED_METHODS_H
#define CALLED_METHODS_H

#include "DataflowPass.h"

class CalledMethods : public DataflowPass {
protected:
  void leastUpperBound(MaybeUninitMethodsSet &preMethods,
                       MaybeUninitMethodsSet &curMethods,
                       std::set<std::string> &result);
  void onAllocationFunctionCall(MaybeUninitMethodsSet &input,
                                std::string &fnName);
  void onDeallocationFunctionCall(MaybeUninitMethodsSet &input,
                                  std::string &fnName);
  void onUnknownFunctionCall(MaybeUninitMethodsSet &input);
  void onReallocFunctionCall(MaybeUninitMethodsSet &input, std::string &fnName);
  void onSafeFunctionCall(MaybeUninitMethodsSet &input, std::string &fnName);
  void onAnnotation(MaybeUninitMethodsSet &input, std::string &fnName,
                    AnnotationType annotationType);

public:
  CalledMethods();
};

#endif