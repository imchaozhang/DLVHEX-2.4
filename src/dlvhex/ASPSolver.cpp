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
 * @file ASPSolver.cpp
 * @author Thomas Krennwallner
 * @date Tue Jun 16 14:34:00 CEST 2009
 *
 * @brief ASP Solvers
 *
 *
 */

#define __STDC_LIMIT_MACROS
#include <boost/cstdint.hpp>
#include "dlvhex/ASPSolver.h"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#undef DLVHEX_BENCHMARK

#include "dlvhex/Benchmarking.h"
#include "dlvhex/DLVProcess.h"
#include "dlvhex/DLVresultParserDriver.h"
#include "dlvhex/ProgramCtx.h"
#include "dlvhex/AnswerSet.hpp"

#ifdef HAVE_LIBDLV
# include "dlv.h"
#endif

#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <list>

#if 0
// activate benchmarking if activated by configure option --enable-debug
#  ifdef DLVHEX_DEBUG
#    define DLVHEX_BENCHMARK
#  endif

#include "dlvhex/Benchmarking.h"
#include "dlvhex/PrintVisitor.h"
#include "dlvhex/Program.h"
#include "dlvhex/globals.h"
#include "dlvhex/AtomSet.h"

#include <boost/scope_exit.hpp>
#include <boost/typeof/typeof.hpp> // seems to be required for scope_exit
#include <cassert>
#endif

DLVHEX_NAMESPACE_BEGIN

namespace ASPSolver
{

DLVSoftware::Options::Options():
  ASPSolverManager::GenericOptions(),
  rewriteHigherOrder(false),
  dropPredicates(false),
  arguments()
{
  arguments.push_back("-silent");
}

DLVSoftware::Options::~Options()
{
}

struct DLVSoftware::Delegate::Impl
{
  Options options;
  DLVProcess proc;
  RegistryPtr reg;

  Impl(const Options& options):
    options(options)
  {
  }

  ~Impl()
  {
    int retcode = proc.close();

    // check for errors
    if (retcode == 127)
    {
      throw FatalError("LP solver command `" + proc.path() + "´ not found!");
    }
    else if (retcode != 0) // other problem
    {
      std::stringstream errstr;

      errstr <<
	"LP solver `" << proc.path() << "´ "
	"bailed out with exitcode " << retcode << ": "
	"re-run dlvhex with `strace -f´.";

      throw FatalError(errstr.str());
    }
  }

  void setupProcess()
  {
    proc.setPath(DLVPATH);
    if( options.includeFacts )
      proc.addOption("-facts");
    else
      proc.addOption("-nofacts");
    BOOST_FOREACH(const std::string& arg, options.arguments)
    {
      proc.addOption(arg);
    }
  }
};

DLVSoftware::Delegate::Delegate(const Options& options):
  pimpl(new Impl(options))
{
}

DLVSoftware::Delegate::~Delegate()
{
}

#define CATCH_RETHROW_DLVDELEGATE \
  catch(const GeneralError& e) { \
    std::stringstream errstr; \
    int retcode = pimpl->proc.close(); \
    errstr << pimpl->proc.path() << " (exitcode = " << retcode << \
      "): " << e.getErrorMsg(); \
    throw FatalError(errstr.str()); \
  } \
  catch(const std::exception& e) \
  { \
    throw FatalError(pimpl->proc.path() + ": " + e.what()); \
  }

void
DLVSoftware::Delegate::useASTInput(const ASPProgram& program)
{
  DLVHEX_BENCHMARK_REGISTER_AND_SCOPE(sid,"DLVSoftware::Delegate::useASTInput");

  DLVProcess& proc = pimpl->proc;
  pimpl->reg = program.registry;
  assert(pimpl->reg);

  // TODO HO checks
  //if( idb.isHigherOrder() && !options.rewriteHigherOrder )
  //  throw SyntaxError("Higher Order Constructions cannot be solved with DLVSoftware without rewriting");

  // in higher-order mode we cannot have aggregates, because then they would
  // almost certainly be recursive, because of our atom-rewriting!
  //if( idb.isHigherOrder() && idb.hasAggregateAtoms() )
  //  throw SyntaxError("Aggregates and Higher Order Constructions must not be used together");

  try
  {
    pimpl->setupProcess();
    // handle maxint
    if( program.maxint > 0 )
    {
      std::ostringstream os;
      os << "-N=" << program.maxint;
      proc.addOption(os.str());
    }
    // request stdin as last parameter
    proc.addOption("--");
    // fork dlv process
    proc.spawn();

    std::ostream& programStream = proc.getOutput();

    // output program
    RawPrinter printer(programStream, program.registry);
    // TODO HO stuff
    //PrinterPtr printer;
    //if( options.rewriteHigherOrder )
    //  printer = PrinterPtr(new HOPrintVisitor(programStream));
    //else
    //  printer = PrinterPtr(new DLVPrintVisitor(programStream));

    if( program.edb != 0 )
    {
      // print edb interpretation as facts
      program.edb->printAsFacts(programStream);
      programStream << "\n";
      programStream.flush();
    }

    printer.printmany(program.idb, "\n");
    programStream.flush();

    proc.endoffile();
  }
  CATCH_RETHROW_DLVDELEGATE
}

#if 0
void
DLVSoftware::Delegate::useStringInput(const std::string& program)
{
  DLVHEX_BENCHMARK_REGISTER_AND_SCOPE(sid,"DLVSoftware::Delegate::useStringInput");

  try
  {
    setupProcess();
    // request stdin as last parameter
    proc.addOption("--");
    // fork dlv process
    proc.spawn();
    proc.getOutput() << program << std::endl;
    proc.endoffile();
  }
  CATCH_RETHROW_DLVDELEGATE
}

void
DLVSoftware::Delegate::useFileInput(const std::string& fileName)
{
  DLVHEX_BENCHMARK_REGISTER_AND_SCOPE(sid,"DLVSoftware::Delegate::useFileInput");

  try
  {
    setupProcess();
    proc.addOption(fileName);
    // fork dlv process
    proc.spawn();
    proc.endoffile();
  }
  CATCH_RETHROW_DLVDELEGATE
}
#endif

namespace
{
  class DLVResults:
    public ASPSolverManager::Results
  {
  public:
    typedef std::list<AnswerSet::Ptr> Storage;
    Storage answersets;
    bool resetCurrent;
    Storage::const_iterator current;

    DLVResults():
      resetCurrent(true),
      current() {}
    virtual ~DLVResults() {}

    void add(AnswerSet::Ptr as)
    {
      answersets.push_back(as);

      // we do this because I'm not sure if a begin()==end() iterator
      // becomes begin() or end() after insertion of the first element
      // (this is the failsafe version)
      if( resetCurrent )
      {
	current = answersets.begin();
	resetCurrent = false;
      }
    }

    virtual AnswerSet::Ptr getNextAnswerSet()
    {
      // if no answer set was ever added, or we reached the end
      if( (resetCurrent == true) ||
	  (current == answersets.end()) )
      {
	return AnswerSet::Ptr();
      }
      else
      {
	Storage::const_iterator ret = current;
	current++;
	return *ret;
      }
    }
  };
}

ASPSolverManager::ResultsPtr 
DLVSoftware::Delegate::getResults()
{
  DLVHEX_BENCHMARK_REGISTER_AND_SCOPE(sid,"DLVSoftware::Delegate::getResults");

  //LOG("getting results");
  try
  {
    // for now, we parse all results and store them into the result container
    // later we should do kind of an online processing here

    boost::shared_ptr<DLVResults> ret(new DLVResults);

    // parse result
    DLVResultParser parser(pimpl->reg);
    // TODO HO stuff
    // options.dropPredicates?(DLVresultParserDriver::HO):(DLVresultParserDriver::FirstOrder));
    parser.parse(pimpl->proc.getInput(), boost::bind(&DLVResults::add, ret.get(), _1));

    ASPSolverManager::ResultsPtr baseret(ret);
    return baseret;
  }
  CATCH_RETHROW_DLVDELEGATE
}

//
// DLVLibSoftware
//
#ifdef HAVE_LIBDLV

struct DLVLibSoftware::Delegate::Impl
{
  Options options;
  PROGRAM_HANDLER *ph;
  RegistryPtr reg;

  Impl(const Options& options):
    options(options),
    ph(create_program_handler())
  {
    if( options.includeFacts )
      ph->setOption(INCLUDE_FACTS, 1);
    else
      ph->setOption(INCLUDE_FACTS, 0);
    BOOST_FOREACH(const std::string& arg, options.arguments)
    {
      if( arg == "-silent" )
      {
	// do nothing?
      }
      else
	throw std::runtime_error("dlv-lib commandline option not implemented: " + arg);
    }
  }

  ~Impl()
  {
    destroy_program_handler(ph);
  }
};

DLVLibSoftware::Delegate::Delegate(const Options& options):
  pimpl(new Impl(options))
{
}

DLVLibSoftware::Delegate::~Delegate()
{
}

void
DLVLibSoftware::Delegate::useASTInput(const ASPProgram& program)
{
  DLVHEX_BENCHMARK_REGISTER_AND_SCOPE(sid,"DLVLibSoftware::Delegate::useASTInput");

  // TODO HO checks

  try
  {
    pimpl->reg = program.registry;
    assert(pimpl->reg);
    if( program.maxint != 0 )
      pimpl->ph->setMaxInt(program.maxint);

    pimpl->ph->clearProgram();

    // output program
    std::stringstream programStream;
    RawPrinter printer(programStream, program.registry);
    // TODO HO stuff

    if( program.edb != 0 )
    {
      // print edb interpretation as facts
      program.edb->printAsFacts(programStream);
      programStream << "\n";
      programStream.flush();
    }

    printer.printmany(program.idb, "\n");
    programStream.flush();

    LOG("sending program to dlv-lib:===");
    LOG(programStream.str());
    LOG("==============================");
    pimpl->ph->Parse(programStream);
    pimpl->ph->ResolveProgram(SYNCRONOUSLY);
  }
  catch(const std::exception& e)
  {
    LOG("EXCEPTION: " << e.what());
    throw;
  }
}

// reuse DLVResults class from above

ASPSolverManager::ResultsPtr 
DLVLibSoftware::Delegate::getResults()
{
  DLVHEX_BENCHMARK_REGISTER_AND_SCOPE(sid,"DLVLibSoftware::Delegate::getResults");

  //LOG("getting results");
  try
  {
    // for now, we parse all results and store them into the result container
    // later we should do kind of an online processing here

    boost::shared_ptr<DLVResults> ret(new DLVResults);

    // TODO really do incremental model fetching
    const std::vector<MODEL> *models = pimpl->ph->getAllModels();
    std::vector<MODEL>::const_iterator itm;
    // iterate over models
    for(itm = models->begin();
        itm != models->end(); ++itm)
    {
      AnswerSet::Ptr as(new AnswerSet(pimpl->reg));

      // iterate over atoms
      for(MODEL_ATOMS::const_iterator ita = itm->begin();
  	ita != itm->end(); ++ita)
      {
        const char* pname = ita->getName();
        assert(pname);
  
        // i have a hunch this might be the encoding
        assert(pname[0] != '-');

	typedef std::list<const char*> ParmList;
	ParmList parms;
  	//LOG("creating predicate with first term '" << pname << "'");
	parms.push_back(pname);
  
	// TODO HO stuff
	// TODO integer terms

        for(MODEL_TERMS::const_iterator itt = ita->getParams().begin();
  	  itt != ita->getParams().end(); ++itt)
        {
	  switch(itt->type)
	  {
	  case 1:
	    // string terms
	    //std::cerr << "creating string term '" << itt->data.item << "'" << std::endl;
	    parms.push_back(itt->data.item);
	    break;
	  case 2:
	    // int terms
	    //std::cerr << "creating int term '" << itt->data.number << "'" << std::endl;
	    assert(false);
	    //ptuple.push_back(new dlvhex::Term(itt->data.number));
	    break;
	  default:
	    throw std::runtime_error("unknown term type!");
	  }
        }

	// for each param in parms: find id and put into tuple
#warning TODO create something like inline ID TermTable::getByStringOrRegister(const std::string& symbol, IDKind kind)
	Tuple ptuple;
	ptuple.reserve(parms.size());
	assert(pimpl->reg);
	for(ParmList::const_iterator itp = parms.begin();
	    itp != parms.end(); ++itp)
	{
	  // constant term
	  ID id = pimpl->reg->terms.getIDByString(*itp);
	  if( id == ID_FAIL )
	  {
	    Term t(ID::MAINKIND_TERM  | ID::SUBKIND_TERM_CONSTANT, *itp);
	    id = pimpl->reg->terms.storeAndGetID(t);
	  }
	  assert(id != ID_FAIL);
	  LOG("got term " << *itp << " with id " << id);
	  ptuple.push_back(id);
	}

	// ogatom
	ID fid = pimpl->reg->ogatoms.getIDByTuple(ptuple);
	if( fid == ID_FAIL )
	{
	  OrdinaryAtom a(ID::MAINKIND_ATOM | ID::SUBKIND_ATOM_ORDINARYG);
	  a.tuple.swap(ptuple);
	  {
	    #warning parsing efficiency problem see HexGrammarPTToASTConverter
	    std::stringstream ss;
	    RawPrinter printer(ss, pimpl->reg);
	    Tuple::const_iterator it = ptuple.begin();
	    printer.print(*it);
	    it++;
	    if( it != ptuple.end() )
	    {
	      ss << "(";
	      printer.print(*it);
	      it++;
	      while(it != ptuple.end())
	      {
		ss << ",";
		printer.print(*it);
		it++;
	      }
	      ss << ")";
	    }
	    a.text = ss.str();
	  }
	  fid = pimpl->reg->ogatoms.storeAndGetID(a);
	  LOG("added fact " << a << " with id " << fid);
	}
	LOG("got fact with id " << fid);
	assert(fid != ID_FAIL);
	as->interpretation->setFact(fid.address);
      }

      ret->add(as);
    }
  
    // TODO: is this necessary?
    // delete models;
   
    ASPSolverManager::ResultsPtr baseret(ret);
    return baseret;
  }
  catch(const std::exception& e)
  {
    LOG("EXCEPTION: " << e.what());
    throw;
  }
}
#endif // HAVE_LIBDLV



#if 0
#if defined(HAVE_DLVDB)
DLVDBSoftware::Options::Options():
  DLVSoftware::Options(),
  typFile()
{
}

DLVDBSoftware::Options::~Options()
{
}

DLVDBSoftware::Delegate::Delegate(const Options& opt):
  DLVSoftware::Delegate(opt),
  options(opt)
{
}

DLVDBSoftware::Delegate::~Delegate()
{
}

void DLVDBSoftware::Delegate::setupProcess()
{
  DLVSoftware::Delegate::setupProcess();

  proc.setPath(DLVDBPATH);
  proc.addOption("-DBSupport"); // turn on database support
  proc.addOption("-ORdr-"); // turn on rewriting of false body rules
  if( !options.typFile.empty() )
    proc.addOption(options.typFile);

}

#endif // defined(HAVE_DLVDB)
#endif

} // namespace ASPSolver

DLVHEX_NAMESPACE_END

/* vim: set noet sw=2 ts=8 tw=80: */
// Local Variables:
// mode: C++
// End:
