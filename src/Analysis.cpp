#include "RunAnalysis.h"
#include "Utils.h"
#include "PointerAnalysis.h"
#include "loadFunctions.h"
#include "CFG.h"

#include <map> 
#include <tuple>
#include <vector> 
#include <list>

#define DEBUG true
#if DEBUG
#define logout(x) errs() << x << "\n";
#define logDomain(x) x->print(errs()); 
#define logOutMemory(x) printMemory(x);
#else
#define logout(x) 
#define logDomain(x) 
#define logOutMemory(x) 
#endif 

// to run: cd build   then 
// sh ../run_test.sh <test_num> 

struct CM {
  std::set<std::string> cmSet; 
  bool setInitialized; 
};

typedef std::map<std::string, std::map<std::string, CM>> CalledMethods; // left = branch name. inner left = var name 
typedef std::map<std::string, std::string> AliasMap;
typedef std::map<std::string, std::string> VarBranchMap;


// worth noting that has an expr is declared when IR does "call void @llvm.dbg.declare [other IR ...]"

// TODO: make sure program does not utterly collapse

namespace dataflow {

std::map<std::string, bool> SafeFunctions;
std::map<std::string, bool> UnsafeFunctions;
std::map<std::string, bool> ReallocFunctions;
std::map<std::string, std::string> MemoryFunctions;
std::vector<std::string> realBranchOrder; // ONLY for debugging purposes 

void loadFunctions() { 
  // working directory is /build 

  std::ifstream safeFunctionsFile("../src/Functions/safe.txt");
  std::ifstream unsafeFunctionsFile("../src/Functions/unsafe.txt");
  std::ifstream reallocFunctionsFile("../src/Functions/realloc.txt");
  std::ifstream memoryFunctionsFile("../src/Functions/test_memory.txt"); //*NOTE: test memory being used here instead of "real" memory allocation functions 

  std::string line; 
  if (safeFunctionsFile.is_open()) {
    while (std::getline(safeFunctionsFile, line)) {
      SafeFunctions[line] = true; 
    }
  }

  if (unsafeFunctionsFile.is_open()) {
    while (std::getline(unsafeFunctionsFile, line)) {
      UnsafeFunctions[line] = true; 
    }
  }

  if (reallocFunctionsFile.is_open()) {
    while (std::getline(reallocFunctionsFile, line)) {
      ReallocFunctions[line] = true; 
    }
  }

  if (memoryFunctionsFile.is_open()) {
    while (std::getline(memoryFunctionsFile, line)) {
      std::string allocationFunction;
      std::string deallocationFunction;  
      std::string s;          

      for (int i = 0; i < line.size(); i++) {
        if (line[i] == ' ') {
          allocationFunction = s; 
          s = ""; 
          continue;
        }
        s += line[i];
      }
      deallocationFunction = s; 
      
      MemoryFunctions[allocationFunction] = deallocationFunction;
    }

  }


  safeFunctionsFile.close();
  unsafeFunctionsFile.close();
  memoryFunctionsFile.close();
  reallocFunctionsFile.close();


}

/**
 * @brief Get the Predecessors of a given instruction in the control-flow graph.
 *
 * @param Inst The instruction to get the predecessors of.
 * @return Vector of all predecessors of Inst.
 */
std::vector<Instruction *> getPredecessors(Instruction *Inst) {
  std::vector<Instruction *> Ret;
  auto Block = Inst->getParent();
  for (auto Iter = Block->rbegin(), End = Block->rend(); Iter != End; ++Iter) {
    if (&(*Iter) == Inst) {
      ++Iter;
      if (Iter != End) {
        Ret.push_back(&(*Iter));
        return Ret;
      }
      for (auto Pre = pred_begin(Block), BE = pred_end(Block); Pre != BE;
           ++Pre) {
        Ret.push_back(&(*((*Pre)->rbegin())));
      }
      return Ret;
    }
  }
  return Ret;
}

/**
 * @brief Get the successors of a given instruction in the control-flow graph.
 *
 * @param Inst The instruction to get the successors of.
 * @return Vector of all successors of Inst.
 */
std::vector<Instruction *> getSuccessors(Instruction *Inst) {
  std::vector<Instruction *> Ret;
  auto Block = Inst->getParent();
  for (auto Iter = Block->begin(), End = Block->end(); Iter != End; ++Iter) {
    if (&(*Iter) == Inst) {
      ++Iter;
      if (Iter != End) {
        Ret.push_back(&(*Iter));
        return Ret;
      }
      for (auto Succ = succ_begin(Block), BS = succ_end(Block); Succ != BS;
           ++Succ) {
        Ret.push_back(&(*((*Succ)->begin())));
      }
      return Ret;
    }
  }
  return Ret;
}

bool isNumber(const std::string& s) {
    for (char const &ch : s) {
        if (std::isdigit(ch) == 0) 
            return false;
    }
    return true;
 }
 

void transfer(Instruction* I, SetVector<Instruction *>& workSet, CalledMethods& calledMethodsEst, AliasMap& aliasedVars) {
  std::string branchName = I->getParent()->getName().str();
  logout("inst " << *I << " branch name = " << branchName)
  bool includes = false; 
  for (auto branch : realBranchOrder) {
    if (branch == branchName) {
      includes = true; 
      break;
    }
  }
  if (!includes) {
    realBranchOrder.push_back(branchName);
  }


   if (auto Alloca = dyn_cast<AllocaInst>(I)) {
      logout("allocate inst, name = " << ("%" + Alloca->getName()))
      calledMethodsEst[branchName]["%" + Alloca->getName().str()] = {};
      
    }
    else if (auto Load = dyn_cast<LoadInst>(I)) {
      logout("(load) name is " << variable(Load) << " for " << variable(Load->getPointerOperand()) )

      std::string varName = variable(Load->getPointerOperand()); 
      while (varName.size() > 1 && isNumber(varName.substr(1))) { 
        varName = aliasedVars[varName]; 
      }

      logout(variable(Load) << " -> " << varName)

      aliasedVars[variable(Load)] = varName;
    }
    else if (auto Store = dyn_cast<StoreInst>(I)) {

      //   store i8* %call, i8** %str, align 8, !dbg !17

      Value* valueToStore = Store->getOperand(0);       // i8* %call
      Value* receivingValue = Store->getOperand(1);   // i8** %str

      logout("value to store = " << valueToStore << " !!!! " << *valueToStore)
      logout("value that's receiving = " << variable(receivingValue))

      
      for (auto Inst : workSet) {
        if (valueToStore == Inst) {
          if (auto Call = dyn_cast<CallInst>(Inst)) {
            std::string varName = variable(Store->getOperand(1)); 
            std::string fnName = Call->getCalledFunction()->getName().str(); 
            logout("fn name = " << fnName)

            if (
              UnsafeFunctions.count(fnName) == 0  && 
              UnsafeFunctions.count(MemoryFunctions[fnName]) == 0 && 
              MemoryFunctions[fnName].size() > 0
            ) {
              //  calledMethodsEst[branchName][varName].insert(fnName);   
               logout("fn name true for " << branchName << " " << varName)
              //  allocation function 

            }

          }

        }
      }

    
    }
    else if (auto Call = dyn_cast<CallInst>(I)) {
      for (unsigned i = 0; i < Call->getNumArgOperands(); ++i) {
          Value *argument = Call->getArgOperand(i);
          std::string argName = variable(argument);


          argName = aliasedVars[argName];
          while (argName.size() > 1 && isNumber(argName.substr(1))) {
            argName = aliasedVars[argName]; 
          }

          logout("arg = " << argName) 

          // * i THINK this is what distinguishes "real" variables defined in the program from constants and other irrelevant llvm stuff 
          if (argName[0] != '%') return; 

          // * (i think this handles) cases where an entirely random and undefined (implicitly declared) function shows up 
          if (Call->getCalledFunction() == NULL) {
            const DebugLoc &debugLoc = I->getDebugLoc();
            std::string location = "Line " + std::to_string(debugLoc.getLine()) + ", Col " + std::to_string(debugLoc.getCol());

            errs() << "CME-WARNING: unknown (unforseen occurence) & unsafe func on " << location << ". must call satisfied property for '" << argName << "' set to false.\n";
            
            return; 
          }

          std::string fnName = Call->getCalledFunction()->getName().str();

          logout("fnName for call = " << fnName << " " << branchName)

          if (UnsafeFunctions[fnName]) {
            logout("unsafe func")
            return; 
          }

          if (ReallocFunctions[fnName]) {
            logout("realloc func")
            // calledMethodsEst[branchName][argName].insert(fnName); 
            return; 
          }

          if (SafeFunctions[fnName]) {
            logout("safe func")
            return; 
          }


          for (auto Pair : MemoryFunctions) {
            logout("pairs and fn name = " << Pair.first << " " << Pair.second << " " << branchName << " " << argName)
            if (fnName == Pair.first) { // allocation function

            }
            else if (fnName == Pair.second) { // deallocation function 
              calledMethodsEst[branchName][argName].cmSet.insert(fnName); 
              calledMethodsEst[branchName][argName].setInitialized = true; 
              break; 

            }
          }
          



      }
    }
    else if (auto Branch = dyn_cast<BranchInst>(I)) {

    }


}

void transferUnbranched(Instruction* I, SetVector<Instruction *>& workSet, std::map<std::string, CM>& calledMethodsSet, AliasMap& aliasedVars) {
  bool includes = false; 
  std::string branchName = I->getParent()->getName().str();
  for (auto branch : realBranchOrder) {
    if (branch == branchName) {
      includes = true; 
      break;
    }
  }
  if (!includes) {
    realBranchOrder.push_back(branchName);
  }

  auto preds = getPredecessors(I);


  if (auto Alloca = dyn_cast<AllocaInst>(I)) {
    calledMethodsSet["%" + Alloca->getName().str()] = {};
    
  }
  else if (auto Load = dyn_cast<LoadInst>(I)) {
    logout("(load) name is " << variable(Load) << " for " << variable(Load->getPointerOperand()) )

      std::string varName = variable(Load->getPointerOperand()); 
      while (varName.size() > 1 && isNumber(varName.substr(1))) {
        varName = aliasedVars[varName]; 
      }

      logout(variable(Load) << " -> " << varName)

      aliasedVars[variable(Load)] = varName;
  }
  else if (auto Store = dyn_cast<StoreInst>(I)) {

    //   store i8* %call, i8** %str, align 8, !dbg !17

    Value* valueToStore = Store->getOperand(0);       // i8* %call
    Value* receivingValue = Store->getOperand(1);   // i8** %str
    
    for (auto Inst : workSet) {
      if (valueToStore == Inst) {
        if (auto Call = dyn_cast<CallInst>(Inst)) {
          std::string varName = variable(Store->getOperand(1)); 
          std::string fnName = Call->getCalledFunction()->getName().str(); 
          if (
            UnsafeFunctions.count(fnName) == 0  && 
            UnsafeFunctions.count(MemoryFunctions[fnName]) == 0 && 
            MemoryFunctions[fnName].size() > 0
          ) {
            //  calledMethodsEst[branchName][varName].insert(fnName);   
            //  allocation function 

          }

        }

      }
    }

  
  }
  else if (auto Call = dyn_cast<CallInst>(I)) {
    for (unsigned i = 0; i < Call->getNumArgOperands(); ++i) {
        Value *argument = Call->getArgOperand(i);
        std::string argName = variable(argument);

        argName = aliasedVars[argName];
        while (argName.size() > 1 && isNumber(argName.substr(1))) {
          argName = aliasedVars[argName]; 
        }


        // * i THINK this is what distinguishes "real" variables defined in the program from constants and other irrelevant llvm stuff 
        if (argName[0] != '%') return; 

        // * (i think this handles) cases where an entirely random and undefined (implicitly declared) function shows up 
        if (Call->getCalledFunction() == NULL) {
          const DebugLoc &debugLoc = I->getDebugLoc();
          std::string location = "Line " + std::to_string(debugLoc.getLine()) + ", Col " + std::to_string(debugLoc.getCol());

          errs() << "CME-WARNING: unknown (unforseen occurence) & unsafe func on " << location << ". must call satisfied property for '" << argName << "' set to false.\n";
          
          return; 
        }

        std::string fnName = Call->getCalledFunction()->getName().str();


        if (UnsafeFunctions[fnName]) {
          logout("unsafe func")
          return; 
        }

        if (ReallocFunctions[fnName]) {
          logout("realloc func")
          // calledMethodsEst[branchName][argName].insert(fnName); 
          return; 
        }

        if (SafeFunctions[fnName]) {
          logout("safe func")
          return; 
        }


        for (auto Pair : MemoryFunctions) {
          if (fnName == Pair.first) { // allocation function

          }
          else if (fnName == Pair.second) { // deallocation function 
            calledMethodsSet[argName].cmSet.insert(fnName); 
            calledMethodsSet[argName].setInitialized = true; 
            break; 

          }
        }
        



    }
  }
  else if (auto Branch = dyn_cast<BranchInst>(I)) {

  }


}

void analyzeCFG(CFG* cfg, CalledMethods& PreCalledMethods, CalledMethods& PostCalledMethods, std::list<std::string> branchesAnalyzed, AliasMap& AliasedVars, std::string priorBranch) {
  std::string currentBranch =  cfg->getBranchName(); 

  std::string m; 
  for (auto k : branchesAnalyzed) {
    m += k + ", ";
  }
  logout("branches analyzed = " << m)

  if (currentBranch == "entry") { 

    PreCalledMethods[currentBranch] = {}; 
    llvm::SetVector<Instruction*> instructions = cfg->getInstructions(); 

    for (Instruction* I : instructions) {
      transferUnbranched(I, instructions, PostCalledMethods[currentBranch], AliasedVars);
    }

    branchesAnalyzed.push_back(currentBranch); 

    for (CFG* succ : cfg->getSuccessors()) {
      analyzeCFG(succ, PreCalledMethods, PostCalledMethods, branchesAnalyzed, AliasedVars, currentBranch); 
    }

  }

  else {
    std::map<std::string, CM> PriorPreCM = std::map<std::string, CM>(PreCalledMethods[currentBranch]); 
    std::map<std::string, CM> PriorPostCM = std::map<std::string, CM>(PostCalledMethods[currentBranch]); 

    bool foundCurrent = std::find(branchesAnalyzed.begin(), branchesAnalyzed.end(), currentBranch) != branchesAnalyzed.end();

    logout("-----\nlogging priorprecm prior = " << priorBranch << " current = " << currentBranch )
    for (auto p : PriorPreCM) {
      std::string m; 
      for (auto k : p.second.cmSet) {
        m += k + ", ";
      }
      logout(p.first << " " << m)
    }


    if (PriorPreCM.size() > 0) {
      logout("need to lub for " << currentBranch << " " << priorBranch)
      

      std::map<std::string, CM> CurrentPreCM = std::map<std::string, CM>(PostCalledMethods[priorBranch]);
      llvm::SetVector<Instruction*> instructions = cfg->getInstructions(); 
     

      // lub PriorPreCM and CurrentPreCM
      std::map<std::string, CM> lub; 

      for (auto Pair1 : PriorPreCM) {
        std::string varName = Pair1.first; 
        CM priorPreCM = Pair1.second; 
        
          
        // intersection 
        std::set<std::string> intersection;

        std::set_intersection(priorPreCM.cmSet.begin(), priorPreCM.cmSet.end(), 
                      CurrentPreCM[varName].cmSet.begin(), CurrentPreCM[varName].cmSet.end(), 
                      std::inserter(intersection, intersection.begin()));

        logout("for var name " << varName)

        std::string m,k; 
        for (auto i : priorPreCM.cmSet) {
          m += i + ", ";
        }
        for (auto i : CurrentPreCM[varName].cmSet) {
          k += i + ", ";
        }
        logout("prior pre cm = " << m)
        logout("current pre cm = " << k)


        lub[varName] = {
          intersection, 
          true
        }; 
        
      
      }

      // fill in the rest 
      for (auto Pair1 : PriorPostCM) {
        std::string varName = Pair1.first; 
        CM priorPostCM = Pair1.second; 

        if (!lub[varName].setInitialized) {
          
          lub[varName] = {
            std::set<std::string>(priorPostCM.cmSet), 
            true 
          }; 

        }
      }

      PreCalledMethods[currentBranch] = std::map<std::string, CM>(lub);

      for (Instruction* I : instructions) {
        transferUnbranched(I, instructions, lub, AliasedVars);
      }

      PostCalledMethods[currentBranch] = lub; 

      branchesAnalyzed.push_back(currentBranch); 

      for (CFG* succ : cfg->getSuccessors()) {
        bool found = std::find(branchesAnalyzed.begin(), branchesAnalyzed.end(), succ->getBranchName()) != branchesAnalyzed.end();
        if (!found) {
          logout("analyzing succesor " << succ->getBranchName())
          analyzeCFG(succ, PreCalledMethods, PostCalledMethods, branchesAnalyzed, AliasedVars, currentBranch); 
        }
        else {
          logout("not analyzing successor " << succ->getBranchName())
        }
      }

    }
    else {
      logout("doing normal flow for " << currentBranch << " and prior = " << priorBranch)  

      std::map<std::string, CM> priorPostCM = std::map<std::string, CM>(PostCalledMethods[priorBranch]); 

      PreCalledMethods[currentBranch] = std::map<std::string, CM>(priorPostCM); 
      llvm::SetVector<Instruction*> instructions = cfg->getInstructions(); 

      std::map<std::string, CM> flowInto = std::map<std::string, CM>(PostCalledMethods[priorBranch]); 

       for (Instruction* I : instructions) {
        transferUnbranched(I, instructions, flowInto, AliasedVars);
      }

      PostCalledMethods[currentBranch] = flowInto;

      branchesAnalyzed.push_back(currentBranch); 
      

      for (CFG* succ : cfg->getSuccessors()) {
        analyzeCFG(succ, PreCalledMethods, PostCalledMethods, branchesAnalyzed, AliasedVars, currentBranch); 
      }

    }

  }

}


void CalledMethodsAnalysis::doAnalysis(Function &F, PointerAnalysis *PA) {
  SetVector<Instruction *> WorkSet;
  SetVector<Value *> PointerSet;

  
  std::string fnName = F.getName().str(); 
  bool functionIsKnown = false; 
  logout("fnname = " << fnName)

  loadFunctions();

  // check if code re-defines a safe function or memory function. if so, cast it as a unsafe function
  if (SafeFunctions[fnName]) {
    SafeFunctions[fnName] = false; 
    UnsafeFunctions[fnName] = true; 
    functionIsKnown = true; 
    errs() << "CME-WARNING: Re-definition of safe function '" << fnName << "' identified on  and will be labelled as unsafe.\n";
  }

  bool ALLOW_REDEFINE = true;  // ONLY for debugging purposes 

  if (!ALLOW_REDEFINE) {
    for (auto Pair : MemoryFunctions) {
      if (fnName == Pair.first) {
        UnsafeFunctions[fnName] = true; 
        functionIsKnown = true; 
        errs() << "**CME-WARNING**: Re-definition of allocation function '" << fnName << "' identified and will be labelled as unsafe.\n";
      }

      if (fnName == Pair.second) {
        UnsafeFunctions[fnName] = true; 
        functionIsKnown = true; 
        errs() << "**CME-WARNING**: Re-definition of deallocation function '" << fnName << "' identified and will be labelled as unsafe.\n";
      }
    }
  }

  if (fnName == "main") {
    functionIsKnown = true; 
  }

  if (!functionIsKnown && !ALLOW_REDEFINE) {
    errs() << "CME-WARNING: Unknown function '" << fnName << "' identified and will be labelled as unsafe.\n";
    UnsafeFunctions[fnName] = true; 
  }

  

  if (fnName != "main") return; 
  
  
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    WorkSet.insert(&(*I));
    PointerSet.insert(&(*I));
    logout("inst = " << *I);
    logout("block = " << I->getParent()->getName())
    logout("preds = ")
    std::vector<Instruction *> preds = getPredecessors(&(*I));
    for (auto pred : preds) {
      logout(pred->getParent()->getName())
    }
    logout("end of preds")

  }

  CalledMethods calledMethodsEstimate; 
  AliasMap AliasedVars; 
  std::map<std::string, SetVector<Instruction*>> branchInstMap; 

  for (auto I : WorkSet) {
    std::string branchName = I->getParent()->getName().str();
    branchInstMap[branchName].insert(I); 

    transfer(I, WorkSet, calledMethodsEstimate, AliasedVars);

  }


  for (auto branch : realBranchOrder) {
    errs() << "branch name = " << branch << "\n";
    for (auto Pair1 : calledMethodsEstimate[branch]) {


      errs() << "> var name = " << Pair1.first << "\n";
      errs() << ">> cm = ";
      std::string cm; 
      for (auto s : Pair1.second.cmSet) {
        cm += s + ", ";
      }
      errs() << cm << "\n"; 
    }
    errs() << "\n";
  }


  CFG TopCFG = CFG("entry"); 

  for (auto I : WorkSet) {
   std::string branchName = I->getParent()->getName().str(); 
   auto preds = getPredecessors(I);
   auto succs = getSuccessors(I); 


   std::string predsString;
   std::string succsString;

   CFG* cfg = TopCFG.getFind(I->getParent()->getName().str()); 

   cfg->setInstructions(branchInstMap[I->getParent()->getName().str()]);

   for (auto p : preds) {
    std::string p_name = p->getParent()->getName().str();
    predsString += p_name + ", ";    
   }

   for (auto p : succs) {
    std::string s_name = p->getParent()->getName().str();
    succsString += s_name + ", ";
   }

   if (branchName == succsString) succsString = ""; 
   if (branchName == predsString) predsString = ""; 
   if (succsString == predsString) continue; 
  

   for (auto p : succs) {
    std::string s_name = p->getParent()->getName().str();

    if (s_name == branchName) continue; 

    if (cfg->checkFind(s_name)) {
      cfg->addSuccessor(cfg->getFind(s_name));
      cfg->getFind(s_name)->addPredecessor(cfg);
      continue; 
    } 

    cfg->addSuccessor(s_name); 
    cfg->getFind(s_name)->addPredecessor(cfg); 
  

   }

  }

  CalledMethods PreCalledMethods; 
  CalledMethods PostCalledMethods; 
  std::list<std::string> branchesAnalyzed; 

  analyzeCFG(&TopCFG, PreCalledMethods, PostCalledMethods, branchesAnalyzed, AliasedVars, ""); 


  logout("\n\nPOST CALLED METHODS")
  for (auto Pair1 : PostCalledMethods) {
    std::string branchName = Pair1.first; 
    logout("branch = " << branchName)
    for (auto Pair2 : Pair1.second) {
      std::string cm; 
      for (auto m : Pair2.second.cmSet) {
        cm += m + ", "; 
      }
      logout(">> var name = " << Pair2.first << " cm = " << cm)
    }
  }

  logout("\n\nLAST BRANCH CALLED METHODS = " << realBranchOrder.back())
  for (auto Pair : PostCalledMethods[realBranchOrder.back()]) {

      std::string cm; 
      for (auto m : Pair.second.cmSet) {
        cm += m + ", "; 
      }
      logout(">> var name = " << Pair.first << " cm = " << cm)
    

  }



}

} // namespace dataflow