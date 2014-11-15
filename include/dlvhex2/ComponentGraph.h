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
 * @file   ComponentGraph.h
 * @author Peter Schueller <ps@kr.tuwien.ac.at>
 * 
 * @brief  Component Graph interface.
 */

#ifndef COMPONENT_GRAPH_HPP_INCLUDED__18102010
#define COMPONENT_GRAPH_HPP_INCLUDED__18102010

#include "dlvhex2/DependencyGraph.h"

#include <boost/concept/assert.hpp>
#include <boost/concept_check.hpp>
#include "boost/tuple/tuple.hpp"

#ifndef NDEBUG
# define COMPGRAPH_SOURCESDEBUG
#endif

DLVHEX_NAMESPACE_BEGIN

/**
 * A component graph is created from a dependency graph by collecting SCCs
 * into single nodes components.
 *
 * A component graph is a dag (acyclic by the above construction).
 *
 * Vertices (= components) store a set of rules and information about the dependencies
 * within the collapsed part of the dependency graph.
 *
 * Edges (= collapsed dependencies) store information about the collapsed
 * dependencies.
 *
 * A component contains
 * - external atoms depending only on other components (=outer eatoms),
 * - rules within the component (=inner rules),
 * - constraints within the component (=inner constraints), and
 * - external atoms depending on rules in the component (=inner eatoms).
 *
 * For each component, only one of these storages must hold an object, except for
 * inner eatoms which can only exist if there are inner rules.
 */
class DLVHEX_EXPORT ComponentGraph
{
  BOOST_CONCEPT_ASSERT((boost::Convertible<DependencyGraph::Node, unsigned int>));
  //////////////////////////////////////////////////////////////////////////////
  // types
  //////////////////////////////////////////////////////////////////////////////
public:
  struct DLVHEX_EXPORT ComponentInfo:
    public ostream_printable<ComponentInfo>
  {
    #ifdef COMPGRAPH_SOURCESDEBUG
    std::list<DependencyGraph::Node> sources;
    #endif

    std::vector<ID> outerEatoms;

    std::vector<ID> innerRules;
    std::vector<ID> innerEatoms;
    std::vector<ID> innerConstraints;
    boost::unordered_map<ID, std::set<ID> > stronglySafeVariables;
    boost::unordered_map<ID, std::set<ID> > stratifiedLiterals;
    std::set<ID> predicatesInComponent;

    // this is determined by calculateComponents
    // and used for selecting model generator factories
		bool disjunctiveHeads;
		bool negativeDependencyBetweenRules;
		bool innerEatomsNonmonotonic;
		bool outerEatomsNonmonotonic;
		bool componentIsMonotonic;
		bool fixedDomain;
		bool recursiveAggregates;

		// previous:
    //bool innerEatomsMonotonicAndOnlyPositiveCycles;
		// := (!innerEatomsNonmonotonic && !negativeDependencyBetweenRules && !disjunctiveHeads)

		ComponentInfo():
      disjunctiveHeads(false),
			negativeDependencyBetweenRules(false),
			innerEatomsNonmonotonic(false),
	outerEatomsNonmonotonic(false),
	componentIsMonotonic(true),
	fixedDomain(true),
	recursiveAggregates(false){}
    std::ostream& print(std::ostream& o) const;
  };

  struct DependencyInfo:
		public DependencyGraph::DependencyInfo,
    public ostream_printable<DependencyInfo>
  {
    #ifdef COMPGRAPH_SOURCESDEBUG
    std::set<DependencyGraph::Dependency> sources;
    #endif

    typedef boost::tuple<ID, ID, DependencyGraph::DependencyInfo> DepEdge;
    std::vector<DepEdge> depEdges;

		DependencyInfo() {}
		DependencyInfo(const DependencyGraph::DependencyInfo& other):
      DependencyGraph::DependencyInfo(other) {}
		const DependencyInfo& operator|=(const DependencyInfo& other);
    std::ostream& print(std::ostream& o) const;
  };

  // we need listS because this graph will be changed a lot by collapsing nodes
  // use setS for out-edges because we don't want to have multiple edges
  typedef boost::adjacency_list<
    boost::setS, boost::listS, boost::bidirectionalS,
    ComponentInfo, DependencyInfo> Graph;
  typedef boost::graph_traits<Graph> Traits;

  typedef Graph::vertex_descriptor Component;
  typedef Graph::edge_descriptor Dependency;
  typedef Traits::vertex_iterator ComponentIterator;
  typedef Traits::edge_iterator DependencyIterator;
  typedef Traits::out_edge_iterator PredecessorIterator;
  typedef Traits::in_edge_iterator SuccessorIterator;

