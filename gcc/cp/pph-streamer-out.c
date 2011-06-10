/* Routines for writing PPH data.
   Copyright (C) 2011 Free Software Foundation, Inc.
   Contributed by Diego Novillo <dnovillo@google.com>.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GCC is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "langhooks.h"
#include "tree-iterator.h"
#include "tree-pretty-print.h"
#include "lto-streamer.h"
#include "pph-streamer.h"
#include "pph.h"
#include "tree-pass.h"
#include "version.h"
#include "cppbuiltin.h"

/* FIXME pph.  This holds the FILE handle for the current PPH file
   that we are writing.  It is necessary because the LTO callbacks do
   not allow passing a FILE handle to them.  */
static FILE *current_pph_file = NULL;


/* Callback for packing value fields in ASTs.  BP is the bitpack 
   we are packing into.  EXPR is the tree to pack.  */

void
pph_pack_value_fields (struct bitpack_d *bp, tree expr)
{
  if (TYPE_P (expr))
    {
      bp_pack_value (bp, TYPE_LANG_FLAG_0 (expr), 1);
      bp_pack_value (bp, TYPE_LANG_FLAG_1 (expr), 1);
      bp_pack_value (bp, TYPE_LANG_FLAG_2 (expr), 1);
      bp_pack_value (bp, TYPE_LANG_FLAG_3 (expr), 1);
      bp_pack_value (bp, TYPE_LANG_FLAG_4 (expr), 1);
      bp_pack_value (bp, TYPE_LANG_FLAG_5 (expr), 1);
      bp_pack_value (bp, TYPE_LANG_FLAG_6 (expr), 1);
    }
  else if (DECL_P (expr))
    {
      bp_pack_value (bp, DECL_LANG_FLAG_0 (expr), 1);
      bp_pack_value (bp, DECL_LANG_FLAG_1 (expr), 1);
      bp_pack_value (bp, DECL_LANG_FLAG_2 (expr), 1);
      bp_pack_value (bp, DECL_LANG_FLAG_3 (expr), 1);
      bp_pack_value (bp, DECL_LANG_FLAG_4 (expr), 1);
      bp_pack_value (bp, DECL_LANG_FLAG_5 (expr), 1);
      bp_pack_value (bp, DECL_LANG_FLAG_6 (expr), 1);
      bp_pack_value (bp, DECL_LANG_FLAG_7 (expr), 1);
      bp_pack_value (bp, DECL_LANG_FLAG_8 (expr), 1);
    }

  bp_pack_value (bp, TREE_LANG_FLAG_0 (expr), 1);
  bp_pack_value (bp, TREE_LANG_FLAG_1 (expr), 1);
  bp_pack_value (bp, TREE_LANG_FLAG_2 (expr), 1);
  bp_pack_value (bp, TREE_LANG_FLAG_3 (expr), 1);
  bp_pack_value (bp, TREE_LANG_FLAG_4 (expr), 1);
  bp_pack_value (bp, TREE_LANG_FLAG_5 (expr), 1);
  bp_pack_value (bp, TREE_LANG_FLAG_6 (expr), 1);
}


/* Initialize buffers and tables in STREAM for writing.  */

void
pph_init_write (pph_stream *stream)
{
  lto_streamer_init ();
  stream->out_state = lto_new_out_decl_state ();
  lto_push_out_decl_state (stream->out_state);
  stream->decl_state_stream = XCNEW (struct lto_output_stream);
  stream->ob = create_output_block (LTO_section_decls);

  /* Associate STREAM with STREAM->OB so we can recover it from the
     streamer hooks.  */
  stream->ob->sdata = (void *) stream;
}


/* Callback for lang_hooks.lto.begin_section.  Open file NAME.  */

static void
pph_begin_section (const char *name ATTRIBUTE_UNUSED)
{
}


/* Callback for lang_hooks.lto.append_data.  Write LEN bytes from DATA
   into current_pph_file.  BLOCK is currently unused.  */

static void
pph_out (const void *data, size_t len, void *block ATTRIBUTE_UNUSED)
{
  if (data)
    fwrite (data, len, 1, current_pph_file);
}


/* Callback for lang_hooks.lto.end_section.  */

static void
pph_end_section (void)
{
}


/* Write the header for the PPH file represented by STREAM.  */

static void
pph_out_header (pph_stream *stream)
{
  pph_file_header header;
  struct lto_output_stream header_stream;
  int major, minor, patchlevel;

  /* Collect version information.  */
  parse_basever (&major, &minor, &patchlevel);
  gcc_assert (major == (char) major);
  gcc_assert (minor == (char) minor);
  gcc_assert (patchlevel == (char) patchlevel);

  /* Write the header for the PPH file.  */
  memset (&header, 0, sizeof (header));
  strcpy (header.id_str, pph_id_str);
  header.major_version = (char) major;
  header.minor_version = (char) minor;
  header.patchlevel = (char) patchlevel;
  header.strtab_size = stream->ob->string_stream->total_size;

  memset (&header_stream, 0, sizeof (header_stream));
  lto_output_data_stream (&header_stream, &header, sizeof (header));
  lto_write_stream (&header_stream);
}


/* Write the body of the PPH file represented by STREAM.  */

static void
pph_out_body (pph_stream *stream)
{
  /* Write the string table.  */
  lto_write_stream (stream->ob->string_stream);

  /* Write out the physical representation for every AST in all the
     streams in STREAM->OUT_STATE.  */
  lto_output_decl_state_streams (stream->ob, stream->out_state);

  /* Now write the vector of all AST references.  */
  lto_output_decl_state_refs (stream->ob, stream->decl_state_stream,
			      stream->out_state);

  /* Finally, physically write all the streams.  */
  lto_write_stream (stream->ob->main_stream);
}


