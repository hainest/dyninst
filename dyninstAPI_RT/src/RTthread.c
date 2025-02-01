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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <threads.h>

#include "RTthread.h"
#include "RTcommon.h"
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "dyninstAPI_RT/h/dyninstRTExport.h"

int DYNINST_multithread_capable;
extern unsigned int DYNINSThasInitialized;

static void (*rt_newthr_cb)(int) = NULL;
void setNewthrCB(void (*cb)(int)) {
    rt_newthr_cb = cb;
}

#define IDX_NONE -1

int tc_lock_init(tc_lock_t *t) {
  mtx_t *mutex = malloc(sizeof(mtx_t));
  int status = mtx_init(mutex, mtx_plain);
  if(status != thrd_success) {
    return status;
  }
  t->internal_lock = mutex;
  t->tid = DYNINST_INITIAL_LOCK_PID;
  return 0;
}

int tc_lock_destroy(tc_lock_t *t) {
  mtx_t *mutex = (mtx_t*)(t->internal_lock);
  mtx_destroy(mutex);
  free(mutex);
  t->internal_lock = NULL;
  t->tid = DYNINST_INITIAL_LOCK_PID;
  return 0;
}

int tc_lock_lock(tc_lock_t *t) {
  return mtx_lock((mtx_t*)(t->internal_lock));
}

int tc_lock_unlock(tc_lock_t *t) {
  return mtx_unlock((mtx_t*)(t->internal_lock));
}
