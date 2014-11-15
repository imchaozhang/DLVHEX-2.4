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
 * @file   fixtureOfflineMB.cpp
 * @author Peter Schueller <ps@kr.tuwien.ac.at>
 * 
 * @brief  Implementation of testing fixtures for offline model building.
 */

#ifndef FIXTUREOFFLINEMB_HPP_INCLUDED__24092010
#define FIXTUREOFFLINEMB_HPP_INCLUDED__24092010

#include "fixtureE1.h"
#include "fixtureE2.h"
#include "fixtureEx1.h"

#include "dlvhex2/OfflineModelBuilder.h"
#include "dlvhex2/Logger.h"

#include <boost/test/unit_test.hpp>

// fixture for testing offline model building with various graphs
// we setup model generator factories
template<typename EvalGraphBaseFixtureT>
struct OfflineModelBuilderTFixture:
  public EvalGraphBaseFixtureT
{
  typedef EvalGraphBaseFixtureT Base;
  typedef dlvhex::OfflineModelBuilder<TestEvalGraph> ModelBuilder;
  typedef ModelBuilder::OptionalModel OptionalModel;

  dlvhex::ModelBuilderConfig<TestEvalGraph> cfg;
  ModelBuilder omb;
  EvalUnit ufinal;

  OfflineModelBuilderTFixture():
    EvalGraphBaseFixtureT(),
    cfg(Base::eg),
    omb(cfg)
  {
    LOG_SCOPE(INFO,"OfflineModelBuilderTFixture<...>", true);
    typedef TestEvalUnitPropertyBase UnitCfg;
    typedef TestEvalGraph::EvalUnitDepPropertyBundle UnitDepCfg;
    TestEvalGraph& eg = Base::eg;

    // setup final unit
    BOOST_TEST_MESSAGE("adding ufinal");
    LOG(INFO,"ufinal = " << ufinal);
    ufinal = eg.addUnit(UnitCfg());

    TestEvalGraph::EvalUnitIterator it, itend;
    boost::tie(it, itend) = eg.getEvalUnits();
    for(; it != itend && *it != ufinal; ++it)
    {
      LOG(INFO,"setting up TestModelGeneratorFactory on unit " << *it);
      eg.propsOf(*it).mgf.reset( 
        new TestModelGeneratorFactory(eg.propsOf(*it).ctx));

      LOG(INFO,"adding dependency from ufinal to unit " << *it << " join order " << *it);
      // we can do this because we know that eval units (= vertices of a vecS adjacency list) are unsigned integers
      eg.addDependency(ufinal, *it, UnitDepCfg(*it));
    }
  }

  ~OfflineModelBuilderTFixture() {}
};

// create one E1 model building fixture
typedef OfflineModelBuilderTFixture<EvalGraphE1Fixture>
  OfflineModelBuilderE1Fixture;

// create one normal E2 model building fixture
typedef OfflineModelBuilderTFixture<EvalGraphE2Fixture>
  OfflineModelBuilderE2Fixture;

// create one E2 model building fixture with mirrored join order u2/u3
typedef OfflineModelBuilderTFixture<EvalGraphE2MirroredFixture>
  OfflineModelBuilderE2MirroredFixture;

// create one Ex1 model building fixture
typedef OfflineModelBuilderTFixture<EvalGraphEx1Fixture>
  OfflineModelBuilderEx1Fixture;

#endif // FIXTUREOFFLINEMB_HPP_INCLUDED__24092010
