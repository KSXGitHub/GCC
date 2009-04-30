/* Graphite polyhedral representation.
   Copyright (C) 2009 Free Software Foundation, Inc.
   Contributed by Sebastian Pop <sebastian.pop@amd.com> and
   Tobias Grosser <grosser@fim.uni-passau.de>.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "ggc.h"
#include "tree.h"
#include "rtl.h"
#include "output.h"
#include "basic-block.h"
#include "diagnostic.h"
#include "tree-flow.h"
#include "toplev.h"
#include "tree-dump.h"
#include "timevar.h"
#include "cfgloop.h"
#include "tree-chrec.h"
#include "tree-data-ref.h"
#include "tree-scalar-evolution.h"
#include "tree-pass.h"
#include "domwalk.h"
#include "value-prof.h"
#include "pointer-set.h"
#include "gimple.h"
#include "params.h"

#ifdef HAVE_cloog
#include "cloog/cloog.h"
#include "ppl_c.h"
#include "sese.h"
#include "graphite-ppl.h"
#include "graphite.h"
#include "graphite-poly.h"
#include "graphite-data-ref.h"

/* Return the maximal loop depth in SCOP.  */

int
scop_max_loop_depth (scop_p scop)
{
  int i;
  poly_bb_p pbb;
  int max_nb_loops = 0;

  for (i = 0; VEC_iterate (poly_bb_p, SCOP_BBS (scop), i, pbb); i++) 
    {    
      int nb_loops = pbb_nb_loops (pbb);
      if (max_nb_loops < nb_loops)
        max_nb_loops = nb_loops;
    }    

  return max_nb_loops;
}

/* Extend the scattering matrix of PBB to MAX_SCATTERING scattering
   dimensions.  */

static void
extend_scattering (poly_bb_p pbb, int max_scattering)
{
  ppl_dimension_type nb_old_dims, nb_new_dims;
  int nb_added_dims, i;

  nb_added_dims = max_scattering - pbb_nb_scattering (pbb); 

  gcc_assert (nb_added_dims >= 0);

  nb_old_dims = pbb_nb_scattering (pbb) + pbb_nb_loops (pbb)
    + scop_nb_params (PBB_SCOP (pbb));
  nb_new_dims = nb_old_dims + nb_added_dims;

  ppl_insert_dimensions (PBB_TRANSFORMED_SCATTERING (pbb),
			 pbb_nb_scattering (pbb), nb_added_dims);

  for (i = max_scattering - nb_added_dims; i < max_scattering; i++)
    {
      ppl_Constraint_t cstr;
      ppl_Coefficient_t coef;
      ppl_Linear_Expression_t expr;
      Value v;

      value_init (v);
      value_set_si (v, 1);
      ppl_new_Coefficient (&coef);
      ppl_new_Linear_Expression_with_dimension (&expr, nb_new_dims);
      ppl_assign_Coefficient_from_mpz_t (coef, v);
      ppl_Linear_Expression_add_to_coefficient (expr, i, coef);
      ppl_new_Constraint (&cstr, expr, PPL_CONSTRAINT_TYPE_EQUAL);
      ppl_Polyhedron_add_constraint (PBB_TRANSFORMED_SCATTERING (pbb), cstr);
      ppl_delete_Constraint (cstr);
      ppl_delete_Coefficient (coef);
      ppl_delete_Linear_Expression (expr);
    }
}

/* All scattering matrices in SCOP will have the same number of scattering
   dimensions.  */

int
unify_scattering_dimensions (scop_p scop)
{
  int i;
  poly_bb_p pbb;
  graphite_dim_t max_scattering = 0;

  for (i = 0; VEC_iterate (poly_bb_p, SCOP_BBS (scop), i, pbb); i++)
    max_scattering = MAX (pbb_nb_scattering (pbb), max_scattering);
  
  for (i = 0; VEC_iterate (poly_bb_p, SCOP_BBS (scop), i, pbb); i++)
    extend_scattering (pbb, max_scattering);

  return max_scattering;
}

/* Prints to FILE the scattering function of PBB.  */

