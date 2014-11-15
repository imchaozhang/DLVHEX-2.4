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
 * @file   PluginContainer.cpp
 * @author Roman Schindlauer
 * @author Thomas Krennwallner
 * @author Peter Schueller
 * @date   Thu Sep 1 17:25:55 2005
 * 
 * @brief  Container class for plugins.
 * 
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "dlvhex2/PluginContainer.h"
#include "dlvhex2/Configuration.h"
#include "dlvhex2/Error.h"
#include "dlvhex2/Logger.h"
#include "dlvhex2/PluginInterface.h"
#include "dlvhex2/Registry.h"

#include <ltdl.h>

#include <sys/types.h>
//#include <dirent.h>
//#include <pwd.h>
#ifdef WIN32
#include <Windows.h>
#endif

#include <boost/foreach.hpp>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <set>

DLVHEX_NAMESPACE_BEGIN

#ifdef POSIX
	#define setenv(VAR, VAL, V) ::setenv(VAR, VAL, V)
	#define unsetenv(VAR) ::unsetenv(VAR)
#else
	void setenv(const char* var, const char* val, int v){
		SetEnvironmentVariable(var, val);
	}

	void unsetenv(const char* var){
		SetEnvironmentVariable(var, NULL);
	}
#endif

struct PluginContainer::LoadedPlugin
{
  // handle is NULL for statically linked plugins
  lt_dlhandle handle;
  PluginInterfacePtr plugin;
  
  LoadedPlugin(lt_dlhandle handle, PluginInterfacePtr plugin):
    handle(handle), plugin(plugin) {}
};

namespace
{

typedef PluginContainer::LoadedPlugin LoadedPlugin;
typedef boost::shared_ptr<LoadedPlugin> LoadedPluginPtr;
typedef PluginContainer::LoadedPluginVector LoadedPluginVector;
typedef PluginInterface* (*t_import)();
typedef int (*t_getversion)();

int
findplugins(const char* filename, lt_ptr data)
{
  std::vector<std::string>* pluginlist = reinterpret_cast<std::vector<std::string>*>(data);

  std::string fn(filename);
  std::string::size_type base = fn.find_last_of("/");

  // if basename starts with 'libdlvhexplugin', then we should have a plugin here
  // we cannot simply try to open any lib, as opening could already overwrite loaded symbols
  // (if we load our own base library again, this re-constructs global data structures)
  if (fn.substr(base).find("/libdlvhexplugin") == 0)
    {
      pluginlist->push_back(fn);
    }

  return 0;
}

void findPluginLibraryCandidates(const std::string& searchpath, std::vector<std::string>& libcandidates)
{
  std::string oldenv_ld;
  const char *envld = ::getenv("LD_LIBRARY_PATH");
  if( envld )
  {
    oldenv_ld = envld;
    unsetenv("LD_LIBRARY_PATH");
  }

  try
  {
    if (lt_dlinit())
      {
        throw GeneralError("Could not initialize libltdl");
      }
    
    //
    // now look into the user's home, and into the global plugin directory
    //
    
    LOG(PLUGIN,"findPluginLibraryCandidates with searchpath='" << searchpath << "'");
    if (lt_dlsetsearchpath(searchpath.c_str()))
      {
        throw GeneralError("Could not set libltdl search path: " + searchpath);
      }

    // search the directory search paths for plugins and setup pluginList
    LOG(PLUGIN,"looking for plugin libraries named 'libdlvhexplugin...'");
    lt_dlforeachfile(NULL, findplugins, reinterpret_cast<void*>(&libcandidates));
  }
  catch(...)
  {
    if( !oldenv_ld.empty() )
      setenv("LD_LIBRARY_PATH", oldenv_ld.c_str(), 1);
    throw;
  }
  if( !oldenv_ld.empty() )
    setenv("LD_LIBRARY_PATH", oldenv_ld.c_str(), 1);
}

void loadCandidates(
    const std::vector<std::string>& libnames,
    LoadedPluginVector& plugins)
{
  BOOST_FOREACH(const std::string& lib, libnames)
  {
    LOG(PLUGIN,"loading Plugin Library: '" << lib << "'");
    // reset lt_dlerror
    (void)lt_dlerror();
    lt_dlhandle dlHandle = lt_dlopenext(lib.c_str());

    // do while false for breaking out easily
    do
    {
      if( dlHandle == NULL )
      {
        LOG(WARNING,"Selected library '" << lib << "' for opening but cannot open: lt_dlerror='" << lt_dlerror() << "' (skipping)");
        LOG(WARNING,"(You might want to use 'LD_DEBUG={files,help,all} dlvhex2 <arguments>' to debug plugin loading.))");
        break;
      }

      t_getversion getversion = reinterpret_cast<t_getversion>(lt_dlsym(dlHandle, PLUGINABIVERSIONFUNCTIONSTRING));
      if( getversion == NULL )
      {
        LOG(INFO,"Library '" << lib << "' selected for opening, but found no "
          "version function '" << PLUGINABIVERSIONFUNCTIONSTRING << "' (skipping)");
        break;
      }

      t_import getplugin = reinterpret_cast<t_import>(lt_dlsym(dlHandle, PLUGINIMPORTFUNCTIONSTRING));
      if( getplugin == NULL )
      {
        LOG(INFO,"Library '" << lib << "' selected for opening, but found no "
          "import function '" << PLUGINIMPORTFUNCTIONSTRING << "' (skipping)");
        break;
      }

      // verify ABI version
      //
      // we use the Apache APR approach to versioning
      // see https://apr.apache.org/versioning.html
      DBGLOG(DBG,"now checking plugin ABI version for " << lib);
      int iversion = getversion();
      {
        int imajor = iversion / 10000;
        int iminor = (iversion / 100) % 100;
        int imicro = iversion % 100;
        LOG(INFO,"got ABI version " << iversion << " from " << lib <<
            ", interpreted this as (" << imajor << "," << iminor << "," << imicro << ")");
        if( imajor != DLVHEX_ABI_VERSION_MAJOR )
        {
          LOG(INFO,"Library '" << lib << "' returned incompatible major ABI version: " <<
              imajor << " (library) != " << DLVHEX_ABI_VERSION_MAJOR << " (dlvhex) (skipping)");
          break;
        }
        if( iminor > DLVHEX_ABI_VERSION_MINOR )
        {
          LOG(INFO,"Library '" << lib << "' returned incompatible minor ABI version: " <<
              iminor << " (library) > " << DLVHEX_ABI_VERSION_MINOR << " (dlvhex) (skipping)");
          break;
        }
        // ignore micro abi version
        // version check successful
      }

      // get it!
      DBGLOG(DBG,"now calling plugin import function for " << lib);
      PluginInterface* plugin(getplugin());
      DBGLOG(DBG,"plugin import function returned " << printptr(plugin));

      // now wrap in non-deleting shared_ptr
      PluginInterfacePtr nopDeletingPluginPtr(plugin, PluginPtrNOPDeleter<PluginInterface>());

      plugins.push_back(LoadedPluginPtr(new LoadedPlugin(dlHandle, nopDeletingPluginPtr)));
    }
    while(false);
  }
}

void selectLoadedPlugins(
    LoadedPluginVector& plugins,
    LoadedPluginVector& candidates)
{
  // remove those with duplicate names
  std::set<std::string> names;
  BOOST_FOREACH(LoadedPluginPtr lplugin, plugins)
  {
    names.insert(lplugin->plugin->getPluginName());
  }
  DBGLOG(DBG,"selectLoadedPlugins: already loaded: " << printset(names));

  LoadedPluginVector::iterator it = candidates.begin();
  while(it != candidates.end())
  {
    const std::string& pname = (*it)->plugin->getPluginName();
    if( names.find(pname) != names.end() )
    {
      // warn, unload, remove, restart loop

      // warn
      LOG(WARNING,"already loaded a plugin with name " << pname << " (skipping)");

      WARNING("TODO check if any pointer is used?")
      DBGLOG(DBG,"usage count on interface ptr is " << (*it)->plugin.use_count() << " (should be 1)");

      // unload lib
      if( 0 != lt_dlclose((*it)->handle) )
      {
        LOG(WARNING,"failed unloading plugin library " << pname << ":" << lt_dlerror());
      }
      // remove
      candidates.erase(it);
      // restart
      it = candidates.begin();
    }
    else
    {
      // check next
      ++it;
    }
  }
}

} // anonymous namespace

#if 0
PluginContainer::PluginContainer(const PluginContainer& pc):
  registry(pc.registry),
  searchPath(pc.searchPath),
  plugins(pc.plugins),
  pluginAtoms(pc.pluginAtoms)
{
}
#endif


PluginContainer::PluginContainer()
{
}

PluginContainer::~PluginContainer()
{
  // these are pointers also existing in vector plugins -> destroy them first
  pluginInterfaces.clear();

  DBGLOG(DBG,"unloading plugins");
  // unload all plugins in reverse order
  for(LoadedPluginVector::reverse_iterator it = plugins.rbegin();
      it != plugins.rend(); ++it)
  {
    LoadedPluginPtr lp = *it;
    const std::string& pname = lp->plugin->getPluginName();
    if( lp->handle == 0 )
    {
      DBGLOG(DBG,"unloading plugin '" << pname << "' not necessary (NULL handle)");
      continue;
    }

    LOG(DBG,"about to unload loaded plugin '" << pname << "'");

    WARNING("TODO check if any pointer is used?")
    unsigned use = lp->plugin.use_count();
    if( use != 1 )
      LOG(WARNING,"usage count on PluginInterfacePtr is " << use << " (should be 1)");

    // unload lib
    if( 0 != lt_dlclose(lp->handle) )
    {
      LOG(WARNING,"failed unloading plugin library '" << pname << "':" << lt_dlerror());
    }
  }
  DBGLOG(DBG,"done unloading");
}

// search for plugins in searchpath and open those that are plugins
// may be called multiple times with different paths
// paths may be separated by ":" just like LD_LIBRARY_PATH
void PluginContainer::loadPlugins(const std::string& search)
{
  LOG_SCOPE(PLUGIN,"loadPlugins",false);

  // find candidates
  std::vector<std::string> libcandidates;
  findPluginLibraryCandidates(search, libcandidates);

  // TODO probably preselect using library names and already loaded plugins

  // load candidates
  LoadedPluginVector plugincandidates;
  loadCandidates(libcandidates, plugincandidates);

  // TODO probably select/unload using PluginInterface and already loaded plugins
  selectLoadedPlugins(plugins, plugincandidates);

  // add new plugins to list of loaded plugins
  BOOST_FOREACH(LoadedPluginPtr cand, plugincandidates)
  {
    // (automatically adds atoms)
    addInternalPlugin(cand);
  }

  // add to existing search path
  if( !searchPath.empty() )
    searchPath += ":";
  searchPath += search;
}

// add dlhandle and plugin interface to the container
void PluginContainer::addInternalPlugin(LoadedPluginPtr lplugin)
{
  LOG(PLUGIN,"adding PluginInterface '" << lplugin->plugin->getPluginName() << "' with dlhandle " << lplugin->handle);

  plugins.push_back(lplugin);
  pluginInterfaces.push_back(lplugin->plugin);
}

// add a PluginInterface to the container
void PluginContainer::addInternalPlugin(PluginInterfacePtr plugin)
{
  addInternalPlugin(LoadedPluginPtr(new LoadedPlugin(0, plugin)));
}

// call printUsage for each loaded plugin
void PluginContainer::printUsage(
    std::ostream& o)
{
  BOOST_FOREACH(PluginInterfacePtr plugin, pluginInterfaces)
  {
    o << std::endl <<
      "Plugin help for " << plugin->getPluginName() << ":" << std::endl;
	  plugin->printUsage(o);
  }
}

DLVHEX_NAMESPACE_END

// Local Variables:
// mode: C++
// End:
