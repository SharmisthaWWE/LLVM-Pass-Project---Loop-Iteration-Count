#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/TypeBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/SimplifyIndVar.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Verifier.h"

using namespace llvm;
using namespace std;


namespace{

    //printf function creation
    static Function* printHelper(LLVMContext& theContext, Module *newModule, Constant* format)
    {
        FunctionType *printType = TypeBuilder<int(char *, ...), false>::get(theContext);

        Function *Func = cast<Function>(newModule->getOrInsertFunction("printf", printType, AttributeSet().addAttribute(newModule->getContext(), 1U, Attribute::NoAlias)));

        return Func;
    }

    //string structure of the printf function
    Constant* geti8StrVal(LLVMContext& theContext, Module& M, char const* str, Twine const& name) {
          LLVMContext& ctx = theContext;
          Constant* strConstant = ConstantDataArray::getString(ctx, str);
          GlobalVariable* GVStr = new GlobalVariable(M, strConstant->getType(), true, GlobalValue::InternalLinkage, strConstant, name);
          Constant* zero = Constant::getNullValue(IntegerType::getInt32Ty(ctx));

          std::vector<Constant*> indices;
          indices.push_back(zero);
          indices.push_back(zero);

          Constant* strVal = ConstantExpr::getGetElementPtr(strConstant->getType(), GVStr, indices, true);   //  (GVStr, indices, true);
          return strVal;
        }

    //LIC pass
    struct LoopIter : public FunctionPass{
        static char ID;

        int counts = 0;
        LoopIter() : FunctionPass(ID) {}

        void getAnalysisUsage(AnalysisUsage &AU) const override {

            AU.addRequired<DominatorTreeWrapperPass>();
            AU.addPreserved<DominatorTreeWrapperPass>();
            AU.addRequired<LoopInfoWrapperPass>();
            AU.addPreserved<LoopInfoWrapperPass>();
            AU.addRequired<ScalarEvolutionWrapperPass>();
            AU.addPreserved<ScalarEvolutionWrapperPass>();
            AU.setPreservesAll();
        }

