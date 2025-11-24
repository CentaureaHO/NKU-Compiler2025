#include <frontend/parser/parser.h>
#include <frontend/ast/ast.h>
#include <frontend/ast/visitor/printer/ast_printer.h>
#include <frontend/symbol/symbol_table.h>
#include <frontend/ast/visitor/sementic_check/ast_checker.h>

#include <middleend/visitor/codegen/ast_codegen.h>
#include <middleend/visitor/printer/module_printer.h>
#include <middleend/module/ir_module.h>
#include <middleend/pass/unify_return.h>

#include <backend/mir/m_module.h>
#include <backend/target/registry.h>
#include <backend/target/target.h>

#include <fstream>
#include <iostream>
#include <iomanip>

/* 如果你简化了框架的实现, 或者解决了框架现存的问题
   或者是用现代C++特性对框架进行了重构, 并且有效地简化了代码或者提高了代码的复用性
   我们鼓励你与助教进行联系或发起pull request, 也许你的改进会被用到下一年的实验框架中
*/
/*
   本学期的编译作业还对你的编译器编译速度有一定要求，编译大部分用例的时间不能超过5s，一些大型的测试用例不得超过30s
   你需要在实现时注意数据结构的选择以及算法的实现细节
   额外的，虽然不做硬性要求，但还是建议在作业实现过程中注意内存管理，避免内存泄漏与其它问题

   Makefile 中提供了 format 目标，如果你安装了 clang-format 工具，可以通过 make format 来格式化代码
       格式化规则可在 .clang-format 文件中查看

    mem_track.sh 脚本可用于调用 valgrind 来检测内存泄漏问题
*/

#define STR_PW 30
#define INT_PW 8
#define MIN_GAP 5
#define STR_REAL_WIDTH (STR_PW - MIN_GAP)

using namespace std;

string truncateString(const string& str, size_t width)
{
    if (str.length() > width) return str.substr(0, width - 3) + "...";
    return str;
}

