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
 * @file   ExtSourceProperties.hpp
 * @author Christoph Redl <redl@kr.tuwien.ac.at>
 * 
 * @brief  Definition of properties of external sources
 */

#ifndef EXTSOURCEPROPERTIES_HPP_
#define EXTSOURCEPROPERTIES_HPP_

#include "dlvhex2/PlatformDefinitions.h"
#include "dlvhex2/Logger.h"
#include "dlvhex2/ID.h"
#include "dlvhex2/fwd.h"
#include "dlvhex2/Printhelpers.h"

#include <vector>
#include <string>

DLVHEX_NAMESPACE_BEGIN

WARNING("TODO what is the difference/intended usage of ExtSourceProperty vs ExtSourceProperties? (the names are not very intuitive)")

/**
 This struct is used to store sets of properties of an external atom.
 E.g. in
   &foo[n,m](X,Y)<monotonic n, antimonotonic m>
 we need to store two properties
 - one for "monotonic n"
 - one for "antimonotonic m"

 Currently the following properties are supported:
		MONOTONIC
		ANTIMONOTONIC
		FUNCTIONAL
		ATOMLEVELLINEAR
		TUPLELEVELLINEAR
		USES_ENVIRONMENT
		RELATIVEFINITEDOMAIN
		FINITEDOMAIN
		FINITEFIBER
		WELLORDERINGSTRLEN
		WELLORDERINGNATURAL
		SUPPORTSETS
		COMPLETEPOSITIVESUPPORTSETS
		COMPLETENEGATIVESUPPORTSETS
		VARIABLEOUTPUTARITY
		CARESABOUTASSIGNED
		CARESABOUTCHANGED
*/

// Stores properties of an external source on one of two levels:
// 1. on the level of plugin atoms
// 2. on the level of individual external atoms
struct ExtSourceProperties
{
	// exactly one of the following pointers will be NULL
	ExternalAtom* ea;	// pointer to the external atom to which this property structure belongs to
	PluginAtom* pa;		// pointer to the plugin atom to which this property structure belongs to;

	// all indices are 0-based
	std::set<int> monotonicInputPredicates;			// indices of monotonic input parameters
	std::set<int> antimonotonicInputPredicates;		// indices of antimonotonic input parameters
	std::set<int> predicateParameterNameIndependence;	// indices of input parameters whose name is irrelevant (only the extension matters)
	std::set<int> finiteOutputDomain;			// indices of output elements with a finite domain
	std::set<std::pair<int, int> > relativeFiniteOutputDomain;			// indices of output elements with a finite domain wrt. some input parameter

	// if an external source is functional, then there must not exist multiple output tuples simultanously;
	// "functionalStart" defines the number of non-functional output terms before the functional output starts
	// That is: Suppose a source has a ternery output, such that the third element is unique for each pair of elements in the first and second position;
	//          Then functionalStart=2 and the source may generate e.g. (a,b,c), (b,b,d), (b,a,d) but not (a,b,c), (a,b,d)
	bool functional;
	int functionalStart;
	bool supportSets;
	bool completePositiveSupportSets;
	bool completeNegativeSupportSets;
	bool variableOutputArity;
	bool caresAboutAssigned;
	bool caresAboutChanged;

	bool atomlevellinear;		// predicate input can be split into single atoms
	bool tuplelevellinear;		// predicate input can be split such that only atoms with the same arguments must be grouped
	bool usesEnvironment;		// external atom uses the environment (cf. acthex)
	bool finiteFiber;		// a fixed output value can be produced only by finitly many different inputs
	std::set<std::pair<int, int> > wellorderingStrlen;	// <i,j> means that output value at position j is strictly smaller than at input position i (strlen)
	std::set<std::pair<int, int> > wellorderingNatural;	// <i,j> means that output value at position j is strictly smaller than at input position i (wrt. natural numbers)

	ExtSourceProperties() : ea(0), pa(0), functionalStart(0){
		functional = false;
		atomlevellinear = false;
		tuplelevellinear = false;
		usesEnvironment = false;
		finiteFiber = false;
		supportSets = false;
		completePositiveSupportSets = false;
		completeNegativeSupportSets = false;
		variableOutputArity = false;
		caresAboutAssigned = false;
		caresAboutChanged = false;
	}

	ExtSourceProperties& operator|=(const ExtSourceProperties& prop2);

