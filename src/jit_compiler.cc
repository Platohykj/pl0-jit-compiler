#include "jit_compiler.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"

namespace pl0 {

using namespace peg::udl;
using namespace llvm;

void JITCompiler::run(const std::shared_ptr<AstPL0> ast) {
  JITCompiler jit;
  jit.compile(ast);
  jit.exec();
  // jit.dump();
}

JITCompiler::JITCompiler() : builder_(context_) {
  module_ = std::make_unique<Module>("pl0", context_);

  tyinfo_ =
      new GlobalVariable(*module_, builder_.getPtrTy(), true,
                         GlobalValue::ExternalLinkage, nullptr, "_ZTIPKc");
}

void JITCompiler::compile(const std::shared_ptr<AstPL0> ast) {
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  compile_libs();
  compile_program(ast);
}

void JITCompiler::exec() {
  std::unique_ptr<ExecutionEngine> ee(
      EngineBuilder(std::move(module_)).create());
  auto ret = ee->runFunction(ee->FindFunctionNamed("main"), {});
}

void JITCompiler::dump() { module_->print(llvm::outs(), nullptr); }

void JITCompiler::compile_switch(const std::shared_ptr<AstPL0> ast) {
  switch (ast->tag) {
    case "assignment"_:
      compile_assignment(ast);
      break;
    case "call"_:
      compile_call(ast);
      break;
    case "statements"_:
      compile_statements(ast);
      break;
    case "if"_:
      compile_if(ast);
      break;
    case "while"_:
      compile_while(ast);
      break;
    case "out"_:
      compile_out(ast);
      break;
    default:
      compile_switch(ast->nodes[0]);
      break;
  }
}

Value* JITCompiler::compile_switch_value(const std::shared_ptr<AstPL0> ast) {
  switch (ast->tag) {
    case "odd"_:
      return compile_odd(ast);
    case "compare"_:
      return compile_compare(ast);
    case "expression"_:
      return compile_expression(ast);
    case "ident"_:
      return compile_ident(ast);
    case "number"_:
      return compile_number(ast);
    default:
      return compile_switch_value(ast->nodes[0]);
  }
}

void JITCompiler::compile_libs() {
  // `out` function
  auto outFn =
      cast<Function>(module_
                         ->getOrInsertFunction("out", builder_.getVoidTy(),
                                               builder_.getInt32Ty())
                         .getCallee());

  {
    auto BB = BasicBlock::Create(context_, "entry", outFn);
    builder_.SetInsertPoint(BB);

    auto printFn = module_->getOrInsertFunction(
        "printf",
        FunctionType::get(builder_.getInt32Ty(),
                          PointerType::get(builder_.getInt8Ty(), 0), true));

    auto val = &*outFn->arg_begin();
    auto fmt = builder_.CreateGlobalStringPtr("%d\n", ".printf.fmt");
    builder_.CreateCall(printFn, {fmt, val});

    builder_.CreateRetVoid();
    verifyFunction(*outFn);
  }
}

void JITCompiler::compile_program(const std::shared_ptr<AstPL0> ast) {
  // `start` function
  auto startFn = cast<Function>(
      module_->getOrInsertFunction("__pl0_start", builder_.getVoidTy())
          .getCallee());

  {
    auto BB = BasicBlock::Create(context_, "entry", startFn);
    builder_.SetInsertPoint(BB);

    compile_block(ast->nodes[0]);

    builder_.CreateRetVoid();
    verifyFunction(*startFn);
  }

  // `main` function
  auto mainFn = cast<Function>(
      module_->getOrInsertFunction("main", builder_.getVoidTy()).getCallee());

  {
    auto personalityFn = Function::Create(
        FunctionType::get(builder_.getInt32Ty(), {}, true),
        GlobalValue::ExternalLinkage, "__gxx_personality_v0", module_.get());

    mainFn->setPersonalityFn(personalityFn);

    auto BB = BasicBlock::Create(context_, "entry", mainFn);
    builder_.SetInsertPoint(BB);

    auto fn = builder_.GetInsertBlock()->getParent();
    auto lpadBB = BasicBlock::Create(context_, "lpad", fn);
    auto endBB = BasicBlock::Create(context_, "end");
    builder_.CreateInvoke(startFn, endBB, lpadBB);

    builder_.SetInsertPoint(lpadBB);

    auto exc = builder_.CreateLandingPad(
        StructType::get(builder_.getPtrTy(), builder_.getInt32Ty()), 1, "exc");

    exc->addClause(ConstantExpr::getBitCast(tyinfo_, builder_.getPtrTy()));

    auto ptr = builder_.CreateExtractValue(exc, {0}, "exc.ptr");
    auto sel = builder_.CreateExtractValue(exc, {1}, "exc.sel");

    auto id = builder_.CreateCall(
        cast<Function>(module_
                           ->getOrInsertFunction("llvm.eh.typeid.for",
                                                 builder_.getInt32Ty(),
                                                 builder_.getPtrTy())
                           .getCallee()),
        {ConstantExpr::getBitCast(tyinfo_, builder_.getPtrTy())}, "tid.int");

    auto catch_with_message =
        BasicBlock::Create(context_, "catch_with_message", fn);
    auto catch_unknown = BasicBlock::Create(context_, "catch_unknown", fn);
    auto cmp = builder_.CreateCmp(CmpInst::ICMP_EQ, sel, id, "tst.int");
    builder_.CreateCondBr(cmp, catch_with_message, catch_unknown);

    auto beginCatchFn =
        cast<Function>(module_
                           ->getOrInsertFunction("__cxa_begin_catch",
                                                 builder_.getPtrTy(),
                                                 builder_.getPtrTy())
                           .getCallee());

    auto endCatchFn =
        module_->getOrInsertFunction("__cxa_end_catch", builder_.getVoidTy());

    auto putFn = module_->getOrInsertFunction("puts", builder_.getInt32Ty(),
                                              builder_.getPtrTy());

    {
      builder_.SetInsertPoint(catch_with_message);

      auto str = builder_.CreateCall(beginCatchFn, ptr, "str");
      builder_.CreateCall(putFn, str);
      builder_.CreateCall(endCatchFn);
      builder_.CreateBr(endBB);
    }

    {
      builder_.SetInsertPoint(catch_unknown);

      builder_.CreateCall(beginCatchFn, ptr);
      auto str =
          builder_.CreateGlobalStringPtr("unknown error...", ".str.unknown");
      builder_.CreateCall(putFn, str);
      builder_.CreateCall(endCatchFn);
      builder_.CreateBr(endBB);
    }

    {
      endBB->insertInto(fn);
      builder_.SetInsertPoint(endBB);

      builder_.CreateRetVoid();
    }

    verifyFunction(*mainFn);
  }
}

void JITCompiler::compile_block(const std::shared_ptr<AstPL0> ast) {
  compile_const(ast->nodes[0]);
  compile_var(ast->nodes[1]);
  compile_procedure(ast->nodes[2]);
  compile_statement(ast->nodes[3]);
}

void JITCompiler::compile_const(const std::shared_ptr<AstPL0> ast) {
  for (auto i = 0u; i < ast->nodes.size(); i += 2) {
    auto ident = ast->nodes[i]->token;
    auto number = ast->nodes[i + 1]->token_to_number<int>();

    auto alloca = builder_.CreateAlloca(builder_.getInt32Ty(), nullptr, ident);
    builder_.CreateStore(builder_.getInt32(number), alloca);
  }
}

void JITCompiler::compile_var(const std::shared_ptr<AstPL0> ast) {
  for (const auto node : ast->nodes) {
    auto ident = node->token;
    builder_.CreateAlloca(builder_.getInt32Ty(), nullptr, ident);
  }
}

void JITCompiler::compile_procedure(const std::shared_ptr<AstPL0> ast) {
  for (auto i = 0u; i < ast->nodes.size(); i += 2) {
    auto ident = ast->nodes[i]->token;
    auto block = ast->nodes[i + 1];

    std::vector<Type*> pt(block->scope->free_variables.size(),
                          PointerType::get(builder_.getInt32Ty(), 0));
    auto fn = cast<Function>(
        module_
            ->getOrInsertFunction(
                ident, FunctionType::get(builder_.getVoidTy(), pt, false))
            .getCallee());

    {
      auto it = block->scope->free_variables.begin();
      for (auto& arg : fn->args()) {
        auto& sv = *it;
        arg.setName(sv);
        ++it;
      }
    }

    {
      auto prevBB = builder_.GetInsertBlock();
      auto BB = BasicBlock::Create(context_, "entry", fn);
      builder_.SetInsertPoint(BB);
      compile_block(block);
      builder_.CreateRetVoid();
      verifyFunction(*fn);
      builder_.SetInsertPoint(prevBB);
    }
  }
}

void JITCompiler::compile_statement(const std::shared_ptr<AstPL0> ast) {
  if (!ast->nodes.empty()) {
    compile_switch(ast->nodes[0]);
  }
}

void JITCompiler::compile_assignment(const std::shared_ptr<AstPL0> ast) {
  auto ident = ast->nodes[0]->token;

  auto fn = builder_.GetInsertBlock()->getParent();
  auto tbl = fn->getValueSymbolTable();
  auto var = tbl->lookup(ident);
  if (!var) {
    throw_runtime_error(ast,
                        "'" + std::string(ident) + "' is not defined...");
  }

  auto val = compile_expression(ast->nodes[1]);
  builder_.CreateStore(val, var);
}

void JITCompiler::compile_call(const std::shared_ptr<AstPL0> ast) {
  auto ident = ast->nodes[0]->token;

  auto scope = get_closest_scope(ast);
  auto block = scope->get_procedure(ident);

  std::vector<Value*> args;
  for (auto& free : block->scope->free_variables) {
    auto fn = builder_.GetInsertBlock()->getParent();
    auto tbl = fn->getValueSymbolTable();
    auto var = tbl->lookup(free);
    if (!var) {
      throw_runtime_error(ast, "'" + std::string(free) + "' is not defined...");
    }
    args.push_back(var);
  }

  auto fn = module_->getFunction(ident);
  builder_.CreateCall(fn, args);
}

void JITCompiler::compile_statements(const std::shared_ptr<AstPL0> ast) {
  for (auto node : ast->nodes) {
    compile_statement(node);
  }
}

void JITCompiler::compile_if(const std::shared_ptr<AstPL0> ast) {
  auto cond = compile_condition(ast->nodes[0]);

  auto fn = builder_.GetInsertBlock()->getParent();
  auto ifTenBB = BasicBlock::Create(context_, "if.then", fn);
  auto ifEndBB = BasicBlock::Create(context_, "if.end");

  builder_.CreateCondBr(cond, ifTenBB, ifEndBB);

  builder_.SetInsertPoint(ifTenBB);
  compile_statement(ast->nodes[1]);
  builder_.CreateBr(ifEndBB);

  ifEndBB->insertInto(fn);
  builder_.SetInsertPoint(ifEndBB);
}

void JITCompiler::compile_while(const std::shared_ptr<AstPL0> ast) {
  auto whileCondBB = BasicBlock::Create(context_, "while.cond");
  builder_.CreateBr(whileCondBB);

  auto fn = builder_.GetInsertBlock()->getParent();
  whileCondBB->insertInto(fn);
  builder_.SetInsertPoint(whileCondBB);

  auto cond = compile_condition(ast->nodes[0]);

  auto whileBodyBB = BasicBlock::Create(context_, "while.body", fn);
  auto whileEndBB = BasicBlock::Create(context_, "while.end");
  builder_.CreateCondBr(cond, whileBodyBB, whileEndBB);

  builder_.SetInsertPoint(whileBodyBB);
  compile_statement(ast->nodes[1]);

  builder_.CreateBr(whileCondBB);

  whileEndBB->insertInto(fn);
  builder_.SetInsertPoint(whileEndBB);
}

Value* JITCompiler::compile_condition(const std::shared_ptr<AstPL0> ast) {
  return compile_switch_value(ast->nodes[0]);
}

Value* JITCompiler::compile_odd(const std::shared_ptr<AstPL0> ast) {
  auto val = compile_expression(ast->nodes[0]);
  return builder_.CreateICmpNE(val, builder_.getInt32(0), "icmpne");
}

Value* JITCompiler::compile_compare(const std::shared_ptr<AstPL0> ast) {
  auto lhs = compile_expression(ast->nodes[0]);
  auto rhs = compile_expression(ast->nodes[2]);

  auto ope = ast->nodes[1]->token;
  switch (ope[0]) {
    case '=':
      return builder_.CreateICmpEQ(lhs, rhs, "icmpeq");
    case '#':
      return builder_.CreateICmpNE(lhs, rhs, "icmpne");
    case '<':
      if (ope.size() == 1) {
        return builder_.CreateICmpSLT(lhs, rhs, "icmpslt");
      }
      // '<='
      return builder_.CreateICmpSLE(lhs, rhs, "icmpsle");
    case '>':
      if (ope.size() == 1) {
        return builder_.CreateICmpSGT(lhs, rhs, "icmpsgt");
      }
      // '>='
      return builder_.CreateICmpSGE(lhs, rhs, "icmpsge");
  }
  return nullptr;
}

void JITCompiler::compile_out(const std::shared_ptr<AstPL0> ast) {
  auto val = compile_expression(ast->nodes[0]);
  auto fn = module_->getFunction("out");
  builder_.CreateCall(fn, val);
}

Value* JITCompiler::compile_expression(const std::shared_ptr<AstPL0> ast) {
  const auto& nodes = ast->nodes;

  auto sign = nodes[0]->token;
  auto negative = !(sign.empty() || sign == "+");

  auto val = compile_term(nodes[1]);
  if (negative) {
    val = builder_.CreateNeg(val, "negative");
  }

  for (auto i = 2u; i < nodes.size(); i += 2) {
    auto ope = nodes[i + 0]->token[0];
    auto rval = compile_term(nodes[i + 1]);
    switch (ope) {
      case '+':
        val = builder_.CreateAdd(val, rval, "add");
        break;
      case '-':
        val = builder_.CreateSub(val, rval, "sub");
        break;
    }
  }
  return val;
}

Value* JITCompiler::compile_term(const std::shared_ptr<AstPL0> ast) {
  const auto& nodes = ast->nodes;
  auto val = compile_factor(nodes[0]);
  for (auto i = 1u; i < nodes.size(); i += 2) {
    auto ope = nodes[i + 0]->token[0];
    auto rval = compile_switch_value(nodes[i + 1]);
    switch (ope) {
      case '*':
        val = builder_.CreateMul(val, rval, "mul");
        break;
      case '/': {
        // Zero divide check
        auto cond = builder_.CreateICmpEQ(rval, builder_.getInt32(0), "icmpeq");

        auto fn = builder_.GetInsertBlock()->getParent();
        auto ifZeroBB = BasicBlock::Create(context_, "zdiv.zero", fn);
        auto ifNonZeroBB = BasicBlock::Create(context_, "zdiv.non_zero");
        builder_.CreateCondBr(cond, ifZeroBB, ifNonZeroBB);

        // zero
        {
          builder_.SetInsertPoint(ifZeroBB);

          Value* eh = nullptr;
          {
            auto fn = cast<Function>(
                module_
                    ->getOrInsertFunction("__cxa_allocate_exception",
                                          builder_.getPtrTy(),
                                          builder_.getInt64Ty())
                    .getCallee());

            eh = builder_.CreateCall(fn, builder_.getInt64(8), "eh");

            auto payload = builder_.CreateBitCast(eh, builder_.getPtrTy(), "payload");

            auto msg = builder_.CreateGlobalStringPtr(
                "divide by 0", ".str.zero_divide", 0, module_.get());

            builder_.CreateStore(msg, payload);
          }

          {
            auto fn = cast<Function>(
                module_
                    ->getOrInsertFunction("__cxa_throw", builder_.getVoidTy(),
                                          builder_.getPtrTy(),
                                          builder_.getPtrTy(),
                                          builder_.getPtrTy())
                    .getCallee());

            builder_.CreateCall(
                fn,
                {eh, ConstantExpr::getBitCast(tyinfo_, builder_.getPtrTy()),
                 ConstantPointerNull::get(builder_.getPtrTy())});
          }

          builder_.CreateUnreachable();
        }

        // no_zero
        {
          ifNonZeroBB->insertInto(fn);
          builder_.SetInsertPoint(ifNonZeroBB);
          val = builder_.CreateSDiv(val, rval, "div");
        }
        break;
      }
    }
  }
  return val;
}

Value* JITCompiler::compile_factor(const std::shared_ptr<AstPL0> ast) {
  return compile_switch_value(ast->nodes[0]);
}

Value* JITCompiler::compile_ident(const std::shared_ptr<AstPL0> ast) {
  auto ident = ast->token;

  auto fn = builder_.GetInsertBlock()->getParent();
  auto tbl = fn->getValueSymbolTable();
  auto var = tbl->lookup(ident);
  if (!var) {
    throw_runtime_error(ast,
                        "'" + std::string(ident) + "' is not defined...");
  }

  return builder_.CreateLoad(builder_.getInt32Ty(), var);
}

Value* JITCompiler::compile_number(const std::shared_ptr<AstPL0> ast) {
  return ConstantInt::getIntegerValue(builder_.getInt32Ty(),
                                      APInt(32, ast->token, 10));
}

}  // namespace pl0
