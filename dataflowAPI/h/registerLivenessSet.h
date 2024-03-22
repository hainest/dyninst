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

#ifndef DYNINST_DATAFLOWAPI_REGISTERLIVENESSSET_H
#define DYNINST_DATAFLOWAPI_REGISTERLIVENESSSET_H

#include "registers/MachRegister.h"
#include "registers/registerSet.h"

#include <unordered_map>

#include <boost/dynamic_bitset.hpp>


namespace Dyninst { namespace DataflowAPI { namespace abi {

  namespace detail {
    struct reg_hasher final {
      std::size_t operator()(Dyninst::MachRegister m) const {
        return m.val();
      }
    };
  }

  class registerLivenessSet final {
    std::unordered_map<Dyninst::MachRegister, size_t, detail::reg_hasher> lookup_table;
    boost::dynamic_bitset<size_t> state;
  public:
    registerLivenessSet(Dyninst::registerSet const& regs) {

      /* Try to use perfect hashing
       *
       *  1) Each MachRester has a unique ID/value
       *  2) The hash table has a slot for each MachRegister
       *  3) The maximum number of elements per bucket (max_load_factor) is 1
       *
       *  Combined, these _should_ allow for open addressing.
       */
      lookup_table.reserve(regs.cardinality());
      lookup_table.max_load_factor(1.0f);

      // Map a register to a linear index that is used to identify the state in `state`.
      size_t index=0;
      for(auto const& r : regs) {
        lookup_table.insert({r, index});
        index++;
      }

      // Make enough room for all of the registers. Their state is unspecified.
      // A value of ``true`` indicates the register is alive. Otherwise, dead.
      state.resize(regs.cardinality());
    }
    registerLivenessSet() = default;

    void killAll() {
      // Set all to ``false`` (dead)
      state.reset();
    }
    void reviveAll() {
      // Set all to ``true`` (alive)
      state.set();
    }
    void kill(Dyninst::MachRegister const& r) {
      auto itr = lookup_table.find(r);
      if(itr == lookup_table.end()) {
        return;
      }
      auto index = itr->second;
      state.set(index, false);
    }
    void revive(Dyninst::MachRegister const& r) {
      auto itr = lookup_table.find(r);
      if(itr == lookup_table.end()) {
        return;
      }
      auto index = itr->second;
      state.set(index, true);
    }
    bool isDead(Dyninst::MachRegister const& r) const {
      auto itr = lookup_table.find(r);
      if(itr == lookup_table.end()) {
        return false;
      }
      auto index = itr->second;
      return state[index] == false;
    }
    bool isAlive(Dyninst::MachRegister const& r) const {
      auto itr = lookup_table.find(r);
      if(itr == lookup_table.end()) {
        return false;
      }
      auto index = itr->second;
      return state[index] == true;
    }

    // Union (in-place)
    void operator|=(registerLivenessSet const& rhs) {
      state |= rhs;
    }

    // Union
    registerLivenessSet operator|(registerLivenessSet const& rhs) const {
      auto tmp = *this;
      tmp |= rhs;
      return tmp;
    }

    // Intersection (in-place)
    void operator&=(registerLivenessSet const& rhs) {
      state &= rhs;
    }

    // Intersection
    registerLivenessSet operator&(registerLivenessSet const& rhs) const {
      auto tmp = *this;
      tmp &= rhs;
      return tmp;
    }

    // Complement
    registerLivenessSet operator~() const {
      auto tmp = *this;
      tmp.state = ~tmp.state;
      return tmp;
    }
  };

}}}

#endif
