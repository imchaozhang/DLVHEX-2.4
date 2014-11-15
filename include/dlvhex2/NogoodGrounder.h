/* dlvhex -- Answer-Set Programming with external interfaces.
 * Copyright (C) 2005, 2006, 2007 Roman Schindlauer
 * Copyright (C) 2006, 2007, 2008, 2009, 2010 Thomas Krennwallner
 * Copyright (C) 2009, 2010 Peter Schüller
 * Copyright (C) 2011, 2012, 2013 Christoph Redl
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
 * @file   NogoodGrounder.hpp
 * @author Christoph Redl <redl@kr.tuwien.ac.at>
 * 
 * @brief  Implements a grounder for nonground nogoods
 */

#ifndef NOGOODGROUNDER_HPP_
#define NOGOODGROUNDER_HPP_

#include "dlvhex2/PlatformDefinitions.h"
#include "dlvhex2/fwd.h"
#include "dlvhex2/ID.h"
#include "dlvhex2/Nogood.h"
#include "dlvhex2/Registry.h"
#include "dlvhex2/AnnotatedGroundProgram.h"

#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

DLVHEX_NAMESPACE_BEGIN

class DLVHEX_EXPORT NogoodGrounder{
protected:
	RegistryPtr reg;
	SimpleNogoodContainerPtr watched;
	SimpleNogoodContainerPtr destination;
	AnnotatedGroundProgram& agp;
public:
	/**
	 * Initializes the nogood grounder for a container of watched nogoods
	 * and a destination for resulting ground nogoods.
	 * @param reg RegistryPtr
	 * @param watched A container with the nogoods to ground
	 * @param destination The container where the resulting nogoods shall be added (possibly the same as watched)
	 * @param agp The ground program for which the nogoods shall be learned
	 */
	NogoodGrounder(RegistryPtr reg, SimpleNogoodContainerPtr watched, SimpleNogoodContainerPtr destination, AnnotatedGroundProgram& agp);

	/**
	 * Makes another grounding step
	 * @param partialInterpretation The current (partial) assignment
	 * @param factWasSet The atoms which have been assigned so far
	 * @param changed The changes in partialInterpretation since the last call (possibly helps the grounder to find relevant ground instances)
	 */
	virtual void update(InterpretationConstPtr partialInterpretation = InterpretationConstPtr(), InterpretationConstPtr factWasSet = InterpretationConstPtr(), InterpretationConstPtr changed = InterpretationConstPtr()) = 0;

	/**
	 * Changes the watched nogood container
	 * @param watched Pointer to the new watched nogood container
	 */
	virtual void resetWatched(SimpleNogoodContainerPtr watched);

	typedef boost::shared_ptr<NogoodGrounder> Ptr;
};

typedef NogoodGrounder::Ptr NogoodGrounderPtr;

/**
 * Instantiates nonground nogoods immediately for all possible substitutions.
 */
class DLVHEX_EXPORT ImmediateNogoodGrounder : public NogoodGrounder{
private:
	int instantiatedNongroundNogoodsIndex;
public:
	ImmediateNogoodGrounder(RegistryPtr reg, SimpleNogoodContainerPtr watched, SimpleNogoodContainerPtr destination, AnnotatedGroundProgram& agp);
	void update(InterpretationConstPtr partialInterpretation = InterpretationConstPtr(), InterpretationConstPtr factWasSet = InterpretationConstPtr(), InterpretationConstPtr changed = InterpretationConstPtr());
	void resetWatched(SimpleNogoodContainerPtr watched);
};

/**
 * Instantiates nonground nogoods stepwise according to the current interpretation.
 * That is, a nogood is instantiated if one of its atoms unifies with the current partial interpretation.
 */
class DLVHEX_EXPORT LazyNogoodGrounder : public NogoodGrounder{
private:
	int watchedNogoodsCount;
	std::vector<std::pair<ID, int> > watchedLiterals;
	std::set<std::pair<IDAddress, int> > alreadyCompared;	// store which atom was already compared to which nonground nogood
public:
	LazyNogoodGrounder(RegistryPtr reg, SimpleNogoodContainerPtr watched, SimpleNogoodContainerPtr destination, AnnotatedGroundProgram& agp);
	void update(InterpretationConstPtr partialInterpretation = InterpretationConstPtr(), InterpretationConstPtr factWasSet = InterpretationConstPtr(), InterpretationConstPtr changed = InterpretationConstPtr());
	void resetWatched(SimpleNogoodContainerPtr watched);
};

DLVHEX_NAMESPACE_END

#endif