/* Flush all the in-memory buffers for STREAM to disk.  */

void
pph_flush_buffers (pph_stream *stream)
{
  gcc_assert (current_pph_file == NULL);
  current_pph_file = stream->file;

  /* Redirect the LTO basic I/O langhooks.  */
  lang_hooks.lto.begin_section = pph_begin_section;
  lang_hooks.lto.append_data = pph_out;
  lang_hooks.lto.end_section = pph_end_section;

  /* Write the state buffers built by pph_out_*() calls.  */
  lto_begin_section (stream->name, false);
  pph_out_header (stream);
  pph_out_body (stream);
  lto_end_section ();
  current_pph_file = NULL;
}


/* Start a new record in STREAM for data in DATA.  If DATA is NULL
   write an end-of-record marker and return false.  If DATA is not NULL
   and did not exist in the pickle cache, add it, write a
   start-of-record marker and return true.  If DATA existed in the
   cache, write a shared-record marker and return false.  */

static inline bool
pph_start_record (pph_stream *stream, void *data)
{
  if (data)
    {
      bool existed_p;
      unsigned ix;
      enum pph_record_marker marker;

      /* If the memory at DATA has already been streamed out, make
	 sure that we don't write it more than once.  Otherwise,
	 the reader will instantiate two different pointers for
	 the same object.

	 Write the index into the cache where DATA has been stored.
	 This way, the reader will know at which slot to
	 re-materialize DATA the first time and where to access it on
	 subsequent reads.  */
      existed_p = pph_cache_add (stream, data, &ix);
      marker = (existed_p) ? PPH_RECORD_SHARED : PPH_RECORD_START;
      pph_out_uchar (stream, marker);
      pph_out_uint (stream, ix);
      return marker == PPH_RECORD_START;
    }
  else
    {
      pph_out_uchar (stream, PPH_RECORD_END);
      return false;
    }
}


/* Write all the fields in lang_decl_base instance LDB to OB.  */

static void
pph_out_ld_base (pph_stream *stream, struct lang_decl_base *ldb)
{
  struct bitpack_d bp;

  bp = bitpack_create (stream->ob->main_stream);
  bp_pack_value (&bp, ldb->selector, 16);
  bp_pack_value (&bp, ldb->language, 4);
  bp_pack_value (&bp, ldb->use_template, 2);
  bp_pack_value (&bp, ldb->not_really_extern, 1);
  bp_pack_value (&bp, ldb->initialized_in_class, 1);
  bp_pack_value (&bp, ldb->repo_available_p, 1);
  bp_pack_value (&bp, ldb->threadprivate_or_deleted_p, 1);
  bp_pack_value (&bp, ldb->anticipated_p, 1);
  bp_pack_value (&bp, ldb->friend_attr, 1);
  bp_pack_value (&bp, ldb->template_conv_p, 1);
  bp_pack_value (&bp, ldb->odr_used, 1);
  bp_pack_value (&bp, ldb->u2sel, 1);
  pph_out_bitpack (stream, &bp);
}


/* Write all the fields in lang_decl_min instance LDM to STREAM.  If REF_P
   is true, all tree fields should be written as references.  */

static void
pph_out_ld_min (pph_stream *stream, struct lang_decl_min *ldm,
		         bool ref_p)
{
  pph_out_tree_or_ref_1 (stream, ldm->template_info, ref_p, 1);
  if (ldm->base.u2sel == 0)
    pph_out_tree_or_ref_1 (stream, ldm->u2.access, ref_p, 1);
  else if (ldm->base.u2sel == 1)
    pph_out_uint (stream, ldm->u2.discriminator);
  else
    gcc_unreachable ();
}


/* Write all the trees in gc VEC V to STREAM.  REF_P is true if the
   trees should be written as references. */

void
pph_out_tree_vec (pph_stream *stream, VEC(tree,gc) *v, bool ref_p)
{
  unsigned i;
  tree t;

  pph_out_uint (stream, VEC_length (tree, v));
  for (i = 0; VEC_iterate (tree, v, i, t); i++)
    pph_out_tree_or_ref (stream, t, ref_p);
}


/* Write all the qualified_typedef_usage_t in VEC V to STREAM.
   REF_P is true if the trees should be written as references. */

static void
pph_out_qual_use_vec (pph_stream *stream,
    VEC(qualified_typedef_usage_t,gc) *v, bool ref_p)
{
  unsigned i;
  qualified_typedef_usage_t *q;

  pph_out_uint (stream, VEC_length (qualified_typedef_usage_t, v));
  for (i = 0; VEC_iterate (qualified_typedef_usage_t, v, i, q); i++)
    {
      pph_out_tree_or_ref (stream, q->typedef_decl, ref_p);
      pph_out_tree_or_ref (stream, q->context, ref_p);
      /* FIXME pph: also write location.  */
    }
}


/* Forward declaration to break cyclic dependencies.  */
static void pph_out_binding_level (pph_stream *,
					    struct cp_binding_level *, bool);


/* Helper for pph_out_cxx_binding.  STREAM, CB and REF_P are as in
   pph_out_cxx_binding.  */