void
print_scattering_function (FILE *file, poly_bb_p pbb)
{
  fprintf (file, "scattering bb_%d (\n", GBB_BB (PBB_BLACK_BOX (pbb))->index);

  if (PBB_TRANSFORMED_SCATTERING (pbb))
    ppl_print_polyhedron_matrix (file, PBB_TRANSFORMED_SCATTERING (pbb));

  fprintf (file, ")\n");
}

/* Prints to FILE the iteration domain of PBB.  */

void
print_iteration_domain (FILE *file, poly_bb_p pbb)
{
  print_pbb_domain (file, pbb);
}

/* Prints to FILE the scattering functions of every PBB of SCOP.  */

void
print_scattering_functions (FILE *file, scop_p scop)
{
  int i;
  poly_bb_p pbb;

  for (i = 0; VEC_iterate (poly_bb_p, SCOP_BBS (scop), i, pbb); i++)
    print_scattering_function (file, pbb);
}

/* Prints to FILE the iteration domains of every PBB of SCOP.  */

void
print_iteration_domains (FILE *file, scop_p scop)
{
  int i;
  poly_bb_p pbb;

  for (i = 0; VEC_iterate (poly_bb_p, SCOP_BBS (scop), i, pbb); i++)
    print_iteration_domain (file, pbb);
}

/* Prints to STDERR the scattering function of PBB.  */

void
debug_scattering_function (poly_bb_p pbb)
{
  print_scattering_function (stderr, pbb);
}

/* Prints to STDERR the iteration domain of PBB.  */

void
debug_iteration_domain (poly_bb_p pbb)
{
  print_iteration_domain (stderr, pbb);
}

/* Prints to STDERR the scattering functions of every PBB of SCOP.  */

void
debug_scattering_functions (scop_p scop)
{
  print_scattering_functions (stderr, scop);
}

/* Prints to STDERR the iteration domains of every PBB of SCOP.  */

void
debug_iteration_domains (scop_p scop)
{
  print_iteration_domains (stderr, scop);
}


/* Write to file_name.graphite the transforms for SCOP.  */

static void
graphite_write_transforms (scop_p scop)
{
  print_scattering_functions (graphite_out_file, scop);
}

/* Read transforms from file_name.graphite and set the transforms on
   SCOP.  */

static bool 
graphite_read_transforms (scop_p scop)
{
  int i;
  poly_bb_p pbb;

  for (i = 0; VEC_iterate (poly_bb_p, SCOP_BBS (scop), i, pbb); i++)
    {
      ppl_Polyhedron_t newp;
      ppl_read_polyhedron_matrix (&newp, graphite_in_file);
      PBB_TRANSFORMED_SCATTERING (pbb) = newp;
    }

  return true;
}

/* Apply graphite transformations to all the basic blocks of SCOP.  */

bool
apply_poly_transforms (scop_p scop)
{
  bool transform_done = false;

  if (flag_graphite_read)
    transform_done |= graphite_read_transforms (scop);

  /* Generate code even if we did not apply any real transformation.
     This also allows to check the performance for the identity
     transformation: GIMPLE -> GRAPHITE -> GIMPLE
     Keep in mind that CLooG optimizes in control, so the loop structure
     may change, even if we only use -fgraphite-identity.  */ 
  if (flag_graphite_identity)
    transform_done = true;

  if (flag_graphite_force_parallel)
    transform_done = true;

  if (flag_loop_block)
    gcc_unreachable (); /* Not yet supported.  */

  if (flag_loop_interchange)
    gcc_unreachable (); /* Not yet supported.  */

  if (flag_loop_strip_mine)
    gcc_unreachable (); /* Not yet supported.  */

  if (flag_graphite_write)
    graphite_write_transforms (scop);

  return transform_done;
}

/* Create a new polyhedral data reference and add it to PBB. It is defined by
   its ACCESSES, its TYPE*/

void
new_poly_dr (poly_bb_p pbb, ppl_Pointset_Powerset_NNC_Polyhedron_t accesses,
	     enum POLY_DR_TYPE type)
{
  poly_dr_p pdr = XNEW (struct poly_dr);

  PDR_BB (pdr) = pbb;
  PDR_ACCESSES (pdr) = accesses;
  PDR_TYPE (pdr) = type;

  VEC_safe_push (poly_dr_p, heap, PBB_DRS (pbb), pdr);
}

