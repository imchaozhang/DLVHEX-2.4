/* dlvhex -- Answer-Set Programming with external interfaces.
 * Copyright (C) 2005, 2006, 2007 Roman Schindlauer
 * Copyright (C) 2006, 2007, 2008, 2009, 2010 Thomas Krennwallner
 * Copyright (C) 2009, 2010 Peter Schüller
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
 * @file   AttributeGraph.h
 * @author Christoph Redl <redl@kr.tuwien.ac.at>
 * 
 * @brief  Stores dependencies between attributes in a program and
 *         checks domain-expansion safety of a program.
 */

#ifndef ATTRIBUTEGRAPH_H_
#define ATTRIBUTEGRAPH_H_

#include "dlvhex2/PlatformDefinitions.h"
#include "dlvhex2/fwd.h"
#include "dlvhex2/Logger.h"
#include "dlvhex2/ID.h"

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/intrusive/unordered_set.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>

DLVHEX_NAMESPACE_BEGIN

class AttributeGraph{
public:
	struct Attribute : private ostream_printable<Attribute>{
		enum Type{
			Ordinary, External
		};
		RegistryPtr reg;
		Type type;
		ID eatomID;
		ID predicate;
		std::vector<ID> inputList;
		ID ruleID;
		bool input;
		int argIndex;

		bool operator==(const Attribute& at2) const;
		bool operator<(const Attribute& at2) const;
		std::ostream& print(std::ostream& o) const;
	};

	typedef boost::adjacency_list<boost::setS, boost::vecS, boost::bidirectionalS, Attribute > Graph;
	typedef boost::graph_traits<Graph> Traits;

	typedef Graph::vertex_descriptor Node;
	typedef Graph::edge_descriptor Dependency;
	typedef Traits::vertex_iterator NodeIterator;
	typedef Traits::edge_iterator DependencyIterator;
	typedef Traits::out_edge_iterator PredecessorIterator;
	typedef Traits::in_edge_iterator SuccessorIterator;

	typedef std::pair<ID, ID> VariableLocation;	// stores rule ID and variable ID
	typedef std::pair<ID, ID> AtomLocation;		// stores rule ID and atom ID
private:
	RegistryPtr reg;
	const std::vector<ID>& idb;

	// attribute graph
	Graph ag;
	boost::unordered_map<ID, std::vector<Attribute> > attributesOfPredicate;
	struct NodeInfoTag {};
	struct NodeMappingInfo
	{
		Attribute at;
		Node node;
		NodeMappingInfo(Attribute at, Node node): at(at), node(node) {}
	};
	typedef boost::multi_index_container<
			NodeMappingInfo,
			boost::multi_index::indexed_by<
				boost::multi_index::hashed_unique<
					boost::multi_index::tag<NodeInfoTag>,
					BOOST_MULTI_INDEX_MEMBER(NodeMappingInfo,Attribute,at)
				>
			>
		> NodeMapping;
	NodeMapping nm;
	typedef NodeMapping::index<NodeInfoTag>::type NodeNodeInfoIndex;
	std::vector<std::vector<Attribute> > depSCC;

	// some indices
	typedef std::pair<std::set<VariableLocation>, boost::unordered_set<Attribute> > SafetyPreconditions;	// stores which variables still need to be bounded
														// and which attributes need to become safe
														// in order to make another attribute safe
	boost::unordered_map<Attribute, SafetyPreconditions> safetyPreconditions;				// stores for each attributes the preconditios for becoming safe

	boost::unordered_map<VariableLocation, boost::unordered_set<Attribute> > attributesSafeByVariable;	// stores for each variable the attributes whose safety depends on this variable
	boost::unordered_map<Attribute, boost::unordered_set<Attribute> > attributesSafeByAttribute;		// stores for each attribute the attributes whose safety depends on this attribute

	boost::unordered_map<Attribute, std::set<AtomLocation> > attributeOccursIn;				// stores for each attribute the atoms where it occurs
	boost::unordered_map<VariableLocation, std::set<AtomLocation> > variableOccursIn;			// stores for each variable the atoms where it occurs

	// output
	boost::unordered_map<ID, int> predicateArity;								// arity of a given (ordinary) predciate
	std::set<Node> cyclicAttributes;
	boost::unordered_set<VariableLocation> boundedVariables;						// currently bounded variables
	boost::unordered_set<Attribute> domainExpansionSafeAttributes;						// current domain-expansion safe attributes
	boost::unordered_set<ID> necessaryExternalAtoms;							// external atoms which are necessary to establish domain-expansion safety
	boost::unordered_set<std::pair<ID, VariableLocation> > boundedByExternals;				// variables bounded by externals, but not (yet) by ordinary atoms

	Attribute getAttribute(ID eatomID, ID predicate, std::vector<ID> inputList, ID ruleID, bool inputAttribute, int argumentIndex);
	Attribute getAttribute(ID predicate, int argumentIndex);
	Node getNode(Attribute at);

	// helper
	bool hasInformationFlow(boost::unordered_map<ID, boost::unordered_set<ID> >& builtinflow, ID from, ID to);
	bool isNewlySafe(Attribute at);

	// trigger functions
	void addBoundedVariable(VariableLocation vl);								// called after a new variable has become bounded to trigger further actions
	void addDomainExpansionSafeAttribute(Attribute at);							// called after an attribute has become safe to trigger further actions
	bool identifyBenignCycles();										// makes output attributes in benign cycles safe; returns if new attributes became safe

	// initialization
	void computeBuiltinInformationFlow(const Rule& rule, boost::unordered_map<ID, boost::unordered_set<ID> >& builtinflow);	// computes for a given rule the
																// information exchange between variables through builtins

	void createDependencyGraph();												// create dependency graph of ordinary and external predicates
	void createPreconditionsAndLocationIndices();										// the indices above
	void computeCyclicAttributes();												// compute attributes which occur in or depend on cycles

	// computation
	void ensureOrdinarySafety();												// restricts the optimization of necessary to keep ordinary safety

	void checkStaticConditions();												// statically checks for domain-expansion safety of attributes
																// and boundedness of variables, i.e., the checks are done only once
	bool checkDynamicConditions();												// iteratively checks for more domain-expansion safety of attributes
																// and boundedness of variables; returns if changes were made
	void computeDomainExpansionSafety();											// calls the previous methods until no more safe attributes can be derived
public:
	AttributeGraph(RegistryPtr reg, const std::vector<ID>& idb);

	bool isDomainExpansionSafe() const;
	bool isExternalAtomNecessaryForDomainExpansionSafety(ID eatomID) const;

	// output graph as graphviz source
	virtual void writeGraphViz(std::ostream& o, bool verbose) const;
};

std::size_t hash_value(const AttributeGraph::Attribute& at);
std::size_t hash_value(const AttributeGraph::VariableLocation& vl);

typedef boost::shared_ptr<AttributeGraph> AttributeGraphPtr;
typedef boost::shared_ptr<const AttributeGraph> AttributeGraphConstPtr;

DLVHEX_NAMESPACE_END

#endif