static void
pph_out_cxx_binding_1 (pph_stream *stream, cxx_binding *cb, bool ref_p)
{
  struct bitpack_d bp;

  if (!pph_start_record (stream, cb))
    return;

  pph_out_tree_or_ref (stream, cb->value, ref_p);
  pph_out_tree_or_ref (stream, cb->type, ref_p);
  pph_out_binding_level (stream, cb->scope, ref_p);
  bp = bitpack_create (stream->ob->main_stream);
  bp_pack_value (&bp, cb->value_is_inherited, 1);
  bp_pack_value (&bp, cb->is_local, 1);
  pph_out_bitpack (stream, &bp);
}


/* Write all the fields of cxx_binding instance CB to STREAM.  REF_P is
   true if the tree fields should be written as references.  */

static void
pph_out_cxx_binding (pph_stream *stream, cxx_binding *cb, bool ref_p)
{
  unsigned num_bindings;
  cxx_binding *prev;

  num_bindings = 0;
  for (prev = cb ? cb->previous : NULL; prev; prev = prev->previous)
    num_bindings++;

  /* Write the list of previous bindings.  */
  pph_out_uint (stream, num_bindings);
  if (num_bindings > 0)
    for (prev = cb->previous; prev; prev = prev->previous)
      pph_out_cxx_binding_1 (stream, prev, ref_p);

  /* Write the current binding at the end.  */
  pph_out_cxx_binding_1 (stream, cb, ref_p);
}


/* Write all the fields of cp_class_binding instance CB to STREAM.  REF_P
   is true if the tree fields should be written as references.  */

static void
pph_out_class_binding (pph_stream *stream, cp_class_binding *cb,
			        bool ref_p)
{
  if (!pph_start_record (stream, cb))
    return;

  pph_out_cxx_binding (stream, cb->base, ref_p);
  pph_out_tree_or_ref (stream, cb->identifier, ref_p);
}


/* Write all the fields of cp_label_binding instance LB to STREAM.  If
   REF_P is true, tree fields will be written as references.  */

static void
pph_out_label_binding (pph_stream *stream, cp_label_binding *lb,
				bool ref_p)
{
  if (!pph_start_record (stream, lb))
    return;

  pph_out_tree_or_ref (stream, lb->label, ref_p);
  pph_out_tree_or_ref (stream, lb->prev_value, ref_p);
}


/* Output a chain of nodes to STREAM starting with FIRST.  Skip any
   nodes that do not match FILTER.  REF_P is true if nodes in the chain
   should be emitted as references.  */

void
pph_out_chain_filtered (pph_stream *stream, tree first, bool ref_p,
			   enum chain_filter filter)
{
  unsigned count;
  tree t;

  /* Special case.  If the caller wants no filtering, it is much
     faster to just call pph_out_chain directly.  */
  if (filter == NONE)
    {
      pph_out_chain (stream, first, ref_p);
      return;
    }

  /* Count all the nodes that match the filter.  */
  for (t = first, count = 0; t; t = TREE_CHAIN (t))
    {
      if (filter == NO_BUILTINS && DECL_P (t) && DECL_IS_BUILTIN (t))
	continue;
      count++;
    }
  pph_out_uint (stream, count);

  /* Output all the nodes that match the filter.  */
  for (t = first; t; t = TREE_CHAIN (t))
    {
      tree saved_chain;

      /* Apply filters to T.  */
      if (filter == NO_BUILTINS && DECL_P (t) && DECL_IS_BUILTIN (t))
	continue;

      /* Clear TREE_CHAIN to avoid blindly recursing into the rest
	 of the list.  */
      saved_chain = TREE_CHAIN (t);
      TREE_CHAIN (t) = NULL_TREE;

      pph_out_tree_or_ref_1 (stream, t, ref_p, 2);

      TREE_CHAIN (t) = saved_chain;
    }
}


/* Write all the fields of cp_binding_level instance BL to STREAM.  If
   REF_P is true, tree fields will be written as references.  */

static void
pph_out_binding_level (pph_stream *stream, struct cp_binding_level *bl,
				bool ref_p)
{
  unsigned i;
  cp_class_binding *cs;
  cp_label_binding *sl;
  struct bitpack_d bp;

  if (!pph_start_record (stream, bl))
    return;

  pph_out_chain_filtered (stream, bl->names, ref_p, NO_BUILTINS);
  pph_out_uint (stream, bl->names_size);
  pph_out_chain_filtered (stream, bl->namespaces, ref_p, NO_BUILTINS);

  pph_out_tree_vec (stream, bl->static_decls, ref_p);

  pph_out_chain_filtered (stream, bl->usings, ref_p, NO_BUILTINS);
  pph_out_chain_filtered (stream, bl->using_directives, ref_p, NO_BUILTINS);

  pph_out_uint (stream, VEC_length (cp_class_binding, bl->class_shadowed));
  for (i = 0; VEC_iterate (cp_class_binding, bl->class_shadowed, i, cs); i++)
    pph_out_class_binding (stream, cs, ref_p);

  pph_out_tree_or_ref (stream, bl->type_shadowed, ref_p);

  pph_out_uint (stream, VEC_length (cp_label_binding, bl->shadowed_labels));
  for (i = 0; VEC_iterate (cp_label_binding, bl->shadowed_labels, i, sl); i++)
    pph_out_label_binding (stream, sl, ref_p);

  pph_out_chain (stream, bl->blocks, ref_p);
  pph_out_tree_or_ref (stream, bl->this_entity, ref_p);
  pph_out_binding_level (stream, bl->level_chain, ref_p);
  pph_out_tree_vec (stream, bl->dead_vars_from_for, ref_p);
  pph_out_chain (stream, bl->statement_list, ref_p);
  pph_out_uint (stream, bl->binding_depth);

  bp = bitpack_create (stream->ob->main_stream);
  bp_pack_value (&bp, bl->kind, 4);
  bp_pack_value (&bp, bl->keep, 1);
  bp_pack_value (&bp, bl->more_cleanups_ok, 1);
  bp_pack_value (&bp, bl->have_cleanups, 1);
  pph_out_bitpack (stream, &bp);
}