/* Free polyhedral data reference PDR.  */

void
free_poly_dr (poly_dr_p pdr)
{
  ppl_delete_Pointset_Powerset_NNC_Polyhedron (PDR_ACCESSES (pdr));

  XDELETE (pdr);
}

/* Create a new polyhedral black box.  */

void
new_poly_bb (scop_p scop, void *black_box)
{
  poly_bb_p pbb = XNEW (struct poly_bb);

  PBB_DOMAIN (pbb) = NULL;
  PBB_SCOP (pbb) = scop;
  pbb_set_black_box (pbb, black_box);
  PBB_TRANSFORMED_SCATTERING (pbb) = NULL;
  PBB_ORIGINAL_SCATTERING (pbb) = NULL;
  PBB_DRS (pbb) = VEC_alloc (poly_dr_p, heap, 3);
  VEC_safe_push (poly_bb_p, heap, SCOP_BBS (scop), pbb);
}

/* Free polyhedral black box.  */

void
free_poly_bb (poly_bb_p pbb)
{
  int i;
  poly_dr_p pdr;
  
  ppl_delete_Pointset_Powerset_NNC_Polyhedron (PBB_DOMAIN (pbb));

  if (PBB_TRANSFORMED_SCATTERING (pbb))
    ppl_delete_Polyhedron (PBB_TRANSFORMED_SCATTERING (pbb));

  if (PBB_ORIGINAL_SCATTERING (pbb))
    ppl_delete_Polyhedron (PBB_ORIGINAL_SCATTERING (pbb));

  if (PBB_DRS (pbb))
    for (i = 0; VEC_iterate (poly_dr_p, PBB_DRS (pbb), i, pdr); i++)
      free_poly_dr (pdr);

  VEC_free (poly_dr_p, heap, PBB_DRS (pbb));
  XDELETE (pbb);
}

/* Creates a new SCOP containing REGION.  */

scop_p
new_scop (void *region)
{
  scop_p scop = XNEW (struct scop);

  SCOP_DEP_GRAPH (scop) = NULL;
  scop_set_region (scop, region);
  SCOP_BBS (scop) = VEC_alloc (poly_bb_p, heap, 3);

  return scop;
}

/* Deletes SCOP.  */

void
free_scop (scop_p scop)
{
  int i;
  poly_bb_p pbb;

  for (i = 0; VEC_iterate (poly_bb_p, SCOP_BBS (scop), i, pbb); i++)
    free_poly_bb (pbb);

  VEC_free (poly_bb_p, heap, SCOP_BBS (scop));

  XDELETE (scop);
}

/* Print to FILE the domain of PBB.  */

void
print_pbb_domain (FILE *file, poly_bb_p pbb)
{
  gimple_bb_p gbb = PBB_BLACK_BOX (pbb);

  fprintf (file, "domains bb_%d (", GBB_BB (gbb)->index);

  if (PBB_DOMAIN (pbb))
    ppl_print_powerset_matrix (file, PBB_DOMAIN (pbb));

  fprintf (file, ")\n");
}

/* Dump the cases of a graphite basic block GBB on FILE.  */

static void
dump_gbb_cases (FILE *file, gimple_bb_p gbb)
{
  int i;
  gimple stmt;
  VEC (gimple, heap) *cases;
  
  if (!gbb)
    return;

  cases = GBB_CONDITION_CASES (gbb);
  if (VEC_empty (gimple, cases))
    return;

  fprintf (file, "cases bb_%d (", GBB_BB (gbb)->index);

  for (i = 0; VEC_iterate (gimple, cases, i, stmt); i++)
    print_gimple_stmt (file, stmt, 0, 0);

  fprintf (file, ")\n");
}

/* Dump conditions of a graphite basic block GBB on FILE.  */

static void
dump_gbb_conditions (FILE *file, gimple_bb_p gbb)
{
  int i;
  gimple stmt;
  VEC (gimple, heap) *conditions;
  
  if (!gbb)
    return;

  conditions = GBB_CONDITIONS (gbb);
  if (VEC_empty (gimple, conditions))
    return;

  fprintf (file, "conditions bb_%d (", GBB_BB (gbb)->index);

  for (i = 0; VEC_iterate (gimple, conditions, i, stmt); i++)
    print_gimple_stmt (file, stmt, 0, 0);

  fprintf (file, ")\n");
}

