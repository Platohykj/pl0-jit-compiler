//
//  main.cc - PL/0 JIT compiler entry point
//
//  Copyright © 2018 Yuji Hirose. All rights reserved.
//  MIT License
//

#include "grammar.h"
#include "jit_compiler.h"
#include "symbol_table.h"
#include "utils.h"
#include <peglib.h>

#include <fstream>
#include <iostream>
#include <vector>

using namespace pl0;
using namespace peg;

int main(int argc, const char** argv) {
  if (argc < 2) {
    std::cout << "usage: pl0 file" << std::endl;
    return 1;
  }

  // Source file path
  auto path = argv[1];

  // Read a source file into memory
  std::vector<char> source;
  std::ifstream ifs(path, std::ios::in | std::ios::binary);
  if (ifs.fail()) {
    std::cerr << "can't open the source file." << std::endl;
    return -1;
  }

  source.resize(static_cast<unsigned int>(ifs.seekg(0, std::ios::end).tellg()));
  if (!source.empty()) {
    ifs.seekg(0, std::ios::beg)
        .read(&source[0], static_cast<std::streamsize>(source.size()));
  }

  // Setup a PEG parser
  parser parser(grammar);
  parser.enable_ast<AstPL0>();
  parser.set_logger([&](size_t ln, size_t col, const std::string& msg) {
    std::cerr << format_error_message(path, ln, col, msg) << std::endl;
  });

  // Parse the source and make an AST
  std::shared_ptr<AstPL0> ast;
  if (parser.parse_n(source.data(), source.size(), ast, path)) {
    try {
      // Make a symbol table on the AST
      SymbolTableBuilder::build_on_ast(ast);

      // JIT compile and execute
      JITCompiler::run(ast);
    } catch (const std::runtime_error& e) {
      std::cerr << e.what() << std::endl;
    }
    return 0;
  }

  return -1;
}
