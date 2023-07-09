#include "CalledMethods.h"
#include "Debug.h"
#include "Constants.h"


CalledMethods::CalledMethods() {
    this->passName = CALLED_METHODS_PASS_NAME; 
}

void CalledMethods::onAllocationFunctionCall(MaybeUninitMethodsSet& input, std::string& fnName) {

}
void CalledMethods::onDeallocationFunctionCall(MaybeUninitMethodsSet& input, std::string& fnName) {
  input.methodsSet.insert(fnName); 
  input.setInitialized = true; 
}
void CalledMethods::onUnknownFunctionCall(MaybeUninitMethodsSet& input) {
  input.methodsSet = {}; 
}
void CalledMethods::onUnsafeFunctionCall(MaybeUninitMethodsSet& input, std::string& fnName) {
  input.methodsSet = {}; 
}
void CalledMethods::onReallocFunctionCall(MaybeUninitMethodsSet& input, std::string& fnName) {
  input.methodsSet = {}; 
}
void CalledMethods::onSafeFunctionCall(MaybeUninitMethodsSet& input, std::string& fnName) {

}
void CalledMethods::leastUpperBound(MaybeUninitMethodsSet& preMethods, MaybeUninitMethodsSet& curMethods, std::set<std::string>& result) {
   std::set_intersection(preMethods.methodsSet.begin(), preMethods.methodsSet.end(), 
                      curMethods.methodsSet.begin(), curMethods.methodsSet.end(), 
                      std::inserter(result, result.begin()));
}