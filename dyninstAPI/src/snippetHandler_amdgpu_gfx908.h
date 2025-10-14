#ifndef DYNINSTAPI_SNIPPETHANDLER_AMDGPU_GFX908
#define DYNINSTAPI_SNIPPETHANDLER_AMDGPU_GFX908

#include "snippetHandler.h"
#include "BPatch_function.h"
#include "BPatch_point.h"
#include "amdgpu-epilogue.h"
#include "amdgpu-prologue.h"

#include <string>
#include <unordered_set>
#include <vector>

using namespace Dyninst::PatchAPI;

// KERNEL-INFO-START
constexpr int NumAmdgpuKernelElems = 3;

namespace Dyninst {

  struct AmdgpuKernelInfo {
    AmdgpuKernelInfo(std::vector<std::string> &words) {
      assert(words.size() == NumAmdgpuKernelElems);
      kdName = words[0];
      kernargBufferSize = std::stoul(words[1]);
      kernargPtrRegister = std::stoul(words[2]);
    }
    std::string getKernelName() const {
      assert(kdName.length() > 3);
      return kdName.substr(0, kdName.length() - 3);
    }

    std::string kdName;
    unsigned kernargBufferSize;
    unsigned kernargPtrRegister;
  };

  struct snippetHandler_amdgpu_gfx908 : Dyninst::snippetHandler {

    void record_points(std::vector<BPatch_point *> const& points) override {
      for (BPatch_function *f : points) {
        auto result = instrumentedFunctions.insert(f);
        if (result.second) {
          insertPrologueIfKernel(f);
          insertEpilogueIfKernel(f);
        }
      }
    }

    void insertPrologueAtPoints(AmdgpuPrologueSnippet &snippet,
                                                    std::vector<BPatch_point *> &points) {
      for (size_t i = 0; i < points.size(); ++i) {
        instPoint *iPoint = static_cast<instPoint *>(points[i]->getPoint(BPatch_callBefore));
        iPoint->pushBack(snippet.ast_wrapper, SnippetType::PROLOGUE);
      }
    }

    void insertPrologueIfKernel(BPatch_function *function) {
      // Go through information for instrumented kernels (functions) and insert a prologue
      // that loads s[94:95] with address of memory for instrumentation variables. This address
      // is at address [kernargPtrRegister] + kernargBufferSize.
      std::vector<BPatch_point *> entryPoints;
      function->getEntryPoints(entryPoints);

      for (auto &kernelInfo : kernelInfos) {
        if (kernelInfo.getKernelName() == function->getMangledName()) {

          auto prologuePtr = boost::make_shared<AmdgpuPrologue>(94, kernelInfo.kernargPtrRegister,
                                                                kernelInfo.kernargBufferSize);

          AstNodePtr prologueNodePtr = boost::make_shared<AmdgpuPrologueNode>(prologuePtr);

          AmdgpuPrologueSnippet prologueSnippet(prologueNodePtr);
          insertPrologueAtPoints(prologueSnippet, entryPoints);
          break;
        }
      }
    }

    void insertEpilogueAtPoints(AmdgpuEpilogueSnippet &snippet,
                                                    std::vector<BPatch_point *> &points) {
      for (size_t i = 0; i < points.size(); ++i) {
        instPoint *iPoint = static_cast<instPoint *>(points[i]->getPoint(BPatch_callAfter));
        iPoint->pushBack(snippet.ast_wrapper, SnippetType::EPILOGUE);
      }
    }

    void insertEpilogueIfKernel(BPatch_function *function) {
      // Go through information for instrumented kernels (functions) and insert a s_dcache_wb
      // instruction at exit points.
      std::vector<BPatch_point *> exitPoints;
      function->getExitPoints(exitPoints);

      for (auto &kernelInfo : kernelInfos) {
        if (kernelInfo.getKernelName() == function->getMangledName()) {

          auto epiloguePtr = boost::make_shared<AmdgpuEpilogue>();

          AstNodePtr epilogueNodePtr = boost::make_shared<AmdgpuEpilogueNode>(epiloguePtr);

          AmdgpuEpilogueSnippet epilogueSnippet(epilogueNodePtr);
          insertEpilogueAtPoints(epilogueSnippet, exitPoints);
          break;
        }
      }
    }

    std::unordered_set<BPatch_function *> instrumentedFunctions;
    std::vector<AmdgpuKernelInfo> kernelInfos;
  };

}

#endif