/* Write out the tree_common fields.  */

static void
pph_out_tree_common (pph_stream *stream, tree t, bool ref_p)
{
  /* The 'struct tree_typed typed' base class is handled in LTO.  */
  pph_out_tree_or_ref (stream, TREE_CHAIN (t), ref_p);
}


/* Write all the fields of c_language_function instance CLF to STREAM.  If
   REF_P is true, all tree fields should be written as references.  */

static void
pph_out_c_language_function (pph_stream *stream,
				      struct c_language_function *clf,
				      bool ref_p)
{
  if (!pph_start_record (stream, clf))
    return;

  pph_out_tree_vec (stream, clf->x_stmt_tree.x_cur_stmt_list, ref_p);
  pph_out_uint (stream, clf->x_stmt_tree.stmts_are_full_exprs_p);
}


/* Write all the fields of language_function instance LF to STREAM.  If
   REF_P is true, all tree fields should be written as references.  */

static void
pph_out_language_function (pph_stream *stream,
				    struct language_function *lf,
				    bool ref_p)
{
  struct bitpack_d bp;

  if (!pph_start_record (stream, lf))
    return;

  pph_out_c_language_function (stream, &lf->base, ref_p);
  pph_out_tree_or_ref (stream, lf->x_cdtor_label, ref_p);
  pph_out_tree_or_ref (stream, lf->x_current_class_ptr, ref_p);
  pph_out_tree_or_ref (stream, lf->x_current_class_ref, ref_p);
  pph_out_tree_or_ref (stream, lf->x_eh_spec_block, ref_p);
  pph_out_tree_or_ref (stream, lf->x_in_charge_parm, ref_p);
  pph_out_tree_or_ref (stream, lf->x_vtt_parm, ref_p);
  pph_out_tree_or_ref (stream, lf->x_return_value, ref_p);
  bp = bitpack_create (stream->ob->main_stream);
  bp_pack_value (&bp, lf->x_returns_value, 1);
  bp_pack_value (&bp, lf->x_returns_null, 1);
  bp_pack_value (&bp, lf->x_returns_abnormally, 1);
  bp_pack_value (&bp, lf->x_in_function_try_handler, 1);
  bp_pack_value (&bp, lf->x_in_base_initializer, 1);
  bp_pack_value (&bp, lf->can_throw, 1);
  pph_out_bitpack (stream, &bp);

  /* FIXME pph.  We are not writing lf->x_named_labels.  */

  pph_out_binding_level (stream, lf->bindings, ref_p);
  pph_out_tree_vec (stream, lf->x_local_names, ref_p);

  /* FIXME pph.  We are not writing lf->extern_decl_map.  */
}


/* Write all the fields of lang_decl_fn instance LDF to STREAM.  If REF_P
   is true, all tree fields should be written as references.  */

static void
pph_out_ld_fn (pph_stream *stream, struct lang_decl_fn *ldf,
			bool ref_p)
{
  struct bitpack_d bp;

  /* Write all the fields in lang_decl_min.  */
  pph_out_ld_min (stream, &ldf->min, ref_p);

  bp = bitpack_create (stream->ob->main_stream);
  bp_pack_value (&bp, ldf->operator_code, 16);
  bp_pack_value (&bp, ldf->global_ctor_p, 1);
  bp_pack_value (&bp, ldf->global_dtor_p, 1);
  bp_pack_value (&bp, ldf->constructor_attr, 1);
  bp_pack_value (&bp, ldf->destructor_attr, 1);
  bp_pack_value (&bp, ldf->assignment_operator_p, 1);
  bp_pack_value (&bp, ldf->static_function, 1);
  bp_pack_value (&bp, ldf->pure_virtual, 1);
  bp_pack_value (&bp, ldf->defaulted_p, 1);
  bp_pack_value (&bp, ldf->has_in_charge_parm_p, 1);
  bp_pack_value (&bp, ldf->has_vtt_parm_p, 1);
  bp_pack_value (&bp, ldf->pending_inline_p, 1);
  bp_pack_value (&bp, ldf->nonconverting, 1);
  bp_pack_value (&bp, ldf->thunk_p, 1);
  bp_pack_value (&bp, ldf->this_thunk_p, 1);
  bp_pack_value (&bp, ldf->hidden_friend_p, 1);
  pph_out_bitpack (stream, &bp);

  pph_out_tree_or_ref (stream, ldf->befriending_classes, ref_p);
  pph_out_tree_or_ref (stream, ldf->context, ref_p);

  if (ldf->thunk_p == 0)
    pph_out_tree_or_ref (stream, ldf->u5.cloned_function, ref_p);
  else if (ldf->thunk_p == 1)
    pph_out_uint (stream, ldf->u5.fixed_offset);
  else
    gcc_unreachable ();

  if (ldf->pending_inline_p == 1)
    pth_save_token_cache (ldf->u.pending_inline_info, stream);
  else if (ldf->pending_inline_p == 0)
    pph_out_language_function (stream, ldf->u.saved_language_function,
					ref_p);
}


/* Write all the fields of lang_decl_ns instance LDNS to STREAM.  If REF_P
   is true, all tree fields should be written as references.  */

static void
pph_out_ld_ns (pph_stream *stream, struct lang_decl_ns *ldns,
			bool ref_p)
{
  pph_out_binding_level (stream, ldns->level, ref_p);
}


