#include <string>
#include <fstream>
#include <iostream>

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace clang::tooling;

using namespace llvm;
using namespace llvm::cl;

class CheckerASTVisitor : public RecursiveASTVisitor<CheckerASTVisitor>
{
public:
    bool VisitParmVarDecl(VarDecl *vardecl) {
        //std::cout << "visit var decl" << std::endl;
        const auto name = vardecl->getNameAsString();
        if (name.length() >= 8) {
            // Get the diagnostic engine
            auto &DE = vardecl->getASTContext().getDiagnostics();
            // Create custom error message
            auto diagID = DE.getCustomDiagID(
                DiagnosticsEngine::Error,
                "var name too long!");
            // Report our custom error
            DE.Report(vardecl->getLocation(), diagID);
        }
        return true;
    };

    auto VisitFunctionDecl(FunctionDecl *f) -> bool
    {
        //std::cout << "Check function name" << std::endl;
        const auto name = f->getNameAsString();
        // Check if name contains _ at any position
        if (name.find("_") != std::string::npos)
        {
            // Get the diagnostic engine
            auto &DE = f->getASTContext().getDiagnostics();
            // Create custom error message
            auto diagID = DE.getCustomDiagID(
                DiagnosticsEngine::Error,
                "Function name contains `_`.");
            // Report our custom error
            DE.Report(f->getLocation(), diagID);
        }
        return true;
    }

    bool VisitCXXRecordDecl(CXXRecordDecl *D)
    {
        // Get number of bases classes
        auto number_of_bases = D->getNumBases();
        // Check if number of bases is bigger than one
        if (number_of_bases > 1)
        {
            // Report custom error message like the last check
            auto &DE = D->getASTContext().getDiagnostics();
            auto diagID = DE.getCustomDiagID(
                DiagnosticsEngine::Error, "Class inhiert from more than one class.");
            auto DB = DE.Report(D->getLocation(), diagID);
        }
        return true;
    }
};

class CheckerASTConsumer : public ASTConsumer
{
public:
    auto HandleTranslationUnit(ASTContext &context) -> void override
    {
        CheckerASTVisitor visitor;
        visitor.TraverseDecl(context.getTranslationUnitDecl());
    }
};

class CheckerFrontendAction : public ASTFrontendAction {
public:
  auto CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InFile)
      -> std::unique_ptr<clang::ASTConsumer> override {
    return std::make_unique<CheckerASTConsumer>();
  }
};

auto main(int argc, const char **argv) -> int
{
    if (argc < 2)
    {
        errs() << "Usage: " << argv[0] << " <filename>.cpp\n";
        return 1;
    }

    auto file_name = argv[1];

    // Read source file content to pass to clang
    std::ifstream if_stream(file_name);
    std::string content((std::istreambuf_iterator<char>(if_stream)),
                        (std::istreambuf_iterator<char>()));

    auto action = std::make_unique<CheckerFrontendAction>();

    // Pass action, file content and file name that used in error message
    clang::tooling::runToolOnCode(std::move(action), content, file_name);
    return 0;
}
