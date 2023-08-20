#include "Annotations/AnnotationHandler.h"

AnnotationHandler::AnnotationHandler() {}

AnnotationHandler::AnnotationHandler(const std::string &fileName) {
  this->addAnnotationsFromFile(fileName);
}

void AnnotationHandler::addAnnotation(const std::string &rawAnnotationString) {
  Annotation *anno = generateAnnotation(rawAnnotationString);

  if (StructAnnotation *structAnno = dynamic_cast<StructAnnotation *>(anno)) {
    this->annotations.push_back(structAnno);
  } else if (FunctionAnnotation *functionAnno =
                 dynamic_cast<FunctionAnnotation *>(anno)) {
    this->annotations.push_back(functionAnno);
  } else if (ParameterAnnotation *paramAnno =
                 dynamic_cast<ParameterAnnotation *>(anno)) {
    this->annotations.push_back(paramAnno);
  } else if (ReturnAnnotation *returnAnno =
                 dynamic_cast<ReturnAnnotation *>(anno)) {
    this->annotations.push_back(returnAnno);
  }
}

void AnnotationHandler::addAnnotationsFromFile(const std::string &fileName) {
  LLVMContext context;
  SMDiagnostic error;

  std::unique_ptr<Module> M = parseIRFile(fileName, error, context);

  for (llvm::GlobalVariable &globalVar : M->globals()) {
    if (globalVar.hasInitializer()) {
      llvm::Constant *initializer = globalVar.getInitializer();
      if (llvm::ConstantDataSequential *dataSeq =
              llvm::dyn_cast<llvm::ConstantDataSequential>(initializer)) {
        if (dataSeq->isString()) {
          std::string stringValue = dataSeq->getAsString().str();
          this->addAnnotation(stringValue);
        }
      }
    }
  }
}

Annotation *
AnnotationHandler::getFunctionAnnotation(const std::string &functionName) {
  for (Annotation *anno : this->annotations) {
    if (FunctionAnnotation *functionAnno =
            dynamic_cast<FunctionAnnotation *>(anno)) {
      if (functionAnno->functionNameEquals(functionName)) {
        return functionAnno;
      }
    }
  }
  return new ErrorAnnotation();
}

Annotation *
AnnotationHandler::getParameterAnnotation(const std::string &functionName,
                                          int nthParameter) {
  for (Annotation *anno : this->annotations) {
    if (ParameterAnnotation *paramAnno =
            dynamic_cast<ParameterAnnotation *>(anno)) {
      if (paramAnno->functionNameEquals(functionName) &&
          paramAnno->nthParameterEquals(nthParameter) &&
          !paramAnno->paramHasField()) {
        return paramAnno;
      }
    }
  }
  return new ErrorAnnotation();
}

Annotation *
AnnotationHandler::getParameterAnnotation(const std::string &functionName,
                                          int nthParameter,
                                          const std::string &field) {
  for (Annotation *anno : this->annotations) {
    if (ParameterAnnotation *paramAnno =
            dynamic_cast<ParameterAnnotation *>(anno)) {
      if (paramAnno->functionNameEquals(functionName) &&
          paramAnno->nthParameterEquals(nthParameter) &&
          paramAnno->fieldNameEquals(field)) {
        return paramAnno;
      }
    }
  }
  return new ErrorAnnotation();
}

Annotation *
AnnotationHandler::getReturnAnnotation(const std::string &functionName) {
  for (Annotation *anno : this->annotations) {
    if (ReturnAnnotation *returnAnno = dynamic_cast<ReturnAnnotation *>(anno)) {
      if (returnAnno->functionNameEquals(functionName) &&
          !returnAnno->returnHasField()) {
        return returnAnno;
      }
    }
  }
  return new ErrorAnnotation();
}

Annotation *
AnnotationHandler::getReturnAnnotation(const std::string &functionName,
                                       const std::string &field) {
  for (Annotation *anno : this->annotations) {
    if (ReturnAnnotation *returnAnno = dynamic_cast<ReturnAnnotation *>(anno)) {
      if (returnAnno->functionNameEquals(functionName) &&
          returnAnno->fieldNameEquals(field)) {
        return returnAnno;
      }
    }
  }
  return new ErrorAnnotation();
}

Annotation *
AnnotationHandler::getStructAnnotation(const std::string &structName,
                                       const std::string &field) {
  for (Annotation *anno : this->annotations) {
    if (StructAnnotation *structAnno = dynamic_cast<StructAnnotation *>(anno)) {
      if (structAnno->structNameEquals(structName) &&
          structAnno->fieldNameEquals(field)) {
        return structAnno;
      }
    }
  }
  return new ErrorAnnotation();
}