/* Write all the fields of lang_decl_parm instance LDP to STREAM.  If REF_P
   is true, all tree fields should be written as references.  */

static void
pph_out_ld_parm (pph_stream *stream, struct lang_decl_parm *ldp)
{
  pph_out_uint (stream, ldp->level);
  pph_out_uint (stream, ldp->index);
}


/* Write all the lang-specific data in DECL to STREAM.  REF_P is true if
   the trees referenced in lang-specific fields should be written as
   references.  */

static void
pph_out_lang_specific (pph_stream *stream, tree decl, bool ref_p)
{
  struct lang_decl *ld;
  struct lang_decl_base *ldb;

  ld = DECL_LANG_SPECIFIC (decl);
  if (!pph_start_record (stream, ld))
    return;
    
  /* Write all the fields in lang_decl_base.  */
  ldb = &ld->u.base;
  pph_out_ld_base (stream, ldb);

  if (ldb->selector == 0)
    {
      /* Write all the fields in lang_decl_min.  */
      pph_out_ld_min (stream, &ld->u.min, ref_p);
    }
  else if (ldb->selector == 1)
    {
      /* Write all the fields in lang_decl_fn.  */
      pph_out_ld_fn (stream, &ld->u.fn, ref_p);
    }
  else if (ldb->selector == 2)
    {
      /* Write all the fields in lang_decl_ns.  */
      pph_out_ld_ns (stream, &ld->u.ns, ref_p);
    }
  else if (ldb->selector == 3)
    {
      /* Write all the fields in lang_decl_parm.  */
      pph_out_ld_parm (stream, &ld->u.parm);
    }
  else
    gcc_unreachable ();
}


/* Write all the fields in lang_type_header instance LTH to STREAM.  */

static void
pph_out_lang_type_header (pph_stream *stream,
				   struct lang_type_header *lth)
{
  struct bitpack_d bp;

  bp = bitpack_create (stream->ob->main_stream);
  bp_pack_value (&bp, lth->is_lang_type_class, 1);
  bp_pack_value (&bp, lth->has_type_conversion, 1);
  bp_pack_value (&bp, lth->has_copy_ctor, 1);
  bp_pack_value (&bp, lth->has_default_ctor, 1);
  bp_pack_value (&bp, lth->const_needs_init, 1);
  bp_pack_value (&bp, lth->ref_needs_init, 1);
  bp_pack_value (&bp, lth->has_const_copy_assign, 1);
  pph_out_bitpack (stream, &bp);
}


/* Write the vector V of tree_pair_s instances to STREAM.  REF_P is
   true if the trees should be written as references.  */

static void
pph_out_tree_pair_vec (pph_stream *stream, VEC(tree_pair_s,gc) *v,
				bool ref_p)
{
  unsigned i;
  tree_pair_s *p;

  pph_out_uint (stream, VEC_length (tree_pair_s, v));
  for (i = 0; VEC_iterate (tree_pair_s, v, i, p); i++)
    {
      pph_out_tree_or_ref (stream, p->purpose, ref_p);
      pph_out_tree_or_ref (stream, p->value, ref_p);
    }
}


/* Write a struct sorted_fields_type instance SFT to STREAM.  REF_P is
   true if the tree nodes should be written as references.  */

static void
pph_out_sorted_fields_type (pph_stream *stream,
				     struct sorted_fields_type *sft, bool ref_p)
{
  int i;

  if (!pph_start_record (stream, sft))
    return;

  pph_out_uint (stream, sft->len);
  for (i = 0; i < sft->len; i++)
    pph_out_tree_or_ref (stream, sft->elts[i], ref_p);
}


/* Write all the fields in lang_type_class instance LTC to STREAM.
   REF_P is true if all the trees in the structure should be written
   as references.  */

static void
pph_out_lang_type_class (pph_stream *stream,
				  struct lang_type_class *ltc, bool ref_p)
{
  struct bitpack_d bp;

  pph_out_uchar (stream, ltc->align);

  bp = bitpack_create (stream->ob->main_stream);
  bp_pack_value (&bp, ltc->has_mutable, 1);
  bp_pack_value (&bp, ltc->com_interface, 1);
  bp_pack_value (&bp, ltc->non_pod_class, 1);
  bp_pack_value (&bp, ltc->nearly_empty_p, 1);
  bp_pack_value (&bp, ltc->user_align, 1);
  bp_pack_value (&bp, ltc->has_copy_assign, 1);
  bp_pack_value (&bp, ltc->has_new, 1);
  bp_pack_value (&bp, ltc->has_array_new, 1);
  bp_pack_value (&bp, ltc->gets_delete, 2);
  bp_pack_value (&bp, ltc->interface_only, 1);
  bp_pack_value (&bp, ltc->interface_unknown, 1);
  bp_pack_value (&bp, ltc->contains_empty_class_p, 1);
  bp_pack_value (&bp, ltc->anon_aggr, 1);
  bp_pack_value (&bp, ltc->non_zero_init, 1);
  bp_pack_value (&bp, ltc->empty_p, 1);
  bp_pack_value (&bp, ltc->vec_new_uses_cookie, 1);
  bp_pack_value (&bp, ltc->declared_class, 1);
  bp_pack_value (&bp, ltc->diamond_shaped, 1);
  bp_pack_value (&bp, ltc->repeated_base, 1);
  bp_pack_value (&bp, ltc->being_defined, 1);
  bp_pack_value (&bp, ltc->java_interface, 1);
  bp_pack_value (&bp, ltc->debug_requested, 1);
  bp_pack_value (&bp, ltc->fields_readonly, 1);
  bp_pack_value (&bp, ltc->use_template, 2);
  bp_pack_value (&bp, ltc->ptrmemfunc_flag, 1);
  bp_pack_value (&bp, ltc->was_anonymous, 1);
  bp_pack_value (&bp, ltc->lazy_default_ctor, 1);
  bp_pack_value (&bp, ltc->lazy_copy_ctor, 1);
  bp_pack_value (&bp, ltc->lazy_copy_assign, 1);
  bp_pack_value (&bp, ltc->lazy_destructor, 1);
  bp_pack_value (&bp, ltc->has_const_copy_ctor, 1);
  bp_pack_value (&bp, ltc->has_complex_copy_ctor, 1);
  bp_pack_value (&bp, ltc->has_complex_copy_assign, 1);
  bp_pack_value (&bp, ltc->non_aggregate, 1);
  bp_pack_value (&bp, ltc->has_complex_dflt, 1);
  bp_pack_value (&bp, ltc->has_list_ctor, 1);
  bp_pack_value (&bp, ltc->non_std_layout, 1);
  bp_pack_value (&bp, ltc->is_literal, 1);
  bp_pack_value (&bp, ltc->lazy_move_ctor, 1);
  bp_pack_value (&bp, ltc->lazy_move_assign, 1);
  bp_pack_value (&bp, ltc->has_complex_move_ctor, 1);
  bp_pack_value (&bp, ltc->has_complex_move_assign, 1);
  bp_pack_value (&bp, ltc->has_constexpr_ctor, 1);
  pph_out_bitpack (stream, &bp);

