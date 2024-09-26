#include "CalledMethods.h"
#include "Constants.h"
#include "Debug.h"

CalledMethods::CalledMethods() {
    this->passName = CALLED_METHODS_PASS_NAME;
}

void CalledMethods::onAllocationFunctionCall(PVAliasSet *input,
        std::string &fnName) {}
void CalledMethods::onDeallocationFunctionCall(PVAliasSet *input,
        std::string &fnName) {
    input->addMethod(fnName);
}
void CalledMethods::onUnknownFunctionCall(PVAliasSet *input) {
    input->clearMethods();
}
void CalledMethods::onReallocFunctionCall(PVAliasSet *input,
        std::string &fnName) {
    input->clearMethods();
}
void CalledMethods::onSafeFunctionCall(PVAliasSet *input, std::string &fnName) {
}

void CalledMethods::leastUpperBound(PVAliasSet &preSet, MethodsSet &curMethodsSet) {
    preSet.methodsSetIntersection(curMethodsSet);
}

void CalledMethods::onAnnotation(PVAliasSet* input, Annotation* annotation) {
    if (annotation->getAnnotationType() == AnnotationType::CallsAnnotation) {
        auto annoMethods = annotation->getAnnotationMethods();
        for (std::string annoMethod : annoMethods) {
            input->addMethod(annoMethod);

        }
    }
}

void CalledMethods::onFunctionCall(PVAliasSet* input, std::string &fnName) {
    input->addMethod(fnName);
}

void CalledMethods::checkIfInputIsSubtypeOfAnnotation(PVAliasSet* input, Annotation* annotation, const std::string& infoOutputIfFail) {
    if (annotation->getAnnotationType() == AnnotationType::CallsAnnotation) {
        std::set<std::string> annoMethods = annotation->getAnnotationMethods();
        this->checkIfInputIsSubtypeOfSet(input,  annoMethods, infoOutputIfFail);
    }
}

void CalledMethods::checkIfInputIsSubtypeOfSet(PVAliasSet* input, std::set<std::string> setToCompareWith, const std::string& infoOutputIfFail) {
    std::set<std::string> pvasMethods = input->getMethodsSet().getMethods();

    if (pvasMethods < setToCompareWith) {
        std::set<std::string> setDifference = rlc_util::getSymmetricDifference(pvasMethods, setToCompareWith);

        logout("ERROR: Input methods is not subtype for CalledMethods. Invalid for " << infoOutputIfFail << ": ");
        logout(rlc_util::formatSet("CalledMethods(\"{}\")\n", setDifference));
    }
}