/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef CODE_OBJECT_H
#define CODE_OBJECT_H

#include <set>
#include <string>
#include <vector>
#include <map>

#include "Symtab.h"
#include "IBSTree.h"

#include "CodeSource.h"
#include "CFGFactory.h"
#include "CFG.h"
#include "ParseContainers.h"

namespace Dyninst {
namespace ParseAPI {

class Parser;   // internals
class ParseCallback;
class ParseCallbackManager;
class CFGModifier;
class CodeSource;

typedef enum {
    PreambleMatching, IdiomMatching
} GapParsingType;

class CodeObject {
   friend class CFGModifier;
 public:
    DYNINST_EXPORT static void version(int& major, int& minor, int& maintenance);
    typedef std::set<Function*,Function::less> funclist;

    DYNINST_EXPORT CodeObject(CodeSource * cs, 
                             CFGFactory * fact = NULL, 
                             ParseCallback * cb = NULL,
                             bool defensiveMode = false,
                             bool ignoreParse = false);
    DYNINST_EXPORT ~CodeObject();

    /** Parsing interface **/
    DYNINST_EXPORT void parse();
    
    DYNINST_EXPORT void parse(Address target, bool recursive);
    DYNINST_EXPORT void parse(const std::vector<Address> &targets, bool recursive);

    DYNINST_EXPORT void parse(CodeRegion *cr, Address target, bool recursive);
    DYNINST_EXPORT void parse(const std::vector<std::pair<Address, CodeRegion *>> &targets, bool recursive);

	struct NewEdgeToParse {
		Block *source;
		Address target;
		EdgeTypeEnum edge_type;
        bool checked; // true if call_ft edges have already had their callees checked
		NewEdgeToParse(Block *a, Address b, EdgeTypeEnum c) : source(a), target(b), edge_type(c), checked(false) {}
        NewEdgeToParse(Block* a, Address b, bool c, EdgeTypeEnum d) : source(a), target(b), edge_type(d), checked(c) { }
	};

    DYNINST_EXPORT bool parseNewEdges( std::vector<NewEdgeToParse> & worklist ); 

    DYNINST_EXPORT void parseGaps(CodeRegion *cr, GapParsingType type=IdiomMatching);

    /** Lookup routines **/

    DYNINST_EXPORT Function * findFuncByEntry(CodeRegion * cr, Address entry);
    DYNINST_EXPORT int findFuncsByBlock(CodeRegion *cr, Block* b, std::set<Function*> &funcs);
    DYNINST_EXPORT int findFuncs(CodeRegion * cr, 
            Address addr, 
            std::set<Function*> & funcs);
    DYNINST_EXPORT int findFuncs(CodeRegion * cr,
            Address start, Address end,
            std::set<Function*> & funcs);
    DYNINST_EXPORT int findCurrentFuncs(CodeRegion * cr,
            Address addr,
            std::set<Function*> & funcs);


    DYNINST_EXPORT const funclist & funcs() { return flist; }

    // blocks
    DYNINST_EXPORT Block * findBlockByEntry(CodeRegion * cr, Address entry);
    DYNINST_EXPORT int findBlocks(CodeRegion * cr, 
        Address addr, std::set<Block*> & blocks);

    DYNINST_EXPORT int findCurrentBlocks(CodeRegion * cr, 
        Address addr, std::set<Block*> & blocks);
    DYNINST_EXPORT Block * findNextBlock(CodeRegion * cr, Address addr);

    DYNINST_EXPORT CodeSource * cs() const { return _cs; }
    DYNINST_EXPORT CFGFactory * fact() const { return _fact; }
    DYNINST_EXPORT bool defensiveMode() { return defensive; }

    DYNINST_EXPORT bool isIATcall(Address insn, std::string &calleeName);

    DYNINST_EXPORT void startCallbackBatch();
    DYNINST_EXPORT void finishCallbackBatch();
    DYNINST_EXPORT void registerCallback(ParseCallback *cb);
    DYNINST_EXPORT void unregisterCallback(ParseCallback *cb);

    DYNINST_EXPORT void finalize();

    DYNINST_EXPORT void destroy(Edge *);
    DYNINST_EXPORT void destroy(Block *);
    DYNINST_EXPORT void destroy(Function *);

    DYNINST_EXPORT Address getFreeAddr() const;
    ParseData* parse_data();

 private:
    void process_hints();
    void add_edge(Block *src, Block *trg, EdgeTypeEnum et);
    // allows Functions to link up return edges after-the-fact
    friend void Function::delayed_link_return(CodeObject *,Block*);
    // allows Functions to finalize (need Parser access)
    friend void Function::finalize();
    // allows Function entry blocks to be moved to new regions
    friend void Function::setEntryBlock(Block *);

  private:
    CodeSource * _cs;
    CFGFactory * _fact;
    ParseCallbackManager * _pcb;

    Parser * parser; // parser implementation

    bool owns_factory;
    bool defensive;
    funclist& flist;
};

// We need CFG.h, which is included by this
template <class OutputIterator>
void Block::getFuncs(OutputIterator result) {
  std::set<Function *> stab;
  _obj->findFuncsByBlock(region(), this, stab);
  std::copy(stab.begin(), stab.end(), result);
}


}//namespace ParseAPI
}//namespace Dyninst

#endif
