#ifndef MUST_CALL_H
#define MUST_CALL_H

#include "DataflowPass.h"

class MustCall : public DataflowPass {
  protected:
    void leastUpperBound(PVAliasSet &preSet, MethodsSet &curMethodsSet);
    void onAllocationFunctionCall(PVAliasSet* input, std::string &fnName);
    void onDeallocationFunctionCall(PVAliasSet* input, std::string &fnName);
    void onUnknownFunctionCall(PVAliasSet* input);
    void onReallocFunctionCall(PVAliasSet* input, std::string &fnName);
    void onSafeFunctionCall(PVAliasSet* input, std::string &fnName);
    void onAnnotation(PVAliasSet* input, Annotation* annotation);
    void onFunctionCall(PVAliasSet* input, std::string &fnName);
    void checkIfInputIsSubtypeOfAnnotation(PVAliasSet* input, Annotation* annotation, const std::string& infoOutputIfFail);
    void checkIfInputIsSubtypeOfSet(PVAliasSet* input, std::set<std::string> setToCompareWith, const std::string& infoOutputIfFail);

  public:
    MustCall();
};

#endif