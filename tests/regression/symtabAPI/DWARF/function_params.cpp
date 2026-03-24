#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include <Function.h>
#include <Symtab.h>
#include <Type.h>
#include <Variable.h>

#include <boost/icl/interval_map.hpp>

#include <unistd.h>

namespace ds = Dyninst::SymtabAPI;

struct local_var {
  ds::localVar *var;
  ds::Function *func;
};

using var_location =
    boost::interval_map<Dyninst::Address, std::vector<local_var>>;

var_location get_variable_locations(ds::Function *f);

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " file\n";
    return EXIT_FAILURE;
  }

  auto *filename = argv[1];
  ds::Symtab *symtab{};

  if (!ds::Symtab::openFile(symtab, filename)) {
    std::cerr << "Unable to open file '" << filename << "'\n";
    return EXIT_FAILURE;
  }

  var_location locations;

  for (auto *f : sym->getAllFunction()) {
    std::clog << "Checking function '" << f->getName() "'\n";

    std::vector<ds::localVar *> lvars;
    f->getParams(lvars);
    f->getLocalVariables(lvars);

    for (auto *var : lvars) {
      if (var->getName() == "this") {
        std::clog << "this <" << var->getType()->getName() << ">\n";
      } else {
        std::clog << var->getName() << " Defined: " << var->getFileName() << ':'
                  << var->getLineNum() << endl;
      }

      for (auto loc : var->getLocationLists()) {
        // this unfortunately seems to be happening. It may either be a
        // problem with the DWARF or a problem with dyninst.
        if (loc.lowPC == 0 || loc.hiPC == 0xFFFFFFFFFFFFFFFF) {
          cerr << "Location List for " << var->getName() << " from "
               << i->getName() << " seems insane [" << hex << loc.lowPC << ','
               << loc.hiPC << "]: skipping\n";
          continue;
        }

        discrete_interval<Address> addr_inter =
            construct<discrete_interval<Address>>(loc.lowPC, loc.hiPC,
                                                  interval_bounds::closed());
        if (verbose) {
          std::clog << "\t\t[" << hex << loc.lowPC << dec;
          vector<Statement *> lines;
          if (i->getModule()->getSourceLines(lines, loc.lowPC))
            for (auto l : lines)
              std::clog << ' ' << l->getFile() << ':' << l->getLine() << 'c'
                        << l->getColumn();
          lines.clear();
          std::clog << ',' << hex << loc.hiPC << dec;
          if (i->getModule()->getSourceLines(lines, loc.hiPC))
            for (auto l : lines)
              std::clog << ' ' << l->getFile() << ':' << l->getLine() << 'c'
                        << l->getColumn();
          std::clog << ']' << endl;
        }
        LVarSet newone;
        pair<localVar *, Function *> newpair;
        newpair.first = var;
        newpair.second = i;
        newone.insert(newpair);
        locations.add(make_pair(addr_inter, newone));
      }
    }
  }

  for (auto i : locations) {
    std::clog << '[' << hex << i.first.lower() << dec << ' ';
    vector<Statement *> lines;
    auto funcp = i.second.begin()->second;
    if (funcp->getModule()->getSourceLines(lines, i.first.lower()))
      for (auto l : lines)
        std::clog << ' ' << l->getFile() << ':' << l->getLine() << 'c'
                  << l->getColumn();
    if (i.first.lower() != i.first.upper()) {
      lines.clear();
      std::clog << ',' << hex << i.first.upper() << dec << ' ';
      if (funcp->getModule()->getSourceLines(lines, i.first.upper()))
        for (auto l : lines)
          std::clog << ' ' << l->getFile() << ':' << l->getLine() << 'c'
                    << l->getColumn();
    }
    std::clog << ']' << ": " << endl;
    for (auto j : i.second) {
      if (j.first->getName() == "this")
        std::clog << "\tthis <"
                  << "Func:" << funcp->getName() << ' '
                  << j.first->getType()->getName() << ">\n";
      else {
        std::clog << '\t' << j.first->getName();
        //	if( !j.first->getFileName().empty())
        std::clog << " [" << j.first->getFileName() << ':'
                  << j.first->getLineNum() << ']';
        std::clog << endl;
      }
    }
    std::clog << endl;
  }
}