	// setter
	inline void addMonotonicInputPredicate(int index) { monotonicInputPredicates.insert(index); }
	inline void addAntimonotonicInputPredicate(int index) { antimonotonicInputPredicates.insert(index); }
	inline void addPredicateParameterNameIndependence(int index) { predicateParameterNameIndependence.insert(index); }
	inline void addFiniteOutputDomain(int index) { finiteOutputDomain.insert(index); }
	inline void addRelativeFiniteOutputDomain(int index1, int index2) { relativeFiniteOutputDomain.insert(std::pair<int, int>(index1, index2)); }
	inline void setFunctional(bool value) { functional = value; }
	inline void setFunctionalStart(int value) { functionalStart = value; }
	inline void setSupportSets(bool value) { supportSets = value; }
	inline void setCompletePositiveSupportSets(bool value) { completePositiveSupportSets = value; }
	inline void setCompleteNegativeSupportSets(bool value) { completeNegativeSupportSets = value; }
	inline void setVariableOutputArity(bool value) { variableOutputArity = value; }
	inline void setCaresAboutAssigned(bool value) { caresAboutAssigned = value; }
	inline void setCaresAboutChanged(bool value) { caresAboutChanged = value; }
	inline void setAtomlevellinear(bool value) { atomlevellinear = value; }
	inline void setTuplelevellinear(bool value) { tuplelevellinear = value; }
	inline void setUsesEnvironment(bool value) { usesEnvironment = value; }
	inline void setFiniteFiber(bool value) { finiteFiber = value; }
	inline void addWellorderingStrlen(int index1, int index2) { wellorderingStrlen.insert(std::pair<int, int>(index1, index2)); }
	inline void addWellorderingNatural(int index1, int index2) { wellorderingNatural.insert(std::pair<int, int>(index1, index2)); }

	/**
	* @return overall monotonicity
	*/
	bool isMonotonic() const;

	/**
	* @return overall antimonotonicity
	*/
	bool isAntimonotonic() const;

	/**
	* @return monotonicity on parameter level
	*/
	bool isMonotonic(int parameterIndex) const
	{ return monotonicInputPredicates.count(parameterIndex) > 0; }

	/**
	* @return antimonotonicity on parameter level
	*/
	bool isAntimonotonic(int parameterIndex) const
	{ return antimonotonicInputPredicates.count(parameterIndex) > 0; }

	/**
	* @return nonmonotonicity on parameter level
	*/
	bool isNonmonotonic(int parameterIndex) const
	{ return !isMonotonic(parameterIndex) && !isAntimonotonic(parameterIndex); }

	/**
	* @return functional
	*/
	bool isFunctional() const
	{ return functional; }

	/**
	* @return linearity on atom level
	*/
	bool isLinearOnAtomLevel() const
	{ return atomlevellinear; }

	/**
	* @return linearity on tuple level
	*/
	bool isLinearOnTupleLevel() const
	{ return tuplelevellinear; }

	/**
	* @return bool True if the name of the predicate parameter with the given index is irrelevant
	*/
	bool isIndependentOfPredicateParameterName(int parameterIndex) const
	{ return predicateParameterNameIndependence.count(parameterIndex) > 0; }

	/**
	* @return true if this Atom uses Environment
	*/
	bool doesItUseEnvironment() const
	{ return usesEnvironment; }
    
	/**
	* @return bool True if the specified output element has a finite domain
	*/
	bool hasFiniteDomain(int outputElement) const
	{ return finiteOutputDomain.count(outputElement) > 0; }
    
	/**
	* @return bool True if the specified output element has a finite domain wrt. to the given input element
	*/
	bool hasRelativeFiniteDomain(int outputElement, int inputElement) const
	{ return relativeFiniteOutputDomain.count(std::pair<int, int>(outputElement, inputElement)) > 0; }

	/**
	* @return finite fiber
	*/
	bool hasFiniteFiber() const
	{ return finiteFiber; }

	/**
	* @return strlen well ordering
	*/
	bool hasWellorderingStrlen(int from, int to) const
	{ return wellorderingStrlen.count(std::pair<int, int>(from, to)) > 0; }

	/**
	* @return natural well ordering
	*/
	bool hasWellorderingNatural(int from, int to) const
	{ return wellorderingNatural.count(std::pair<int, int>(from, to)) > 0; }

	/**
	* @return supportSets
	*/
	bool providesSupportSets() const
	{ return supportSets; }

	/**
	* @return completePositiveSupportSets
	*/
	bool providesCompletePositiveSupportSets() const
	{ return completePositiveSupportSets; }

	/**
	* @return completeNegativeSupportSets
	*/
	bool providesCompleteNegativeSupportSets() const
	{ return completeNegativeSupportSets; }

	/**
	* @return variableOutputArity
	*/
	bool hasVariableOutputArity() const
	{ return variableOutputArity; }

	/**
	* @return caresAboutAssigned
	*/
	bool doesCareAboutAssigned() const
	{ return caresAboutAssigned; }

	/**
	* @return caresAboutChanged
	*/
	bool doesCareAboutChanged() const
	{ return caresAboutChanged; }

	/**
	* Parses external source properties given as vectors of terms and integrates them into the current instance of the class
	*/
	void interpretProperties(RegistryPtr reg, const ExternalAtom& atom, const std::vector<std::vector<std::string> >& props);
};

DLVHEX_NAMESPACE_END

#endif

