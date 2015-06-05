/* Preamble and helpers for the autogenerated generic-match.c file.
   Copyright (C) 2014-2015 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "hash-set.h"
#include "vec.h"
#include "input.h"
#include "alias.h"
#include "symtab.h"
#include "options.h"
#include "inchash.h"
#include "tree.h"
#include "fold-const.h"
#include "stringpool.h"
#include "stor-layout.h"
#include "flags.h"
#include "tm.h"
#include "hard-reg-set.h"
#include "function.h"
#include "predict.h"
#include "basic-block.h"
#include "tree-ssa-alias.h"
#include "internal-fn.h"
#include "gimple-expr.h"
#include "is-a.h"
#include "gimple.h"
#include "gimple-ssa.h"
#include "tree-ssanames.h"
#include "gimple-fold.h"
#include "gimple-iterator.h"
#include "hashtab.h"
#include "rtl.h"
#include "statistics.h"
#include "insn-config.h"
#include "expmed.h"
#include "dojump.h"
#include "explow.h"
#include "calls.h"
#include "emit-rtl.h"
#include "varasm.h"
#include "stmt.h"
#include "expr.h"
#include "tree-dfa.h"
#include "builtins.h"
#include "tree-phinodes.h"
#include "ssa-iterators.h"
#include "dumpfile.h"
#include "generic-match.h"

/* Routine to determine if the types T1 and T2 are effectively
   the same for GENERIC.  If T1 or T2 is not a type, the test
   applies to their TREE_TYPE.  */

static inline bool
types_match (tree t1, tree t2)
{
  if (!TYPE_P (t1))
    t1 = TREE_TYPE (t1);
  if (!TYPE_P (t2))
    t2 = TREE_TYPE (t2);

  return TYPE_MAIN_VARIANT (t1) == TYPE_MAIN_VARIANT (t2);
}

/* Return if T has a single use.  For GENERIC, we assume this is
   always true.  */

static inline bool
single_use (tree t ATTRIBUTE_UNUSED)
{
  return true;
}