/* Print to FILE the domain and scattering function of PBB.  */

void
print_pbb (FILE *file, poly_bb_p pbb)
{
  dump_gbb_conditions (file, PBB_BLACK_BOX (pbb));
  dump_gbb_cases (file, PBB_BLACK_BOX (pbb));
  print_pbb_domain (file, pbb);
  print_scattering_function (file, pbb);
}

/* Print to FILE the SCOP.  */

void
print_scop (FILE *file, scop_p scop)
{
  int i;
  poly_bb_p pbb;

  for (i = 0; VEC_iterate (poly_bb_p, SCOP_BBS (scop), i, pbb); i++)
    print_pbb (file, pbb);
}

/* Print to STDERR the domain of PBB.  */

void
debug_pbb_domain (poly_bb_p pbb)
{
  print_pbb_domain (stderr, pbb);
}

/* Print to FILE the domain and scattering function of PBB.  */

void
debug_pbb (poly_bb_p pbb)
{
  print_pbb (stderr, pbb);
}

/* Print to STDERR the SCOP.  */

void
debug_scop (scop_p scop)
{
  print_scop (stderr, scop);
}



/* Returns a polyhedron of dimension DIM.

   Maps the dimensions [0, ..., cut - 1] of polyhedron P to OFFSET0
   and the dimensions [cut, ..., nb_dim] to DIM - GDIM.  */

static ppl_Pointset_Powerset_NNC_Polyhedron_t
map_into_dep_poly (graphite_dim_t dim, graphite_dim_t gdim,
		   ppl_Pointset_Powerset_NNC_Polyhedron_t p,
		   graphite_dim_t cut,
		   graphite_dim_t offset)
{
  ppl_Pointset_Powerset_NNC_Polyhedron_t res;

  ppl_new_Pointset_Powerset_NNC_Polyhedron_from_Pointset_Powerset_NNC_Polyhedron (&res, p);
  ppl_insert_dimensions_pointset (res, 0, offset);
  ppl_insert_dimensions_pointset (res, offset + cut, dim - offset - cut - gdim);

  return res;
}

/* Swap [cut0, ..., cut1] to the end of DR: "a CUT0 b CUT1 c" is
   transformed into "a CUT0 c CUT1' b"

   Add NB0 zeros before "a":  "00...0 a CUT0 c CUT1' b"
   Add NB1 zeros between "a" and "c":  "00...0 a 00...0 c CUT1' b"
   Add DIM - NB0 - NB1 - PDIM zeros between "c" and "b":  "00...0 a 00...0 c 00...0 b"
*/

static ppl_Pointset_Powerset_NNC_Polyhedron_t
map_dr_into_dep_poly (graphite_dim_t dim,
		      ppl_Pointset_Powerset_NNC_Polyhedron_t dr, 
		      graphite_dim_t cut0, graphite_dim_t cut1,
		      graphite_dim_t nb0, graphite_dim_t nb1)
{
  ppl_dimension_type pdim;
  ppl_dimension_type *map;
  ppl_Pointset_Powerset_NNC_Polyhedron_t res;
  ppl_dimension_type i;

  ppl_new_Pointset_Powerset_NNC_Polyhedron_from_Pointset_Powerset_NNC_Polyhedron (&res, dr);
  ppl_Pointset_Powerset_NNC_Polyhedron_space_dimension (res, &pdim);

  map = (ppl_dimension_type *) XNEWVEC (ppl_dimension_type, pdim);

  /* First mapping: move 'g' vector to right position.  */
  for (i = 0; i < cut0; i++)
    map[i] = i;

  for (i = cut0; i < cut1; i++)
    map[i] = pdim - cut1 + i;

  for (i = cut1; i < pdim; i++)
    map[i] = cut0 + i - cut1;

  ppl_Pointset_Powerset_NNC_Polyhedron_map_space_dimensions (res, map, pdim);
  free (map);

  /* After swapping 's' and 'g' vectors, we have to update a new cut.  */
  cut1 = pdim - cut1 + cut0;
  
  ppl_insert_dimensions_pointset (res, 0, nb0);
  ppl_insert_dimensions_pointset (res, nb0 + cut0, nb1);
  ppl_insert_dimensions_pointset (res, nb0 + nb1 + cut1, dim - nb0 - nb1 - pdim);

  return res;
}

/* Builds a constraints of the form "POS1 - POS2 CSTR_TYPE C" */

static ppl_Constraint_t
build_pairwise_constraint (graphite_dim_t dim,
		           graphite_dim_t pos1, graphite_dim_t pos2,
			   int c, enum ppl_enum_Constraint_Type cstr_type)
{
  ppl_Linear_Expression_t expr;
  ppl_Constraint_t cstr;
  ppl_Coefficient_t coef;
  Value v, v_op, v_c;

  value_init (v);
  value_init (v_op);
  value_init (v_c);
 
  value_set_si (v, 1);
  value_set_si (v_op, -1);
  value_set_si (v_c, c);

  ppl_new_Coefficient (&coef);
  ppl_new_Linear_Expression_with_dimension (&expr, dim);
  
  ppl_assign_Coefficient_from_mpz_t (coef, v);
  ppl_Linear_Expression_add_to_coefficient (expr, pos1, coef);
  ppl_assign_Coefficient_from_mpz_t (coef, v_op);
  ppl_Linear_Expression_add_to_coefficient (expr, pos2, coef);
  ppl_assign_Coefficient_from_mpz_t (coef, v_c);
  ppl_Linear_Expression_add_to_inhomogeneous (expr, coef);

  ppl_new_Constraint (&cstr, expr, cstr_type);

  ppl_delete_Linear_Expression (expr);
  ppl_delete_Coefficient (coef);
  value_clear (v);
  value_clear (v_op);
  value_clear (v_c);

  return cstr;
}

/* Builds subscript equality constraints.  */

static ppl_Pointset_Powerset_NNC_Polyhedron_t
dr_equality_constraints (graphite_dim_t dim,
		         graphite_dim_t pos, graphite_dim_t nb_subscripts)
{
  ppl_Polyhedron_t subscript_equalities;
  ppl_Pointset_Powerset_NNC_Polyhedron_t res;
  Value v, v_op;
  unsigned int i;

  value_init (v);
  value_init (v_op);
  value_set_si (v, 1);
  value_set_si (v_op, -1);

  ppl_new_NNC_Polyhedron_from_space_dimension (&subscript_equalities, dim, 0);
  for (i = 0; i < nb_subscripts; i++)
    {
      ppl_Linear_Expression_t expr;
      ppl_Constraint_t cstr;
      ppl_Coefficient_t coef;
      
      ppl_new_Coefficient (&coef);
      ppl_new_Linear_Expression_with_dimension (&expr, dim);
      
      ppl_assign_Coefficient_from_mpz_t (coef, v);
      ppl_Linear_Expression_add_to_coefficient (expr, pos + i, coef);
      ppl_assign_Coefficient_from_mpz_t (coef, v_op);
      ppl_Linear_Expression_add_to_coefficient (expr, pos + i + nb_subscripts, coef);

      ppl_new_Constraint (&cstr, expr, PPL_CONSTRAINT_TYPE_EQUAL);
      ppl_Polyhedron_add_constraint (subscript_equalities, cstr);

      ppl_delete_Linear_Expression (expr);
      ppl_delete_Constraint (cstr);
      ppl_delete_Coefficient (coef);
    }

  ppl_new_Pointset_Powerset_NNC_Polyhedron_from_NNC_Polyhedron (&res, subscript_equalities);
  value_clear (v);
  value_clear (v_op);
  ppl_delete_Polyhedron (subscript_equalities);

  return res;
}

/* Builds scheduling equality constraints.  */

static ppl_Pointset_Powerset_NNC_Polyhedron_t
build_pairwise_scheduling_equality (graphite_dim_t dim,
		                    graphite_dim_t pos, graphite_dim_t offset)
{
  ppl_Pointset_Powerset_NNC_Polyhedron_t res;
  ppl_Polyhedron_t equalities;
  ppl_Constraint_t cstr;

  ppl_new_NNC_Polyhedron_from_space_dimension (&equalities, dim, 0);

  cstr = build_pairwise_constraint (dim, pos, pos + offset, 0, PPL_CONSTRAINT_TYPE_EQUAL); 
  ppl_Polyhedron_add_constraint (equalities, cstr);
  ppl_delete_Constraint (cstr);

  ppl_new_Pointset_Powerset_NNC_Polyhedron_from_NNC_Polyhedron (&res, equalities);
  ppl_delete_Polyhedron (equalities);
  return res;
}

/* Builds scheduling inequality constraints.  */

static ppl_Pointset_Powerset_NNC_Polyhedron_t
build_pairwise_scheduling_inequality (graphite_dim_t dim,
				      graphite_dim_t pos,
				      graphite_dim_t offset,
				      bool direction)
{
  ppl_Pointset_Powerset_NNC_Polyhedron_t res;
  ppl_Polyhedron_t equalities;
  ppl_Constraint_t cstr;

  ppl_new_NNC_Polyhedron_from_space_dimension (&equalities, dim, 0);

  if (direction == 1)
    cstr = build_pairwise_constraint (dim, pos, pos + offset, 0, PPL_CONSTRAINT_TYPE_GREATER_THAN); 
  else
    cstr = build_pairwise_constraint (dim, pos, pos + offset, 0, PPL_CONSTRAINT_TYPE_LESS_THAN); 

  ppl_Polyhedron_add_constraint (equalities, cstr);
  ppl_delete_Constraint (cstr);

  ppl_new_Pointset_Powerset_NNC_Polyhedron_from_NNC_Polyhedron (&res, equalities);
  ppl_delete_Polyhedron (equalities);
  return res;
}

/* Returns true when adding the lexicographical constraints at level I
   to the RES dependence polyhedron returns an empty polyhedron.  */

static bool
lexicographically_gt_p (ppl_Pointset_Powerset_NNC_Polyhedron_t res,
			graphite_dim_t dim,
			graphite_dim_t offset,
			bool direction, graphite_dim_t i)
{
  ppl_Pointset_Powerset_NNC_Polyhedron_t ineq;
  bool empty_p;

  ineq = build_pairwise_scheduling_inequality (dim, i, offset,
					       direction);
  ppl_Pointset_Powerset_NNC_Polyhedron_intersection_assign (ineq, res);

  empty_p = ppl_Pointset_Powerset_NNC_Polyhedron_is_empty (ineq);
  ppl_Pointset_Powerset_NNC_Polyhedron_intersection_assign (res, ineq);
  ppl_delete_Pointset_Powerset_NNC_Polyhedron (ineq);

  return empty_p;
}

/* Build the precedence constraints for the lexicographical comparison
   of time vectors RES following the lexicographical order.  */

static void
build_lexicographically_gt_constraint (ppl_Pointset_Powerset_NNC_Polyhedron_t res,
				       graphite_dim_t dim,
				       graphite_dim_t tdim1, graphite_dim_t offset,
				       bool direction)
{
  graphite_dim_t i;

  if (!lexicographically_gt_p (res, dim, offset, direction, 0))
    return;

  for (i = 0; i < tdim1 - 1; i++)
    {
      ppl_Pointset_Powerset_NNC_Polyhedron_t sceq;

      sceq = build_pairwise_scheduling_equality (dim, i, offset);
      ppl_Pointset_Powerset_NNC_Polyhedron_intersection_assign (res, sceq);
      ppl_delete_Pointset_Powerset_NNC_Polyhedron (sceq);

      if (!lexicographically_gt_p (res, dim, offset, direction, i + 1))
	return;
    }
}

/*  Build the dependence polyhedron for data references PDR1 and PDR2.  */