  pph_out_tree_or_ref (stream, ltc->primary_base, ref_p);
  pph_out_tree_pair_vec (stream, ltc->vcall_indices, ref_p);
  pph_out_tree_or_ref (stream, ltc->vtables, ref_p);
  pph_out_tree_or_ref (stream, ltc->typeinfo_var, ref_p);
  pph_out_tree_vec (stream, ltc->vbases, ref_p);
  if (pph_start_record (stream, ltc->nested_udts))
    pph_out_binding_table (stream, ltc->nested_udts, ref_p);
  pph_out_tree_or_ref (stream, ltc->as_base, ref_p);
  pph_out_tree_vec (stream, ltc->pure_virtuals, ref_p);
  pph_out_tree_or_ref (stream, ltc->friend_classes, ref_p);
  pph_out_tree_vec (stream, ltc->methods, ref_p);
  pph_out_tree_or_ref (stream, ltc->key_method, ref_p);
  pph_out_tree_or_ref (stream, ltc->decl_list, ref_p);
  pph_out_tree_or_ref (stream, ltc->template_info, ref_p);
  pph_out_tree_or_ref (stream, ltc->befriending_classes, ref_p);
  pph_out_tree_or_ref (stream, ltc->objc_info, ref_p);
  pph_out_sorted_fields_type (stream, ltc->sorted_fields, ref_p);
  pph_out_tree_or_ref (stream, ltc->lambda_expr, ref_p);
}


/* Write struct lang_type_ptrmem instance LTP to STREAM.  If REF_P is
   true, all fields in the structure are written as references.  */

static void
pph_out_lang_type_ptrmem (pph_stream *stream, struct
				   lang_type_ptrmem *ltp, bool ref_p)
{
  pph_out_tree_or_ref (stream, ltp->record, ref_p);
}


/* Write all the lang-specific fields of TYPE to STREAM.  REF_P is
   true if tree nodes in the structure need to be written as
   references.  */

static void
pph_out_lang_type (pph_stream *stream, tree type, bool ref_p)
{
  struct lang_type *lt;

  lt = TYPE_LANG_SPECIFIC (type);
  if (!pph_start_record (stream, lt))
    return;

  pph_out_lang_type_header (stream, &lt->u.h);
  if (lt->u.h.is_lang_type_class)
    pph_out_lang_type_class (stream, &lt->u.c, ref_p);
  else
    pph_out_lang_type_ptrmem (stream, &lt->u.ptrmem, ref_p);
}


/* Write header information for some AST nodes not handled by the
   common streamer code.  EXPR is the tree to write to output block
   OB.  If EXPR does not need to be handled specially, do nothing.  */

void
pph_out_tree_header (struct output_block *ob, tree expr)
{
  pph_stream *stream = (pph_stream *) ob->sdata;

  if (TREE_CODE (expr) == CALL_EXPR)
    pph_out_uint (stream, call_expr_nargs (expr));
}


/* Callback for writing ASTs to a stream.  This writes all the fields
   that are not processed by default by the common tree pickler.
   OB and REF_P are as in lto_write_tree.  EXPR is the tree to write.  */

