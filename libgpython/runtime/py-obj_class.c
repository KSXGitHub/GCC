/* This file is part of GCC.

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <gmp.h>
#include <mpfr.h>

#include <gpython/gpython.h>
#include <gpython/vectors.h>
#include <gpython/objects.h>
#include <gpython/runtime.h>

typedef void (*__field_init_ptr)(void *);

static
void gpy_object_classobj_init_decl_attribs (const void * self,
					    gpy_object_attrib_t ** attribs)
{
  int idx;
  unsigned char * selfptr = (unsigned char *)self;
  for (idx = 0; attribs[idx] != NULL; ++idx)
    {
      gpy_object_attrib_t * i = attribs[idx];
      unsigned char * typeoffs = selfptr + (i->offset);

      gpy_object_t ** offs = (gpy_object_t **) typeoffs;
      if (i->addr)
	{
	  gpy_object_t * attrib = (gpy_object_t *)i->addr;
	  *offs = i->addr;
	}
      else
	*offs = NULL;
    }
}

static
void gpy_object_classobj_methodattribs_addself (gpy_object_attrib_t ** attribs,
						gpy_object_t * self)
{
  int idx;
  for (idx = 0; attribs[idx] != NULL; ++idx)
    {
      gpy_object_attrib_t * i = attribs[idx];
      if (i->addr)
	{
	  gpy_object_t * att = (gpy_object_t *) i->addr;
	  gpy_object_classmethod_inherit_self (att, self);
	}
    }
}

/* Class's internal type is generated by the compiler ...*/
gpy_object_t * gpy_object_classobj_new (gpy_typedef_t * type,
					gpy_object_t ** args)
{
  gpy_object_t * retval = NULL_OBJECT;

  bool check = gpy_args_check_fmt (args, "A,i,s.");
  gpy_assert (check);

  gpy_object_attrib_t ** attribs = gpy_args_lit_parse_attrib_table (args[0]);
  int size = gpy_args_lit_parse_int (args[1]);
  char * ident = gpy_args_lit_parse_string (args[2]);

  void * self = gpy_malloc (size);
  gpy_object_classobj_init_decl_attribs (self, attribs);

  gpy_typedef_t * ctype = gpy_malloc (sizeof (gpy_typedef_t));
  ctype->identifier = ident;
  ctype->state_size = size;
  ctype->tp_new = type->tp_new;
  ctype->tp_dealloc = type->tp_dealloc;
  ctype->tp_print = type->tp_print;
  ctype->tp_call = type->tp_call;
  ctype->binary_protocol = type->binary_protocol;
  ctype->members_defintion = attribs;

  retval = gpy_create_object_decl (ctype, self);

  /* we need to walk though the field_init here */
  unsigned char * __field_init__ = gpy_rr_eval_attrib_reference (retval, "__field_init__");
  gpy_object_t * field_init = *((gpy_object_t **) __field_init__);
  unsigned char * codeaddr = gpy_object_classmethod_getaddr (field_init);
  gpy_assert (codeaddr);

  __field_init_ptr c = (__field_init_ptr)codeaddr;
  c (self);

  return retval;
}

void gpy_object_classobj_dealloc (gpy_object_t * self)
{
  gpy_assert (self->T == TYPE_OBJECT_DECL);
  gpy_object_state_t * object_state = self->o.object_state;

  gpy_free (object_state->state);
  object_state->state = NULL;
}

void gpy_object_classobj_print (gpy_object_t * self, FILE *fd, bool newline)
{
  switch (self->T)
    {
    case TYPE_OBJECT_STATE:
      fprintf (fd, "class object instance <%p> ", (void *)self);
      break;

    case TYPE_OBJECT_DECL:
      fprintf (fd, "class object decl <%p> ", (void *)self);
      break;

    default:
      fatal ("something went very wrong here!\n");
      break;
    }
  if (newline)
    fprintf (fd, "\n");
}

static
gpy_object_t ** gpy_object_classobj_setupargs (gpy_object_t ** args,
					       gpy_object_t * self)
{
  int idx = 0;
  gpy_object_t ** ptr;
  for (ptr = args; *ptr != NULL; ++ptr)
    idx ++;

  gpy_object_t ** newargs = calloc (idx+2, sizeof (gpy_object_t *));
  *newargs = self;
  gpy_object_t ** newargsptr = newargs;
  newargsptr++;

  for (ptr = args; *ptr != NULL; ++ptr)
    {
      *newargsptr = *ptr;
      newargsptr++;
    }
  *newargsptr = NULL;

  return newargs;
}

gpy_object_t * gpy_object_classobj_call (gpy_object_t * self,
					 gpy_object_t ** args)
{
  gpy_object_t * retval = NULL_OBJECT;
  gpy_assert (self->T == TYPE_OBJECT_DECL);

  gpy_typedef_t * type = self->o.object_state->definition;
  void * oldstate = self->o.object_state->state;

  void * newstate = malloc (type->state_size);
  memcpy (newstate, oldstate, type->state_size);
  retval = gpy_create_object_state (type, newstate);
  gpy_assert (retval);

  unsigned char * __init__ = gpy_rr_eval_attrib_reference (retval, "__init__");
  gpy_object_t * init = *((gpy_object_t **) __init__);

  gpy_assert (init->T == TYPE_OBJECT_DECL);
  gpy_typedef_t * calltype = init->o.object_state->definition;
  if (type->tp_call)
    {
      gpy_object_t ** arguments = gpy_object_classobj_setupargs (args, retval);
      calltype->tp_call (init, arguments);
    }
  else
    fatal ("name is not callable!\n");

  return retval;
}

static struct gpy_typedef_t class_obj = {
  "classobj",
  0,
  &gpy_object_classobj_new,
  &gpy_object_classobj_dealloc,
  &gpy_object_classobj_print,
  &gpy_object_classobj_call,
  NULL,
  NULL
};

void gpy_obj_class_mod_init (gpy_vector_t * const vec)
{
  gpy_vec_push (vec, &class_obj);
}
