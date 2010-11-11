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
 * @file   TestEvalEndToEnd.cpp
 * @author Peter Schueller <ps@kr.tuwien.ac.at>
 * 
 * @brief  Test evaluation starting from HEX program to final models.
 *
 * Functional external atoms are provided in fixture.
 */

#include "dlvhex/EvalGraphBuilder.hpp"
#include "dlvhex/EvalHeuristicOldDlvhex.hpp"
#include "dlvhex/HexParser.hpp"
#include "dlvhex/ProgramCtx.h"
#include "dlvhex/PluginInterface.h"
#include "dlvhex/DependencyGraph.hpp"
#include "dlvhex/ComponentGraph.hpp"
#include "dlvhex/ModelGenerator.hpp"
#include "dlvhex/OnlineModelBuilder.hpp"

// this must be included before dummytypes!
#define BOOST_TEST_MODULE __FILE__
#include <boost/test/unit_test.hpp>

#include "fixturesExt1.hpp"

#include <iostream>
#include <fstream>
#include <cstdlib>

#define LOG_REGISTRY_PROGRAM(ctx) \
  ctx.registry->logContents(); \
	RawPrinter printer(std::cerr, ctx.registry); \
	LOG("edb"); \
	printer.printmany(ctx.edb,"\n"); \
	std::cerr << std::endl; \
	LOG("edb end"); \
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

typedef FinalEvalGraph::EvalUnit EvalUnit;
typedef OnlineModelBuilder<FinalEvalGraph> FinalOnlineModelBuilder;
typedef FinalOnlineModelBuilder::Model Model;
typedef FinalOnlineModelBuilder::OptionalModel OptionalModel;

BOOST_FIXTURE_TEST_CASE(testEvalHeuristicExt1,ProgramExt1ProgramCtxDependencyGraphComponentGraphFixture) 
{
  // eval graph
  FinalEvalGraph eg;

  {
    // create builder that supervises the construction of eg
    EvalGraphBuilder egbuilder(ctx, compgraph, eg);

    {
      // create heuristic, which sends commands to egbuilder
      EvalHeuristicOldDlvhex heuristicOldDlvhex(egbuilder);
      heuristicOldDlvhex.build();
      LOG("building eval graph finished");

      // log the (changed) component graph
      {
        const char* fnamev = "testEvalEndToEndExt1Verbose.dot";
        LOG("dumping verbose graph to " << fnamev);
        std::ofstream filev(fnamev);
        compgraph.writeGraphViz(filev, true);
        makeGraphVizPdf(fnamev);

        const char* fnamet = "testEvalEndToEndExt1Terse.dot";
        LOG("dumping terse graph to " << fnamet);
        std::ofstream filet(fnamet);
        compgraph.writeGraphViz(filet, false);
        makeGraphVizPdf(fnamet);
      }
    }
  }

  //
  // evaluate
  //
  FinalOnlineModelBuilder omb(eg);

  EvalUnit ufinal;

  // setup final unit
  {
    BOOST_TEST_MESSAGE("adding ufinal");
    LOG("ufinal = " << ufinal);
    ufinal = eg.addUnit(FinalEvalGraph::EvalUnitPropertyBundle());

    FinalEvalGraph::EvalUnitIterator it, itend;
    boost::tie(it, itend) = eg.getEvalUnits();
    for(; it != itend && *it != ufinal; ++it)
    {
      LOG("adding dependency from ufinal to unit " << *it << " join order " << *it);
      // we can do this because we know that eval units (= vertices of a vecS adjacency list) are unsigned integers
      eg.addDependency(ufinal, *it, FinalEvalGraph::EvalUnitDepPropertyBundle(*it));
    }
  }

  LOG("initial eval/model graph:");
  omb.logEvalGraphModelGraph();

  // evaluate
  BOOST_MESSAGE("requesting model #1");
  OptionalModel m1 = omb.getNextIModel(ufinal);

  LOG("eval/model graph after requesting first model");
  omb.logEvalGraphModelGraph();
}