static ppl_Pointset_Powerset_NNC_Polyhedron_t
dependence_polyhedron (poly_bb_p pbb1, poly_bb_p pbb2,
		       ppl_Pointset_Powerset_NNC_Polyhedron_t d1,
		       ppl_Pointset_Powerset_NNC_Polyhedron_t d2,
		       poly_dr_p pdr1, poly_dr_p pdr2,
	               ppl_Polyhedron_t s1, ppl_Polyhedron_t s2,
		       bool direction)
{
  scop_p scop = PBB_SCOP (pbb1);
  graphite_dim_t tdim1 = pbb_nb_scattering_dims (s1, pbb1, scop);
  graphite_dim_t tdim2 = pbb_nb_scattering_dims (s2, pbb2, scop);
  graphite_dim_t ddim1 = pbb_nb_loops (pbb1);
  graphite_dim_t ddim2 = pbb_nb_loops (pbb2);
  graphite_dim_t sdim1 = pdr_accessp_nb_subscripts (pdr1) + 1;
  graphite_dim_t sdim2 = pdr_accessp_nb_subscripts (pdr2) + 1;
  graphite_dim_t gdim = scop_nb_params (scop);
  graphite_dim_t dim = tdim1 + ddim1 + tdim2 + ddim2 + sdim1 + sdim2 + gdim;
  ppl_Pointset_Powerset_NNC_Polyhedron_t res;
  ppl_Pointset_Powerset_NNC_Polyhedron_t id1, id2, isc1, isc2, idr1, idr2;
  ppl_Pointset_Powerset_NNC_Polyhedron_t sc1, sc2, dreq;
  
  gcc_assert (PBB_SCOP (pbb1) == PBB_SCOP (pbb2));
  ppl_new_Pointset_Powerset_NNC_Polyhedron_from_NNC_Polyhedron (&sc1, s1);
  ppl_new_Pointset_Powerset_NNC_Polyhedron_from_NNC_Polyhedron (&sc2, s2);

  id1 = map_into_dep_poly (dim, gdim, d1, ddim1, tdim1);
  id2 = map_into_dep_poly (dim, gdim, d2, ddim2, tdim1 + ddim1 + tdim2);
  isc1 = map_into_dep_poly (dim, gdim, sc1, ddim1 + tdim1, 0);
  isc2 = map_into_dep_poly (dim, gdim, sc2, ddim2 + tdim2, tdim1 + ddim1);

  idr1 = map_dr_into_dep_poly (dim, PDR_ACCESSES (pdr1), ddim1, ddim1 + gdim,
			       tdim1, tdim2 + ddim2);
  idr2 = map_dr_into_dep_poly (dim, PDR_ACCESSES (pdr2), ddim2, ddim2 + gdim,
			       tdim1 + ddim1 + tdim2, sdim1);

  /* Now add the subscript equalities.  */
  dreq = dr_equality_constraints (dim, tdim1 + ddim1 + tdim2 + ddim2, sdim1); 

  ppl_new_Pointset_Powerset_NNC_Polyhedron_from_space_dimension (&res, dim, 0);
  ppl_Pointset_Powerset_NNC_Polyhedron_intersection_assign (res, id1);
  ppl_Pointset_Powerset_NNC_Polyhedron_intersection_assign (res, id2);
  ppl_Pointset_Powerset_NNC_Polyhedron_intersection_assign (res, isc1);
  ppl_Pointset_Powerset_NNC_Polyhedron_intersection_assign (res, isc2);
  ppl_Pointset_Powerset_NNC_Polyhedron_intersection_assign (res, idr1);
  ppl_Pointset_Powerset_NNC_Polyhedron_intersection_assign (res, idr2);
  ppl_Pointset_Powerset_NNC_Polyhedron_intersection_assign (res, dreq);
  ppl_delete_Pointset_Powerset_NNC_Polyhedron (id1);
  ppl_delete_Pointset_Powerset_NNC_Polyhedron (id2);
  ppl_delete_Pointset_Powerset_NNC_Polyhedron (sc1);
  ppl_delete_Pointset_Powerset_NNC_Polyhedron (sc2);
  ppl_delete_Pointset_Powerset_NNC_Polyhedron (isc1);
  ppl_delete_Pointset_Powerset_NNC_Polyhedron (isc2);
  ppl_delete_Pointset_Powerset_NNC_Polyhedron (idr1);
  ppl_delete_Pointset_Powerset_NNC_Polyhedron (idr2);
  ppl_delete_Pointset_Powerset_NNC_Polyhedron (dreq);

  if (!ppl_Pointset_Powerset_NNC_Polyhedron_is_empty (res))
    build_lexicographically_gt_constraint (res, dim, MIN (tdim1, tdim2),
					   tdim1 + ddim1, direction);

  return res;
}

