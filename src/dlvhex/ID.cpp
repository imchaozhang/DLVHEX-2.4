#include "dlvhex/ID.hpp"
#include <boost/functional/hash.hpp>
#include <iomanip>

DLVHEX_NAMESPACE_BEGIN

std::size_t hash_value(const ID& id)
{
  std::size_t seed = 0;
  boost::hash_combine(seed, id.kind);
  boost::hash_combine(seed, id.address);
  return seed;
}

std::ostream& ID::print(std::ostream& o) const
{
  o << "ID(0x" <<
      std::setfill('0') << std::hex << std::setw(8) << kind << "," << std::setfill(' ') <<
      std::dec << std::setw(4) << address;
  if( kind == ALL_ONES && address == ALL_ONES )
    return o << " fail)"; // plus bailout

  if( !!(kind & NAF_MASK) )
    o << " naf";
  const unsigned MAINKIND_MAX = 4;
  const char* mainkinds[MAINKIND_MAX] = {
    " atom",
    " term",
    " literal",
    " rule",
  };
  const unsigned mainkind = (kind & MAINKIND_MASK) >> MAINKIND_SHIFT;
  assert(mainkind < MAINKIND_MAX);
  o << mainkinds[mainkind];

  const unsigned SUBKIND_MAX = 9;
  const char* subkinds[MAINKIND_MAX][SUBKIND_MAX] = {
    { " ordinary_ground", " ordinary_nonground", " builtin",         " aggregate", "", "", " external", "", " module" },
    { " constant",        " integer",            " variable",        " builtin",   "predicate", "", ""          },
    { " ordinary_ground", " ordinary_nonground", " builtin",         " aggregate", "", "", " external", "", " module" },
    { " regular",         " constraint",         " weak_constraint", "",           "", "", ""          }
  };
  const unsigned subkind = (kind & SUBKIND_MASK) >> SUBKIND_SHIFT;
  assert(subkind < SUBKIND_MAX);
  assert(subkinds[mainkind][subkind][0] != 0);
  o << subkinds[mainkind][subkind];
  return o << ")";
}

// returns builtin term ID
// static
ID ID::termFromBuiltinString(const std::string& op)
{
  assert(!op.empty());
  switch(op.size())
  {
  case 1:
    switch(op[0])
    {
    case '=': return ID::termFromBuiltin(ID::TERM_BUILTIN_EQ);
    case '<': return ID::termFromBuiltin(ID::TERM_BUILTIN_LT);
    case '>': return ID::termFromBuiltin(ID::TERM_BUILTIN_GT);
    case '*': return ID::termFromBuiltin(ID::TERM_BUILTIN_MUL);
    case '+': return ID::termFromBuiltin(ID::TERM_BUILTIN_ADD);
    default: assert(false); return ID_FAIL;
    }
  case 2:
    if( op == "!=" || op == "<>" )
    {
      return ID::termFromBuiltin(ID::TERM_BUILTIN_NE);
    }
    else if( op == "<=" )
    {
      return ID::termFromBuiltin(ID::TERM_BUILTIN_LE);
    }
    else if( op == ">=" )
    {
      return ID::termFromBuiltin(ID::TERM_BUILTIN_GE);
    }
    else
    {
      assert(false); return ID_FAIL;
    }
  }
  if( op == "#succ" )
  {
    return ID::termFromBuiltin(ID::TERM_BUILTIN_SUCC);
  }
  else if( op == "#int" )
  {
    return ID::termFromBuiltin(ID::TERM_BUILTIN_INT);
  }
  else
  {
    assert(false);
    return ID_FAIL;
  }
}

namespace
{
	const char* builtinTerms[] =
	{
		"=",
		"!=",
		"<",
		"<=",
		">",
		">=",
		"#sum",
		"#count",
		"#min",
		"#avg",
		"#int",
		"#succ",
		"*",
		"+",
	};
}

const char* ID::stringFromBuiltinTerm(IDAddress addr)
{
	return builtinTerms[addr];
}

DLVHEX_NAMESPACE_END
