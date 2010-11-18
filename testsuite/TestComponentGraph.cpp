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
 * @file   TestComponentGraph.cpp
 * @author Peter Schueller <ps@kr.tuwien.ac.at>
 * 
 * @brief  Test the component graph
 */

#include <boost/cstdint.hpp>
#include "dlvhex/ComponentGraph.hpp"
#include "dlvhex/DependencyGraph.hpp"
#include "dlvhex/HexParser.hpp"
#include "dlvhex/ProgramCtx.h"
#include "dlvhex/PluginInterface.h"

#define BOOST_TEST_MODULE "TestComponentGraph"
#include <boost/test/unit_test.hpp>

#include "fixturesExt1.hpp"
#include "fixturesMCS.hpp"

#include <iostream>
#include <fstream>
#include <cstdlib>

#define LOG_REGISTRY_PROGRAM(ctx) \
  LOG(*ctx.registry); \
	RawPrinter printer(std::cerr, ctx.registry); \
	std::cerr << "edb = " << *ctx.edb << std::endl; \
	LOG("idb"); \
	printer.printmany(ctx.idb,"\n"); \
	std::cerr << std::endl; \
	LOG("idb end");

inline void makeGraphVizPdf(const char* fname)
{
  std::ostringstream ss;
  ss << "dot " << fname << " -Tpdf -o " << fname << ".pdf";
  system(ss.str().c_str());
}

DLVHEX_NAMESPACE_USE

BOOST_AUTO_TEST_CASE(testNonext) 
{
  ProgramCtx ctx;
  ctx.registry = RegistryPtr(new Registry);

  std::stringstream ss;
  ss <<
    "a v f(X)." << std::endl <<
    "b :- X(a), not f(b)." << std::endl <<
    ":- X(b), not f(a)." << std::endl;
  HexParser parser(ctx);
  BOOST_REQUIRE_NO_THROW(parser.parse(ss));

	//LOG_REGISTRY_PROGRAM(ctx);

  DependencyGraph depgraph(ctx.registry);
	std::vector<ID> auxRules;
	depgraph.createDependencies(ctx.idb, auxRules);

	ComponentGraph compgraph(depgraph, ctx.registry);

  // TODO test dependencies (will do manually with graphviz at the moment)

  const char* fnamev = "testComponentGraphNonextVerbose.dot";
  LOG("dumping verbose graph to " << fnamev);
  std::ofstream filev(fnamev);
  compgraph.writeGraphViz(filev, true);
  makeGraphVizPdf(fnamev);

  const char* fnamet = "testComponentGraphNonextTerse.dot";
  LOG("dumping terse graph to " << fnamet);
  std::ofstream filet(fnamet);
  compgraph.writeGraphViz(filet, false);
  makeGraphVizPdf(fnamet);
}

BOOST_FIXTURE_TEST_CASE(testExt1, ProgramExt1ProgramCtxDependencyGraphFixture) 
{
  LOG("createing compgraph");
  ComponentGraph compgraph(depgraph, ctx.registry);

  // TODO test scc infos (will do manually with graphviz at the moment)

  const char* fnamev = "testComponentGraphExt1Verbose.dot";
  LOG("dumping verbose graph to " << fnamev);
  std::ofstream filev(fnamev);
  compgraph.writeGraphViz(filev, true);
  makeGraphVizPdf(fnamev);

  const char* fnamet = "testComponentGraphExt1Terse.dot";
  LOG("dumping terse graph to " << fnamet);
  std::ofstream filet(fnamet);
  compgraph.writeGraphViz(filet, false);
  makeGraphVizPdf(fnamet);

	// test collapsing (poor (wo)man's way)
	// [we trust on the order of components to stay the same!]
	{
		LOG("components are ordered as follows:" << printrange(
					boost::make_iterator_range(compgraph.getComponents())));
		typedef ComponentGraph::Component Component;
		ComponentGraph::ComponentIterator it, itend, itc0, itc1, itc2, itc3, itc4, itc5, itc6;
		boost::tie(it, itend) = compgraph.getComponents();
		itc0 = it; it++;
		itc1 = it; it++;
		itc2 = it; it++;
		itc3 = it; it++;
		itc4 = it; it++;
		itc5 = it; it++;
		itc6 = it; it++;
		assert(it == itend);

		std::set<Component> coll0;
		coll0.insert(*itc0);
		coll0.insert(*itc1);
		coll0.insert(*itc4);

		std::set<Component> coll1;
		coll1.insert(*itc2);
		coll1.insert(*itc5);

		std::set<Component> coll2;
		coll2.insert(*itc3);
		coll2.insert(*itc6);

		Component comp0 = compgraph.collapseComponents(coll0);
		LOG("collapsing 0 yielded component " << comp0);
		Component comp1 = compgraph.collapseComponents(coll1);
		LOG("collapsing 1 yielded component " << comp1);
		Component comp2 = compgraph.collapseComponents(coll2);
		LOG("collapsing 2 yielded component " << comp2);
	}

	// print final result
	{
		const char* fnamev = "testComponentGraphExt1CollapsedVerbose.dot";
		LOG("dumping verbose graph to " << fnamev);
		std::ofstream filev(fnamev);
		compgraph.writeGraphViz(filev, true);
		makeGraphVizPdf(fnamev);

		const char* fnamet = "testComponentGraphExt1Collapsederse.dot";
		LOG("dumping terse graph to " << fnamet);
		std::ofstream filet(fnamet);
		compgraph.writeGraphViz(filet, false);
		makeGraphVizPdf(fnamet);
	}
}

// example using MCS-IE encoding from KR 2010 for calculation of equilibria in medical example
BOOST_FIXTURE_TEST_CASE(testMCSMedEQ,ProgramMCSMedEQProgramCtxDependencyGraphFixture) 
{
	ComponentGraph compgraph(depgraph, ctx.registry);

  // TODO test scc infos (will do manually with graphviz at the moment)

  const char* fnamev = "testComponentGraphMCSMedEqVerbose.dot";
  LOG("dumping verbose graph to " << fnamev);
  std::ofstream filev(fnamev);
  compgraph.writeGraphViz(filev, true);
  makeGraphVizPdf(fnamev);

  const char* fnamet = "testComponentGraphMCSMedEqTerse.dot";
  LOG("dumping terse graph to " << fnamet);
  std::ofstream filet(fnamet);
  compgraph.writeGraphViz(filet, false);
  makeGraphVizPdf(fnamet);
}

// example using MCS-IE encoding from KR 2010 for calculation of diagnoses in medical example
BOOST_FIXTURE_TEST_CASE(testMCSMedD,ProgramMCSMedEQProgramCtxDependencyGraphFixture) 
{
	//LOG_REGISTRY_PROGRAM(ctx);

	ComponentGraph compgraph(depgraph, ctx.registry);

  // TODO test scc infos (will do manually with graphviz at the moment)

  const char* fnamev = "testComponentGraphMCSMedDVerbose.dot";
  LOG("dumping verbose graph to " << fnamev);
  std::ofstream filev(fnamev);
  compgraph.writeGraphViz(filev, true);
  makeGraphVizPdf(fnamev);

  const char* fnamet = "testComponentGraphMCSMedDTerse.dot";
  LOG("dumping terse graph to " << fnamet);
  std::ofstream filet(fnamet);
  compgraph.writeGraphViz(filet, false);
  makeGraphVizPdf(fnamet);
}

// TODO test SCCs containing extatoms
