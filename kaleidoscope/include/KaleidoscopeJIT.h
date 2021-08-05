//===----- KaleidoscopeJIT.h - A simple JIT for Kaleidoscope ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Contains a simple JIT definition for use in the kaleidoscope tutorials.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_EXECUTIONENGINE_ORC_KALEIDOSCOPEJIT_H
#define LLVM_EXECUTIONENGINE_ORC_KALEIDOSCOPEJIT_H

#include "llvm/ADT/iterator_range.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JITSymbolFlags.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/RuntimeDyld.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/LambdaResolver.h"
#include "llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Mangler.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
/**
 * JIT 分为两个阶段，在运行时生成机器码和执行机器码
 * 基本原理：在堆上分配可读可写可执行的内存块，将JIT生成的机器码拷贝到那个内存块，将内存块转换为指针类型执行
 * JIT 引擎的 ExecutionManager 类调用LLVM代码生成器，产生目标平台机器指令的二进制代码保存在内存中，并返回指向编译后函数的指针
 * 然后通过函数指针指向指令所在内存区域即可指向该函数。在此过程内存管理负责指向内存分配、释放、权限处理、库加载空间分配等操作
**/

namespace llvm
{
  namespace orc
  {

    class KaleidoscopeJIT
    {
    public:
      typedef ObjectLinkingLayer<> ObjLayerT;
      typedef IRCompileLayer<ObjLayerT> CompileLayerT;
      typedef CompileLayerT::ModuleSetHandleT ModuleHandleT;

      KaleidoscopeJIT()
          : TM(EngineBuilder().selectTarget()), DL(TM->createDataLayout()),
            CompileLayer(ObjectLayer, SimpleCompiler(*TM))
      {
        llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
      }

      TargetMachine &getTargetMachine() { return *TM; }

      ModuleHandleT addModule(std::unique_ptr<Module> M)
      {
        // 我们需要一个内存管理器来分配内存和为这个新模块解析符号。创建一个解析器在JIT中搜索
        auto Resolver = createLambdaResolver(
            [&](const std::string &Name)
            {
              if (auto Sym = findMangledSymbol(Name))
                return Sym.toRuntimeDyldSymbol();
              return RuntimeDyld::SymbolInfo(nullptr);
            },
            [](const std::string &S)
            { return nullptr; });
        auto H = CompileLayer.addModuleSet(singletonSet(std::move(M)),
                                           make_unique<SectionMemoryManager>(),
                                           std::move(Resolver));

        ModuleHandles.push_back(H);
        return H;
      }

      void removeModule(ModuleHandleT H)
      {
        ModuleHandles.erase(
            std::find(ModuleHandles.begin(), ModuleHandles.end(), H));
        CompileLayer.removeModuleSet(H);
      }

      JITSymbol findSymbol(const std::string Name)
      {
        return findMangledSymbol(mangle(Name));
      }

    private:
      std::string mangle(const std::string &Name)
      {
        std::string MangledName;
        {
          raw_string_ostream MangledNameStream(MangledName);
          Mangler::getNameWithPrefix(MangledNameStream, Name, DL);
        }
        return MangledName;
      }

      template <typename T>
      static std::vector<T> singletonSet(T t)
      {
        std::vector<T> Vec;
        Vec.push_back(std::move(t));
        return Vec;
      }

      JITSymbol findMangledSymbol(const std::string &Name)
      {
        // 通过dlsym在已添加的模块从后往前找符号，目的是在REPL(循环解释执行)找到最新定义的符号
        for (auto H : make_range(ModuleHandles.rbegin(), ModuleHandles.rend()))
          if (auto Sym = CompileLayer.findSymbolIn(H, Name, true))
            return Sym;

        // 如果在JIT中找不到这个符号，就尝试在进程的符号表中找
        if (auto SymAddr = RTDyldMemoryManager::getSymbolAddressInProcess(Name))
          return JITSymbol(SymAddr, JITSymbolFlags::Exported);

        return nullptr;
      }

      std::unique_ptr<TargetMachine> TM;
      const DataLayout DL;
      ObjLayerT ObjectLayer;
      CompileLayerT CompileLayer;
      std::vector<ModuleHandleT> ModuleHandles;
    };

  } // end namespace orc
} // end namespace llvm

#endif // LLVM_EXECUTIONENGINE_ORC_KALEIDOSCOPEJIT_H