int main(int argc, char** argv)
{
    string   inputFile     = "";
    string   outputFile    = "";
    string   step          = "-llvm";
    string   march         = "riscv64";
    int      optimizeLevel = 0;
    ostream* outStream     = &cout;
    ofstream outFile;

    for (int i = 1; i < argc; i++)
    {
        string arg = argv[i];

        if (arg == "-lexer" || arg == "-parser" || arg == "-llvm" || arg == "-S") { step = arg; }
        else if (arg == "-o")
        {
            if (i + 1 < argc)
                outputFile = argv[++i];
            else
            {
                cerr << "Error: -o option requires a filename" << endl;
                return 1;
            }
        }
        else if (arg == "-march")
        {
            if (i + 1 < argc)
                march = argv[++i];
            else
            {
                cerr << "Error: -march option requires a target (e.g., riscv64)" << endl;
                return 1;
            }
        }
        else if (arg == "-O" || arg == "-O1") { optimizeLevel = 1; }
        else if (arg == "-O0") { optimizeLevel = 0; }
        else if (arg == "-O2") { optimizeLevel = 2; }
        else if (arg == "-O3") { optimizeLevel = 3; }
        else if (arg[0] != '-') { inputFile = arg; }
        else
        {
            cerr << "Unknown option: " << arg << endl;
            return 1;
        }
    }

    if (inputFile.empty())
    {
        cerr << "Error: No input file specified" << endl;
        cerr << "Usage: " << argv[0] << " [-lexer|-parser|-llvm|-S] [-o output_file] input_file [-O]" << endl;
        return 1;
    }

    if (!outputFile.empty())
    {
        outFile.open(outputFile);
        if (!outFile)
        {
            cerr << "Cannot open output file " << outputFile << endl;
            return 1;
        }
        outStream = &outFile;
    }

    cout << "Input file: " << inputFile << endl;
    cout << "Step: " << step << endl;
    cout << "Output: " << (outputFile.empty() ? "standard output" : outputFile) << endl;
    cout << "Optimize level: " << optimizeLevel << endl;

    ifstream       in(inputFile);
    istream*       inStream = &in;
    FE::AST::Node* ast      = nullptr;
    int            ret      = 0;

    if (!in)
    {
        cerr << "Cannot open input file " << inputFile << endl;
        ret = 1;
        goto cleanup_outfile;
    }

    /*
     * Lab 1: 词法分析
     *
     * 本实验的目标是实现词法分析器，将源代码文本流转换为 Token 序列。
     *
     * 主要任务:
     * - 在 `frontend/parser/parser.cpp` 中为 `FE::Parser::parseTokens_impl` 方法提供具体实现。
     * - `iParser` 接口 (位于 `interfaces/frontend/iparser.h`) 使用 CRTP 模式，
     *   其实际功能由派生类 `FE::Parser` 完成。
     * - 如果你希望尝试其它方法来完成本次实验，也可以自行从 `iParser` 派生出新的类来实现。
     *
     * 建议实现流程 (使用 Flex):
     * 1. `frontend/parser/yacc.y`: 声明词法分析器需要识别的 Token 列表。
     * 2. `frontend/parser/lexer.l`: 为 Token 编写正则表达式匹配规则和处理逻辑。
     * 3. `frontend/parser/parser.cpp`: 调用 Flex 生成的分析器，将结果转换为自定义的 `Token` 结构体。
     *
     * 注意事项:
     * - Bison 警告: 在完成语法分析前，编译 `yacc.y` 时产生的警告可暂时忽略。
     * - 负数处理: 词法分析阶段应将负数视为一个减号 (`-`) Token 和一个正数 Token 的组合，
     *   而非单个负数 Token。
     *
     * 期望输出示例:
     * 在 `testcase/lexer/` 目录下提供了一些测试用例以及它们的预期输出，可以自行查看。
     */
    {
        FE::Parser parser(inStream, outStream);

        if (step == "-lexer")
        {
            auto tokens = parser.parseTokens();

            *outStream << left;
            *outStream << setw(STR_PW) << "Token" << setw(STR_PW) << "Lexeme" << setw(STR_PW) << "Property"
                       << setw(INT_PW) << "Line" << setw(INT_PW) << "Column" << endl;

            for (auto& token : tokens)
            {
                *outStream << setw(STR_PW) << truncateString(token.token_name, STR_REAL_WIDTH) << setw(STR_PW)
                           << truncateString(token.lexeme, STR_REAL_WIDTH);

                if (token.type == FE::Token::TokenType::T_INT)
                    *outStream << setw(STR_PW) << token.ival;
                else if (token.type == FE::Token::TokenType::T_LL)
                    *outStream << setw(STR_PW) << token.lval;
                else if (token.type == FE::Token::TokenType::T_FLOAT)
                    *outStream << setw(STR_PW) << token.fval;
                else if (token.type == FE::Token::TokenType::T_DOUBLE)
                    *outStream << setw(STR_PW) << token.dval;
                else if (token.type == FE::Token::TokenType::T_STRING)
                    *outStream << setw(STR_PW) << token.sval;
                else
                    *outStream << setw(STR_PW) << " ";

                *outStream << setw(INT_PW) << token.line_number << setw(INT_PW) << token.column_number << endl;
            }

            ret = 0;
            goto cleanup_files;
        }

        /*
         * Lab 2: 语法分析 (Syntax Analysis)
         *
         * 本实验的目标是实现语法分析器，将 Token 序列转换为抽象语法树 (AST)。
         * 语法规则可以参考 `doc/SysY2022语言定义-V1.pdf`，或自行设计。
         * 所实现的语法规则应当可以覆盖 SysY 语言的语法结构。
         *
         * 主要任务:
         * - 构建 AST: 在 `frontend/parser/yacc.y` 中定义语法规则以生成 AST。
         * - AST 成员: 在生成 AST 节点时，正确设置其 `Entry* entry` 成员，
         *   使其指向对应的符号表项。同时正确设置其它属性，如节点的类型与字面量等。
         *
         * 相关文件:
         * - `frontend/parser/yacc.y`: Bison 语法规则定义。
         * - `frontend/ast/`: AST 节点定义目录 (`ast.h`, `expr.h`, `decl.h`, `stmt.h`)。
         * - `interfaces/frontend/symbol/symbol_entry.h`: 符号表项定义。
         *
         * 提示:
         * - AST 的打印功能已提供，位于 `frontend/ast/visitor/printer/`。
         *   可以通过以下方式来使用它。在后续的实验中也会使用到类似的访问者模式，你也可以使用 `apply`
         *   函数来简化访问者的调用。
         *   ```
         *   FE::AST::ASTPrinter printer;
         *   apply(printer, *ast, outStream);
         *   ```
         *
         * 期望输出示例:
         * 在 `testcase/parser/` 目录下提供了一些测试用例以及它们的预期输出，可以自行查看。
         */
        ast = parser.parseAST();
        if (!ast)
        {
            cerr << "Parsing failed." << endl;
            ret = 1;
            goto cleanup_files;
        }

        if (step == "-parser")
        {
            FE::AST::ASTPrinter printer;
            std::ostream*       osPtr = outStream;
            apply(printer, *ast, osPtr);

            ret = 0;
            goto cleanup_ast;
        }

        /*
         * Lab 3-1: 语义分析
         *
         * 本阶段需要实现语义分析器，遍历 AST 并检查源程序的静态语义正确性。
         * 你的编译器需要能识别并报告以下错误：
         * - 变量/函数重定义、变量未定义
         * - 全局变量的运行期初始化
         * - 非法的数组维度/下标/初始化
         * - 函数调用参数不匹配
         * - 非法赋值、非法返回值
         * - `main` 函数未定义
         * - 循环外的 `break`/`continue`
         * - 非法的操作数类型 (如对浮点数取模)
         *
         * 主要任务:
         * 1. 实现符号表:
         *    - 符号表用于在作用域中管理符号定义。接口位于 `interfaces/frontend/symbol/isymbol_table.h`。
         *    - 你需要实现 `frontend/symbol/symbol_table.h`中的 `enterScope`, `exitScope`, `addSymbol`, `getSymbol`
         * 等方法。
         *
         * 2. 实现语义检查器:
         *    - 实现符号表后，你需要通过访问 AST 来执行检查。
         *    - 框架提供了 `frontend/ast/visitor/sementic_check/ast_checker.h` 中的 `ASTChecker` 访问者。
         *    - 你需要填充各个 `visit` 方法的逻辑，利用符号表来发现并记录语义错误。
         *
         * 提示:
         * - 你可能最开始并不理解这一部分的 "实现语义检查器"
         * 具体要做哪些工作。简单来说就是，检查是否存在上述的语义错误，以及
         * 维护符号属性，如变量的类型、函数的参数等以供后续的 IR 生成使用。
         * 因此框架中保留了较为简单的几个 `visit` 方法的实现作为示例，你可以参考它们来实现其他节点的检查逻辑。
         */
        FE::AST::ASTChecker checker;
        bool                accept = apply(checker, *ast);
        if (!accept)
        {
            cerr << "Semantic check failed with " << checker.errors.size() << " errors." << endl;
            for (const auto& err : checker.errors) cerr << "Error: " << err << endl;
            ret = 1;
            goto cleanup_ast;
        }

        /*
         * Lab 3-2: 中间代码生成 (IR Generation)
         *
         * 目标:
         * - 将 AST 翻译为中间表示 (IR)。实现一个 AST 访问者, 将声明/表达式/语句映射为 IR
         * Module/Function/Block/Instruction。
         *
         * 主要任务:
         * - 在 middleend/visitor/codegen/ 中补全 ASTCodeGen 的各个 visit 接口
         * - 函数: 生成函数定义与基本块; 处理参数映射与返回; 维护循环起止标签
         * - 变量: 局部变量分配 (alloca/store); 全局变量定义与初值; 数组寻址 (GEP)
         * - 表达式: 字面量/一元/二元运算; 短路逻辑; 必要的类型转换
         * - 控制流: if/while/for/break/continue 的基本块拼接与条件/无条件分支
         * - 调用: 库函数声明与函数调用
         *
         * 相关文件:
         * - middleend/visitor/codegen/ast_codegen.{h,cpp}
         * - middleend/visitor/codegen/expr_codegen.cpp
         * - middleend/visitor/codegen/stmt_codegen.cpp
         * - middleend/visitor/codegen/decl_codegen.cpp
         * - middleend/visitor/codegen/type_convert.cpp (类型转换与算符实现)
         * - middleend/module/ir_module.{h,cpp} (IR 数据结构)
         * - middleend/visitor/printer/module_printer.{h,cpp} (IR 打印)
         *
         * 提示:
         * - 可先实现字面量、简单算术与顺序语句, 再逐步支持数组与控制流
         * - 通过 -llvm 输出验证 IR 是否符合预期
         */
        ME::ASTCodeGen codegen(checker.getGlbSymbols(), checker.getFuncDecls());
        ME::Module     m;

        apply(codegen, *ast, &m);

        if (optimizeLevel > 0)
        {
            /*
             * Lab 4: 中间代码优化
             *
             * 选做此部分的同学需至少完成完整形式的 mem2reg 以及可消除不可达块/无用语句的死代码消除
             * 完成必要优化后，可继续实现下面的优化：
             * - 稀疏条件常量传播（需实现跨访存的传播）
             * - 标量运算的循环不变量外提
             * - 标量运算的公共子表达式消除
             * - 函数内联
             * - 激进死代码消除（基于控制依赖图，需删除死循环）
             * - 难度不低于上述 pass 的其它优化
             */
            // 下面这个 pass 可以作为参考，主要是示范如何通过cache获取分析pass的结果
            ME::UnifyReturnPass unifyReturnPass;
            unifyReturnPass.runOnModule(m);
        }

        if (step == "-llvm")
        {
            // 这一部分的打印有完整实现提供，如果你未对 IR 结构有改动，可以直接使用
            ME::IRPrinter printer;
            printer.visit(m, *outStream);
            ret = 0;
            goto cleanup_ast;
        }

        if (step != "-S")
        {
            cerr << "Unknown step: " << step << endl;
            ret = 1;
            goto cleanup_ast;
        }

        /*
         * Lab 5: 后端代码生成
         *
         * 从这里开始进入后端。请先阅读 `backend/README.md` 了解后端目录结构、流水线与各阶段职责
         * （指令选择 → 帧降低 → 寄存器分配 → 栈降低）。
         *
         * 后端的“主流程入口”不在此文件，而在各目标架构的 Target 类：
         * - AArch64: `backend/targets/aarch64/aarch64_target.cpp` 的 `AArch64Target::runPipeline`
         * - RISC-V:  `backend/targets/riscv64/rv64_target.cpp` 的 `RV64::Target::runPipeline`
         * main 中通过 `BE::Targeting::TargetRegistry::getTarget(march)` 获取具体后端实例，
         * 然后调用 `target->runPipeline(&m, &backendModule, outStream)` 串起后续步骤。
         *
         * 关于 `BE::Targeting::TargetRegistry`：
         * - 作用：维护“目标字符串 → Target 工厂/实例”的全局注册表，作为后端选择与复用的入口。
         * - 注册：各架构在其 target 源文件内通过静态构造注册工厂函数，例如：
         *   AArch64 在 `aarch64_target.cpp` 中注册 "aarch64"/"armv8"；
         *   RISC‑V 在 `rv64_target.cpp` 中注册 "riscv64"/"riscv"/"rv64"。
         * - 获取：使用命令行 `-march` 的字符串（默认 "riscv64"）调用 `getTarget(march)`：
         *     `auto* tgt = BE::Targeting::TargetRegistry::getTarget(march);`
         *   若已创建过则返回缓存实例，否则通过对应工厂创建并缓存；找不到则返回 `nullptr`。
         *
         * 主要任务：
         * - 选择并完成你的指令选择（DAG ISel 或直接 IR→MIR），产出包含 FrameIndex 等抽象的 MIR；
         * - 在 AArch64/RV64 的 `runPipeline` 内按 README.md 给出的顺序串起各 Pass；
         * 
         * 补充说明：
         * - 项目根目录下提供了 arm2bin.sh 和 rv2bin.sh 脚本，可以用于将 AArch64 和 RISC-V 的汇编代码转换为二进制文件。
         *     默认输入为 `test.s`，输出为 `test.bin`，可通过命令行参数修改。
         * - 关于后端调试，请同学们善用 gdb 工具。arm2bin.sh 和 rv2bin.sh 脚本中已经添加了 -g 选项，可以生成调试信息。
         *     随后，你可以使用 gdb-multiarch 工具来调试 AArch64 和 RISC-V 的程序。具体启动示例如下：
         *     ```bash
         *     qemu-riscv64 -g {port} {exec} &      // {port} 为调试端口，{exec} 为可执行文件，& 表示后台运行
         *                                          // 如 `qemu-riscv64 -g 1234 test.bin &`
         *                                          // 当然不加 & 也可以，新开一个终端运行 gdb 就行
         *     gdb-multiarch {exec}
         *     (gdb) target remote:{port}
         *     ```
         *     gdb 的使用相信大家在 OS 课上已经有所了解，这里就略过。
         */
        BE::Module backendModule;
        auto*      tgt = BE::Targeting::TargetRegistry::getTarget(march);
        if (!tgt)
        {
            cerr << "Unknown target: " << march << endl;
            ret = 1;
            goto cleanup_ast;
        }

        tgt->runPipeline(&m, &backendModule, outStream);

        ret = 0;
    }

cleanup_ast:
    delete ast;
    ast = nullptr;

cleanup_files:
    if (in.is_open()) in.close();

cleanup_outfile:
    if (outFile.is_open()) outFile.close();

    return ret;
}