/* Returns true when the PBB_TRANSFORMED_SCATTERING functions of PBB1
   and PBB2 respect the data dependences of PBB_ORIGINAL_SCATTERING
   functions.  */

static bool
graphite_legal_transform_dr (poly_bb_p pbb1, poly_bb_p pbb2,
			     poly_dr_p pdr1, poly_dr_p pdr2)
{
  ppl_Pointset_Powerset_NNC_Polyhedron_t d1 = PBB_DOMAIN (pbb1);
  ppl_Pointset_Powerset_NNC_Polyhedron_t d2 = PBB_DOMAIN (pbb2);
  ppl_Polyhedron_t so1 = PBB_ORIGINAL_SCATTERING (pbb1);
  ppl_Polyhedron_t so2 = PBB_ORIGINAL_SCATTERING (pbb2);
  ppl_Pointset_Powerset_NNC_Polyhedron_t po;

  graphite_dim_t sdim1 = pdr_accessp_nb_subscripts (pdr1) + 1;
  graphite_dim_t sdim2 = pdr_accessp_nb_subscripts (pdr2) + 1;

  if (sdim1 != sdim2)
    return true;

  po = dependence_polyhedron (pbb1, pbb2, d1, d2, pdr1, pdr2, so1, so2, true);

  if (ppl_Pointset_Powerset_NNC_Polyhedron_is_empty (po))
    return true;
  else
    {
      ppl_Polyhedron_t st1 = PBB_TRANSFORMED_SCATTERING (pbb1);
      ppl_Polyhedron_t st2 = PBB_TRANSFORMED_SCATTERING (pbb2);
      ppl_Pointset_Powerset_NNC_Polyhedron_t pt;
      graphite_dim_t ddim1 = pbb_nb_loops (pbb1);
      scop_p scop = PBB_SCOP (pbb1);
      graphite_dim_t otdim1 = pbb_nb_scattering_dims (so1, pbb1, scop);
      graphite_dim_t otdim2 = pbb_nb_scattering_dims (so2, pbb2, scop);
      graphite_dim_t ttdim1 = pbb_nb_scattering_dims (st1, pbb1, scop);
      graphite_dim_t ttdim2 = pbb_nb_scattering_dims (st2, pbb2, scop);

      pt = dependence_polyhedron (pbb1, pbb2, d1, d2, pdr1, pdr2, st1, st2, false);
      
      /* Extend PO and PT to have the same dimensions.  */
      ppl_insert_dimensions_pointset (po, otdim1, ttdim1);
      ppl_insert_dimensions_pointset (po, otdim1 + ttdim1 + ddim1 + otdim2, ttdim2);
      ppl_insert_dimensions_pointset (pt, 0, otdim1);
      ppl_insert_dimensions_pointset (pt, otdim1 + ttdim1 + ddim1, otdim2);

      ppl_Pointset_Powerset_NNC_Polyhedron_intersection_assign (po, pt);
      return ppl_Pointset_Powerset_NNC_Polyhedron_is_empty (po);
    }
}

/* Iterates over the data references of PBB1 and PBB2 and detect
   whether the transformed schedule is correct.  */

static bool
graphite_legal_transform_bb (poly_bb_p pbb1, poly_bb_p pbb2)
{
  int i, j;
  poly_dr_p pdr1, pdr2;

  for (i = 0; VEC_iterate (poly_dr_p, PBB_DRS (pbb1), i, pdr1); i++)
    for (j = 0; VEC_iterate (poly_dr_p, PBB_DRS (pbb2), j, pdr2); j++)
      if (!graphite_legal_transform_dr (pbb1, pbb2, pdr1, pdr2))
        return false;
  return true;
}

/* Iterates over the SCOP and detect whether the transformed schedule
   is correct.  */

bool
graphite_legal_transform (scop_p scop)
{
  int i, j;
  poly_bb_p pbb1, pbb2;

  for (i = 0; VEC_iterate (poly_bb_p, SCOP_BBS (scop), i, pbb1); i++)
    for (j = 0; VEC_iterate (poly_bb_p, SCOP_BBS (scop), j, pbb2); j++)
      if (!graphite_legal_transform_bb (pbb1, pbb2))
	return false;

  return true;
}


#endif

