/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/* $Id$ */
/**
 * @file CxxUtils/unused.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2011
 * @brief Macro to mark a variable as unused.
 *
 * Use like:
 *
 *   int ATH_UNUSED(v);
 *
 * Of course, if a variable is really unused, it's better to simply
 * remove it rather than trying to paper over the warning like this.
 *
 * But there are some cases where in regression tests one might want
 * to legitimately have an unused variable (particularly with gcc 4.6,
 * where the unused warnings also catch variables assigned to but
 * not read).  This is really meant for these cases.
 */

#ifndef CXXUTILS_UNUSED_H
#define CXXUTILS_UNUSED_H


// For now, declare both UNUSED and ATH_UNUSED.  ATH_UNUSED is preferred for new code;
// UNUSED has been found to conflict with some externals.
#ifdef __GNUC__
# define UNUSED(v) v __attribute__((unused))
# define ATH_UNUSED(v) v __attribute__((unused))
#else
# define UNUSED(v) v
# define ATH_UNUSED(v) v
#endif


// Declare that a member is unused, to suppress warnings.
// Only clang recognizes an unused attribute for a member.
#ifdef __clang__
# define ATH_UNUSED_MEMBER(v) v __attribute__((unused))
#else
# define ATH_UNUSED_MEMBER(v) v
#endif


#endif /* not CXXUTILS_UNUSED_H */