	typedef std::set<Component> ComponentSet;
	typedef std::map<Component, DependencyInfo> DepMap;

  //////////////////////////////////////////////////////////////////////////////
  // members
  //////////////////////////////////////////////////////////////////////////////
protected:
  // for debugging and printing
  RegistryPtr reg;
  #ifdef COMPGRAPH_SOURCESDEBUG
  // in non-debug mode this graph's lifetime can end
  // after the constructor finished
  const DependencyGraph& dg;
  #endif
  Graph cg;

  //////////////////////////////////////////////////////////////////////////////
  // methods
  //////////////////////////////////////////////////////////////////////////////
protected:
	// only to be used by explicit clone method
	ComponentGraph(const ComponentGraph& other);
public:
	ComponentGraph(const DependencyGraph& dg, RegistryPtr reg);
	virtual ~ComponentGraph();

	// for explicit cloning of the graph
	ComponentGraph* clone() const;

	//
	// modifiers
	//

	// collapse several components into one
  // originals are put into new component and then removed
  // shared are just copied into new component
	Component collapseComponents(
			const ComponentSet& originals, const ComponentSet& shared=ComponentSet());

  //
  // mighty helper for collapsing components
  //

  // Compute the dependency infos and component info
  // before putting components `comps' and `sharedcomps' into a new component.
  //
  // `sharedcomps' may only contain components with constraints that can be shared.
  //
  // This method does not change the graph, it only changes the output arguments,
  // hence it is const (and should stay const).
  // 
  // This method throws an exception if this operation makes the DAG cyclic.
  void computeCollapsedComponentInfos(
      const ComponentSet& comps, const ComponentSet& sharedcomps,
      DepMap& newIncomingDependencies, DepMap& newOutgoingDependencies,
      ComponentInfo& newComponentInfo) const; // see comment above about const!

	//
	// accessors
	//

	// get const graph to apply external algorithms
	inline const Graph& getInternalGraph()
		const { return cg; }

  // output graph as graphviz source
  virtual void writeGraphViz(std::ostream& o, bool verbose) const;

  // get range over all components
  inline std::pair<ComponentIterator, ComponentIterator> getComponents() const
    { return boost::vertices(cg); }
  inline std::pair<DependencyIterator, DependencyIterator> getDependencies() const
    { return boost::edges(cg); }

	// get node info given node
	inline const ComponentInfo& getComponentInfo(Component c) const
		{ return cg[c]; }

	// get dependency info given dependency
	inline const DependencyInfo& getDependencyInfo(Dependency dep) const
		{ return cg[dep]; }

	// get dependencies (to predecessors) = arcs from this component to others
  inline std::pair<PredecessorIterator, PredecessorIterator>
  getDependencies(Component c) const
		{ return boost::out_edges(c, cg); }

	// get provides (dependencies to successors) = arcs from other component to this one
  inline std::pair<SuccessorIterator, SuccessorIterator>
  getProvides(Component c) const
		{ return boost::in_edges(c, cg); }

	// get source of dependency = component that depends
  inline Component sourceOf(Dependency d) const
		{ return boost::source(d, cg); }

	// get target of dependency = component upon which the source depends
  inline Component targetOf(Dependency d) const
		{ return boost::target(d, cg); }

	// get node/dependency properties
	inline const ComponentInfo& propsOf(Component c) const
		{ return cg[c]; }
	inline ComponentInfo& propsOf(Component c)
		{ return cg[c]; }
	inline const DependencyInfo& propsOf(Dependency d) const
		{ return cg[d]; }
	inline DependencyInfo& propsOf(Dependency d)
		{ return cg[d]; }

	// counting -> mainly for allocating and testing
  inline unsigned countComponents() const
		{ return boost::num_vertices(cg); }
  inline unsigned countDependencies() const
		{ return boost::num_edges(cg); }

	//
	// helpers
	//
protected:
  // helpers for writeGraphViz: extend for more output
  virtual void writeGraphVizComponentLabel(std::ostream& o, Component c, unsigned index, bool verbose) const;
  virtual void writeGraphVizDependencyLabel(std::ostream& o, Dependency dep, bool verbose) const;

protected:
  // helpers for constructor
  void calculateComponents(const DependencyGraph& dg);

  bool calculateFixedDomain(ComponentInfo& ci);
  bool computeRecursiveAggregatesInComponent(ComponentInfo& ci);

public:
  static void calculateStratificationInfo(RegistryPtr reg, ComponentInfo& ci);
};

DLVHEX_NAMESPACE_END

#endif // DEPENDENCY_GRAPH_HPP_INCLUDED__18102010
