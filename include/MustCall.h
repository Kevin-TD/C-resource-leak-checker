#ifndef MUST_CALL_H
#define MUST_CALL_H

#include "PassType.h"

class MustCall : public PassType {
protected:    
    void leastUpperBound(MaybeUninitMethodsSet& preMethods, MaybeUninitMethodsSet& curMethods, std::set<std::string>& result); 
    void onAllocationFunctionCall(MaybeUninitMethodsSet& input, std::string& fnName);
    void onDeallocationFunctionCall(MaybeUninitMethodsSet& input, std::string& fnName);
    void onUnknownFunctionCall(MaybeUninitMethodsSet& input);
    void onUnsafeFunctionCall(MaybeUninitMethodsSet& input, std::string& fnName);
    void onReallocFunctionCall(MaybeUninitMethodsSet& input, std::string& fnName);
    void onSafeFunctionCall(MaybeUninitMethodsSet& input, std::string& fnName);
public:
    MustCall(); 
};


#endif 