        virtual bool runOnFunction(Function&F){

            LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

            int countLoop = 0;

            Module *newModule1 = F.getParent();
            newModule1->setDataLayout("e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-"
                    "i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-"
                    "a0:0:64-s0:64:64-f80:128:128");
            newModule1->setTargetTriple("x86_64-unknown-linux-gnu");

            //Iteration on loops
            for(LoopInfo::iterator i = LI.begin(), e = LI.end(); i != e; ++i)
            {
                Loop *L = *i;
                int subloop = 1;
                countLoop++;

                static LLVMContext Context;
                BasicBlock *parentLoopPred = L->getLoopPredecessor();
                BasicBlock *parentLoopHead = L->getHeader();

                std::vector<Loop*> subLoopAll = L->getSubLoops();
                Loop::iterator j, f;

                //Parent Loop Calculation
                //Counter = 0 instruction insertion
                Value *one1 = ConstantInt::get(Type::getInt32Ty(Context), -1, true);
                Instruction *ACounter1 = new AllocaInst(Type::getInt32Ty(Context), 0, 4, "counter");  //(IntegerType::get(mContext, 32), one, "counter", loopPredecessor);  //(IntegerType::get(mContext, 32), IR.getInt32(0), "counter");
                parentLoopPred->getInstList().insert(parentLoopPred->getFirstInsertionPt(), ACounter1);

                Instruction *SCounter1 = new StoreInst(one1, ACounter1);
                SCounter1->insertAfter(ACounter1);

                //Counter++ instruction insertion
                Value *IValue1 = ConstantInt::get(Type::getInt32Ty(Context), 1, true);
                Instruction *CValue1 = new LoadInst(ACounter1, "midCount");
                parentLoopHead->getInstList().insert(parentLoopHead->getFirstInsertionPt(), CValue1);

                Instruction *NValue1 = BinaryOperator::CreateNSWAdd(CValue1, IValue1, "incremented");//  Instruction::Add, CValue, IValue, "");
                NValue1->insertAfter(CValue1);

                Instruction *SNValue1 = new StoreInst(NValue1, ACounter1);
                SNValue1->insertAfter(NValue1);

                Value *PValue1 = SNValue1->getOperand(0);
                std::vector<Value *> printArrayRef1;

                Value *PValue12 = ConstantInt::get(Type::getInt32Ty(Context), subLoopAll.size(), true);
                std::vector<Value *> printArrayRef12;

                //Print information collection
                string functionName1 = "\nIn Function @" + F.getName().str() + "\nLIC = %d\n";
                char* funcName1 = new char[functionName1.size() + 1];
                std::copy(functionName1.begin(), functionName1.end(), funcName1);
                funcName1[functionName1.size()] = '\0';

                //Insertion of printf function in all exit blocks
                SmallVector<BasicBlock*, 8> blocks1;

                L->getExitBlocks(blocks1);

                SmallSetVector<BasicBlock *, 8> ExitBlockSet1(blocks1.begin(), blocks1.end());

                for(SmallSetVector<BasicBlock *, 8>::iterator ib = ExitBlockSet1.begin(), eb = ExitBlockSet1.end(); ib != eb; ++ib)
                {
                    BasicBlock *printCount1 = *ib;

                    Constant *stringFormat1;
                    if(subLoopAll.size() > 0) stringFormat1 = geti8StrVal(newModule1->getContext(), *newModule1, "\nParent LIC = %d\n", ".str");
                    else stringFormat1 = geti8StrVal(newModule1->getContext(), *newModule1, funcName1, ".str");

                    Function *printStructure1 = printHelper(newModule1->getContext(), newModule1, stringFormat1);

                    printArrayRef1.push_back(stringFormat1);
                    printArrayRef1.push_back(PValue1);

                    Instruction *printCall1 = CallInst::Create(printStructure1, printArrayRef1, "call");
                    printCount1->getInstList().insert(printCount1->getFirstInsertionPt(), printCall1);
                }
                delete[] funcName1;

                //Subloop Calculation
                if(subLoopAll.size() > 0)
                {
                    if(subloop <= 1)
                    {
                        string functionName = "\nIn Function @" + F.getName().str() + "\nParent Loop has = %d subloop(s).\n";
                        char* funcName = new char[functionName.size() + 1];
                        std::copy(functionName.begin(), functionName.end(), funcName);
                        funcName[functionName.size()] = '\0';

                        Constant *stringFormat12 = geti8StrVal(newModule1->getContext(), *newModule1, funcName, ".str");
                        Function *printStructure12 = printHelper(newModule1->getContext(), newModule1, stringFormat12);

                        delete[] funcName;
                        printArrayRef12.push_back(stringFormat12);
                        printArrayRef12.push_back(PValue12);

                        Instruction *printCall12 = CallInst::Create(printStructure12, printArrayRef12, "call");
                        printCall12->insertAfter(SCounter1);

                        //Subloop Serial Print
                        Value *PValueS = ConstantInt::get(Type::getInt32Ty(Context), subloop, true);
                        std::vector<Value *> printArrayRefS;

                        Constant *stringFormatS = geti8StrVal(newModule1->getContext(), *newModule1, "\nIteration count for SubLoop = %d\n", ".str");
                        Function *printStructureS = printHelper(newModule1->getContext(), newModule1, stringFormatS);

                        printArrayRefS.push_back(stringFormatS);
                        printArrayRefS.push_back(PValueS);

                        Instruction *printCallS = CallInst::Create(printStructureS, printArrayRefS, "call");       //(printFunction, printArrayRef, "Jao"); //printType, printFunction, , );
                        printCallS->insertAfter(printCall12);
                    }

                    else{
                        //Subloop Serial Print
                        Value *PValueS = ConstantInt::get(Type::getInt32Ty(Context), subloop, true);
                        std::vector<Value *> printArrayRefS;

                        Constant *stringFormatS = geti8StrVal(newModule1->getContext(), *newModule1, "\nIteration count for SubLoop = %d\n", ".str");
                        Function *printStructureS = printHelper(newModule1->getContext(), newModule1, stringFormatS);

                        printArrayRefS.push_back(stringFormatS);
                        printArrayRefS.push_back(PValueS);

                        Instruction *printCallS = CallInst::Create(printStructureS, printArrayRefS, "call");       //(printFunction, printArrayRef, "Jao"); //printType, printFunction, , );
                        printCallS->insertAfter(SCounter1);
                    }
                }

                for(j = subLoopAll.begin(), f = subLoopAll.end(); j != f; ++j)
                {
                    subloop++;
                    Loop *p = *j;

                    BasicBlock *loopPredecessor = p->getLoopPredecessor();
                    BasicBlock *loopHeader = p->getHeader();

                    //Counter = 0 instruction insertion
                    static LLVMContext mContext;

                    Value *one = ConstantInt::get(Type::getInt32Ty(mContext), -1, true);
                    Instruction *ACounter = new AllocaInst(Type::getInt32Ty(mContext), 0, 4, "counter");  //(IntegerType::get(mContext, 32), one, "counter", loopPredecessor);  //(IntegerType::get(mContext, 32), IR.getInt32(0), "counter");
                    loopPredecessor->getInstList().insert(loopPredecessor->getFirstInsertionPt(), ACounter);

                    Instruction *SCounter = new StoreInst(one, ACounter);
                    SCounter->insertAfter(ACounter);

                    //Counter++ instruction insertion
                    Value *IValue = ConstantInt::get(Type::getInt32Ty(mContext), 1, true);
                    Instruction *CValue = new LoadInst(ACounter, "midCount");
                    loopHeader->getInstList().insert(loopHeader->getFirstInsertionPt(), CValue);

                    Instruction *NValue = BinaryOperator::CreateNSWAdd(CValue, IValue, "incremented");//  Instruction::Add, CValue, IValue, "");
                    NValue->insertAfter(CValue);

                    Instruction *SNValue = new StoreInst(NValue, ACounter);
                    SNValue->insertAfter(NValue);

                    Value *PValue = SNValue->getOperand(0);
                    std::vector<Value *> printArrayRef;

                    //Print information collection
                    SmallVector<BasicBlock*, 8> blocks;

                    p->getExitBlocks(blocks);

                    SmallSetVector<BasicBlock *, 8> ExitBlockSet(blocks.begin(), blocks.end());

                    for(SmallSetVector<BasicBlock *, 8>::iterator ib = ExitBlockSet.begin(), eb = ExitBlockSet.end(); ib != eb; ++ib)
                    {
                        BasicBlock *printCount = *ib;

                        Module *newModule = F.getParent();
                        newModule->setDataLayout("e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-"
                                "i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-"
                                "a0:0:64-s0:64:64-f80:128:128");
                        newModule->setTargetTriple("x86_64-unknown-linux-gnu");

                        Constant *stringFormat = geti8StrVal(newModule->getContext(), *newModule, "Sub LIC = %d\n", ".str");
                        Function *printStructure = printHelper(newModule->getContext(), newModule, stringFormat);

                        printArrayRef.push_back(stringFormat);
                        printArrayRef.push_back(PValue);

                        Instruction *printCall = CallInst::Create(printStructure, printArrayRef, "call");
                        printCount->getInstList().insert(printCount->getFirstInsertionPt(), printCall);
                    }
                }
            }

            return false;
        }
    };
}

char LoopIter::ID = 0;
static RegisterPass<LoopIter> X("LoopCount", "Counts number of loops per program", false, false);