void
pph_write_tree (struct output_block *ob, tree expr, bool ref_p)
{
  pph_stream *stream = (pph_stream *) ob->sdata;

  switch (TREE_CODE (expr))
    {
    /* TREES NEEDING EXTRA WORK */

    /* tcc_declaration */

    case DEBUG_EXPR_DECL:
    case IMPORTED_DECL:
    case LABEL_DECL:
    case RESULT_DECL:
      pph_out_tree_or_ref_1 (stream, DECL_INITIAL (expr), ref_p, 3);
      break;

    case CONST_DECL:
    case FIELD_DECL:
    case NAMESPACE_DECL:
    case PARM_DECL:
    case USING_DECL:
    case VAR_DECL:
      /* FIXME pph: Should we merge DECL_INITIAL into lang_specific? */
      pph_out_tree_or_ref_1 (stream, DECL_INITIAL (expr), ref_p, 3);
      pph_out_lang_specific (stream, expr, ref_p);
      break;

    case FUNCTION_DECL:
      pph_out_tree_or_ref_1 (stream, DECL_INITIAL (expr), ref_p, 3);
      pph_out_lang_specific (stream, expr, ref_p);
      pph_out_tree_or_ref_1 (stream, DECL_SAVED_TREE (expr), ref_p, 3);
      break;

    case TYPE_DECL:
      pph_out_tree_or_ref_1 (stream, DECL_INITIAL (expr), ref_p, 3);
      pph_out_lang_specific (stream, expr, ref_p);
      pph_out_tree_or_ref_1 (stream, DECL_ORIGINAL_TYPE (expr), ref_p, 3);
      break;

    case TEMPLATE_DECL:
      pph_out_tree_or_ref_1 (stream, DECL_INITIAL (expr), ref_p, 3);
      pph_out_lang_specific (stream, expr, ref_p);
      pph_out_tree_or_ref_1 (stream, DECL_TEMPLATE_RESULT (expr), ref_p, 3);
      pph_out_tree_or_ref_1 (stream, DECL_TEMPLATE_PARMS (expr), ref_p, 3);
      pph_out_tree_or_ref_1 (stream, DECL_CONTEXT (expr), ref_p, 3);
      break;

    /* tcc_type */

    case ARRAY_TYPE:
    case BOOLEAN_TYPE:
    case COMPLEX_TYPE:
    case ENUMERAL_TYPE:
    case FIXED_POINT_TYPE:
    case FUNCTION_TYPE:
    case INTEGER_TYPE:
    case LANG_TYPE:
    case METHOD_TYPE:
    case NULLPTR_TYPE:
    case OFFSET_TYPE:
    case POINTER_TYPE:
    case REAL_TYPE:
    case REFERENCE_TYPE:
    case VECTOR_TYPE:
    case VOID_TYPE:
      pph_out_lang_type (stream, expr, ref_p);
      break;

    case QUAL_UNION_TYPE:
    case RECORD_TYPE:
    case UNION_TYPE:
      pph_out_lang_type (stream, expr, ref_p);
      pph_out_tree_or_ref_1 (stream, TYPE_BINFO (expr), ref_p, 3);
      break;

    case BOUND_TEMPLATE_TEMPLATE_PARM:
    case DECLTYPE_TYPE:
    case TEMPLATE_TEMPLATE_PARM:
    case TEMPLATE_TYPE_PARM:
    case TYPENAME_TYPE:
    case TYPEOF_TYPE:
      pph_out_lang_type (stream, expr, ref_p);
      pph_out_tree_or_ref_1 (stream, TYPE_CACHED_VALUES (expr), ref_p, 3);
      /* Note that we are using TYPED_CACHED_VALUES for it access to
         the generic .values field of types. */
      break;

    /* tcc_statement */

    case STATEMENT_LIST:
      {
        tree_stmt_iterator i;
        unsigned num_stmts;

        /* Compute and write the number of statements in the list.  */
        for (num_stmts = 0, i = tsi_start (expr); !tsi_end_p (i); tsi_next (&i))
	  num_stmts++;

        pph_out_uint (stream, num_stmts);

        /* Write the statements.  */
        for (i = tsi_start (expr); !tsi_end_p (i); tsi_next (&i))
	  pph_out_tree_or_ref_1 (stream, tsi_stmt (i), ref_p, 3);
      }
      break;

    /* tcc_expression */

    /* tcc_unary */

    /* tcc_vl_exp */

    /* tcc_reference */

    /* tcc_constant */

    /* tcc_exceptional */

    case OVERLOAD:
      pph_out_tree_common (stream, expr, ref_p);
      pph_out_tree_or_ref_1 (stream, OVL_CURRENT (expr), ref_p, 3);
      break;

    case IDENTIFIER_NODE:
      {
        struct lang_identifier *id = LANG_IDENTIFIER_CAST (expr);
        pph_out_cxx_binding (stream, id->namespace_bindings, ref_p);
        pph_out_cxx_binding (stream, id->bindings, ref_p);
        pph_out_tree_or_ref_1 (stream, id->class_template_info, ref_p, 3);
        pph_out_tree_or_ref_1 (stream, id->label_value, ref_p, 3);
	pph_out_tree_or_ref_1 (stream, TREE_TYPE (expr), ref_p, 3);
      }
      break;

    case BASELINK:
      pph_out_tree_common (stream, expr, ref_p);
      pph_out_tree_or_ref_1 (stream, BASELINK_BINFO (expr), ref_p, 3);
      pph_out_tree_or_ref_1 (stream, BASELINK_FUNCTIONS (expr), ref_p, 3);
      pph_out_tree_or_ref_1 (stream, BASELINK_ACCESS_BINFO (expr), ref_p, 3);
      break;

    case TEMPLATE_INFO:
      pph_out_tree_common (stream, expr, ref_p);
      pph_out_qual_use_vec (stream,
          TI_TYPEDEFS_NEEDING_ACCESS_CHECKING (expr), ref_p);
      break;

    case TEMPLATE_PARM_INDEX: 
      {
        template_parm_index *p = TEMPLATE_PARM_INDEX_CAST (expr);
        pph_out_tree_common (stream, expr, ref_p);
        pph_out_uint (stream, p->index);
        pph_out_uint (stream, p->level);
        pph_out_uint (stream, p->orig_level);
        pph_out_uint (stream, p->num_siblings);
        pph_out_tree_or_ref_1 (stream, p->decl, ref_p, 3);
        /* FIXME pph: Is TEMPLATE_PARM_PARAMETER_PACK using TREE_LANG_FLAG_0
           already handled?  */
      }
      break;

    /* tcc_constant */

    case PTRMEM_CST:
      pph_out_tree_common (stream, expr, ref_p);
      pph_out_tree_or_ref_1 (stream, PTRMEM_CST_MEMBER (expr), ref_p, 3);
      break;

    /* tcc_exceptional */

    case DEFAULT_ARG:
      pph_out_tree_common (stream, expr, ref_p);
      pth_save_token_cache (DEFARG_TOKENS (expr), stream);
      pph_out_tree_vec (stream, DEFARG_INSTANTIATIONS (expr), ref_p);
      break;

    case STATIC_ASSERT:
      pph_out_tree_common (stream, expr, ref_p);
      pph_out_tree_or_ref_1 (stream, STATIC_ASSERT_CONDITION (expr), ref_p, 3);
      pph_out_tree_or_ref_1 (stream, STATIC_ASSERT_MESSAGE (expr), ref_p, 3);
      /* FIXME pph: also STATIC_ASSERT_SOURCE_LOCATION (expr).  */
      break;

    case ARGUMENT_PACK_SELECT:
      pph_out_tree_common (stream, expr, ref_p);
      pph_out_tree_or_ref_1 (stream, ARGUMENT_PACK_SELECT_FROM_PACK (expr),
                                     ref_p, 3);
      pph_out_uint (stream, ARGUMENT_PACK_SELECT_INDEX (expr));
      break;

    case TRAIT_EXPR:
      pph_out_tree_common (stream, expr, ref_p);
      pph_out_tree_or_ref_1 (stream, TRAIT_EXPR_TYPE1 (expr), ref_p, 3);
      pph_out_tree_or_ref_1 (stream, TRAIT_EXPR_TYPE2 (expr), ref_p, 3);
      pph_out_uint (stream, TRAIT_EXPR_KIND (expr));
      break;

    case LAMBDA_EXPR:
      {
        struct tree_lambda_expr *e
            = (struct tree_lambda_expr *)LAMBDA_EXPR_CHECK (expr);
        pph_out_tree_common (stream, expr, ref_p);
        /* FIXME pph: also e->locus.  */
        pph_out_tree_or_ref_1 (stream, e->capture_list, ref_p, 3);
        pph_out_tree_or_ref_1 (stream, e->this_capture, ref_p, 3);
        pph_out_tree_or_ref_1 (stream, e->return_type, ref_p, 3);
        pph_out_tree_or_ref_1 (stream, e->extra_scope, ref_p, 3);
        pph_out_uint (stream, e->discriminator);
      }
      break;


    /* TREES ALREADY HANDLED */

    /* tcc_declaration */

    case TRANSLATION_UNIT_DECL:

    /* tcc_exceptional */

    case TREE_BINFO:
    case TREE_LIST:
    case TREE_VEC:

      break;

    /* TREES UNIMPLEMENTED */

    /* tcc_declaration */

    /* tcc_type */

    case TYPE_ARGUMENT_PACK:
    case TYPE_PACK_EXPANSION:
    case UNBOUND_CLASS_TEMPLATE:

    /* tcc_statement */

    case USING_STMT:
    case TRY_BLOCK:
    case EH_SPEC_BLOCK:
    case HANDLER:
    case CLEANUP_STMT:
    case IF_STMT:
    case FOR_STMT:
    case RANGE_FOR_STMT:
    case WHILE_STMT:
    case DO_STMT:
    case BREAK_STMT:
    case CONTINUE_STMT:
    case SWITCH_STMT:

    /* tcc_expression */

    case NEW_EXPR:
    case VEC_NEW_EXPR:
    case DELETE_EXPR:
    case VEC_DELETE_EXPR:
    case TYPE_EXPR:
    case VEC_INIT_EXPR:
    case THROW_EXPR:
    case EMPTY_CLASS_EXPR:
    case TEMPLATE_ID_EXPR:
    case PSEUDO_DTOR_EXPR:
    case MODOP_EXPR:
    case DOTSTAR_EXPR:
    case TYPEID_EXPR:
    case NON_DEPENDENT_EXPR:
    case CTOR_INITIALIZER:
    case MUST_NOT_THROW_EXPR:
    case EXPR_STMT:
    case TAG_DEFN:
    case OFFSETOF_EXPR:
    case SIZEOF_EXPR:
    case ARROW_EXPR:
    case ALIGNOF_EXPR:
    case AT_ENCODE_EXPR:
    case STMT_EXPR:
    case NONTYPE_ARGUMENT_PACK:
    case EXPR_PACK_EXPANSION:

    /* tcc_unary */

    case CAST_EXPR:
    case REINTERPRET_CAST_EXPR:
    case CONST_CAST_EXPR:
    case STATIC_CAST_EXPR:
    case DYNAMIC_CAST_EXPR:
    case NOEXCEPT_EXPR:
    case UNARY_PLUS_EXPR:

    /* tcc_reference */

    case MEMBER_REF:
    case OFFSET_REF:
    case SCOPE_REF:

    /* tcc_vl_exp */

    case AGGR_INIT_EXPR:


      if (flag_pph_untree)
        fprintf (pph_logfile, "PPH: unimplemented tree node %s\n",
                 tree_code_name[TREE_CODE (expr)]);
      break;


    /* TREES UNRECOGNIZED */

    default:
      if (flag_pph_untree)
        fprintf (pph_logfile, "PPH: unrecognized tree node %s\n",
                 tree_code_name[TREE_CODE (expr)]);
    }
}
