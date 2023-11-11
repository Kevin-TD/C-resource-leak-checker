#ifndef DATAFLOW_PASS_H
#define DATAFLOW_PASS_H

#include "Annotations/Annotation.h"
#include "Annotations/AnnotationHandler.h"
#include "Annotations/ErrorAnnotation.h"
#include "Annotations/FunctionAnnotation.h"
#include "Annotations/ParameterAnnotation.h"
#include "Annotations/ReturnAnnotation.h"
#include "Annotations/StructAnnotation.h"
#include "CFG.h"
#include "ProgramRepresentation/FullFile.h"
#include "RunAnalysis.h"
#include "Utils.h"

class DataflowPass {
private:
  void analyzeCFG(CFG *cfg, ProgramFunction &preProgramFunction,
                  ProgramFunction &postProgramFunction,
                  std::string priorBranch);
  void transfer(Instruction *instruction, ProgramPoint &inputProgramPoint);
  void insertAnnotation(Annotation *annotation, ProgramVariable *pv);

  // a helper function that handles functions with Sret attribute.
  // returns true if the function had an Sret attribute and was handled,
  // and false if not.
  /*
    LLVM uses sret attributes as an optimization if the return type is
    large. a struct with more than 2 fields can trigger this optimization.

    docs: https://llvm.org/docs/LangRef.html#parameter-attributes
    less credible forum post:
    https://discourse.llvm.org/t/optimizing-sret-on-caller-side/60660

    our instructions end up looking like this:
    call void @does_something_a(%struct.my_struct* sret %B,
    %struct.my_struct* byval align 8 %A), !dbg !58

    here's how we will handle these types of instructions:

    say we have the instruction
    call void @does_something_a(%struct.my_struct* sret %B,
    %struct.my_struct* byval align 8 %A), !dbg !58

    (this is from C code
    my_struct B = does_something_a(A);)

    *we assume LHS is always a struct and first argument always has sret
    attribute

    we will:
    -destructure B into its fields (B.0, B.1, ...)
    -determine B's MustCall/CalledMethods from annotations on return on
    does_something_a
    -destructure A into its fields (A.0, A.1, ...)
    -determine A's MustCall/CalledMethods from annotations on params on
    does_something a past the first argument, it could be struct or
    non-struct. non-structs will have their MustCall/CalledMethods
    determined, but without the destructuring
  */
  bool handleSretCallForCallInsts(CallInst *call, int argIndex,
                                  const std::string &fnName,
                                  const std::string &argName,
                                  ProgramPoint &programPoint);

  // a helper function that handles function calls that are implicit,
  // or  identified as a memory, realloc, or safe function (/Functions files).
  // also handles when llvm debug function is called. returns true if we
  // handled the call, and false otherwise.
  bool handleIfKnownFunctionForCallInsts(CallInst *call, ProgramVariable *pv);

  // a helper function that checks for parameter annotations on call
  // instructions. returns true if an annotation was found, and false if not.
  bool handleIfAnnotationExistsForCallInsts(const std::string &fnName,
                                            int argIndex, ProgramVariable *pv);

protected:
  ProgramFunction programFunction;
  AnnotationHandler annotations;
  std::string testName;
  CFG *cfg;
  FullFile expectedResult;

  virtual void leastUpperBound(MethodsSet &preMethods, MethodsSet &curMethods,
                               MethodsSet &result) = 0;

  virtual void onAllocationFunctionCall(MethodsSet *input,
                                        std::string &fnName) = 0;
  virtual void onDeallocationFunctionCall(MethodsSet *input,
                                          std::string &fnName) = 0;
  virtual void onUnknownFunctionCall(MethodsSet *input) = 0;
  virtual void onReallocFunctionCall(MethodsSet *input,
                                     std::string &fnName) = 0;
  virtual void onSafeFunctionCall(MethodsSet *input, std::string &fnName) = 0;
  virtual void onAnnotation(MethodsSet *input, std::string &fnName,
                            AnnotationType annotationType) = 0;

public:
  void setFunctions(std::set<std::string> safeFunctions,
                    std::set<std::string> reallocFunctions,
                    std::map<std::string, std::string> memoryFunctions,
                    AnnotationHandler annotations);

  std::set<std::string> safeFunctions;
  std::set<std::string> reallocFunctions;
  std::map<std::string, std::string> memoryFunctions;
  std::string passName;

  ProgramFunction generatePassResults();

  void setCFG(CFG *cfg);
  void setExpectedResult(FullFile expectedResult);
  void setProgramFunction(ProgramFunction programFunction);
  void setAnnotations(AnnotationHandler annotations);

  FullFile getExpectedResult();
};

#endif