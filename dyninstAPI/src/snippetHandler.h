#ifndef DYNINSTAPI_SNIPPETHANDLER
#define DYNINSTAPI_SNIPPETHANDLER

#include <vector>

class BPatch_point;

namespace Dyninst {

  struct snippetHandler {
    virtual void record_points(std::vector<BPatch_point *> const&) {}
    virtual ~snippetHandler() = default;
  };

}

#endif
