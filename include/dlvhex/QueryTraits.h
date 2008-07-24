/* dlvhex -- Answer-Set Programming with external interfaces.
 * Copyright (C) 2005, 2006, 2007 Roman Schindlauer
 * 
 * This file is part of dlvhex.
 *
 * dlvhex is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * dlvhex is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with dlvhex; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


/**
 * @file   NegationTraits.h
 * @author Thomas Krennwallner
 * @date   Mon July 14 09:38:18 2008
 * 
 * @brief  Traits for Query.
 * 
 * 
 */


#if !defined(_DLVHEX_QUERYTRAITS_H)
#define _DLVHEX_QUERYTRAITS_H

#include "dlvhex/PlatformDefinitions.h"

DLVHEX_NAMESPACE_BEGIN

/**
 * The base class for the query traits.
 */
struct QueryTraits {};

/**
 * The brave reasoning trait.
 */
struct Brave : public QueryTraits {};

/**
 * The cautious reasoning trait.
 */
struct Cautious : public QueryTraits {};

DLVHEX_NAMESPACE_END

#endif /* _DLVHEX_QUERYTRAITS_H */


// Local Variables:
// mode: C++
// End: