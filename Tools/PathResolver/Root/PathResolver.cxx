/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "PathResolver/PathResolver.h"

#ifndef XAOD_STANDALONE
#include "GaudiKernel/System.h"
#endif


#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <stdlib.h>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/filesystem.hpp>

#include "TFile.h"
#include "TSystem.h"

namespace bf = boost::filesystem;
using namespace std;


static const char* path_separator = ",:";


//
///////////////////////////////////////////////////////////////////////////
//

typedef enum {
  PR_regular_file,
  PR_directory
} PR_file_type;

typedef enum {
  PR_local,
  PR_recursive
} PR_search_type;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static bool
PR_find( const bf::path& file, const string& search_list,
         PR_file_type file_type, PathResolver::SearchType search_type,
         string& result ) {

  bool found(false);

  // look for file as specified first

  try {
    if ( ( file_type == PR_regular_file && is_regular_file( file ) ) ||
         ( file_type == PR_directory && is_directory( file ) ) ) {
      result = bf::system_complete(file).string();
      return true;
    }
  } catch (bf::filesystem_error /*err*/) {
  }

  // assume that "." is always part of the search path, so check locally first

  try {
    bf::path local = bf::initial_path() / file;
    if ( ( file_type == PR_regular_file && is_regular_file( local ) ) ||
         ( file_type == PR_directory && is_directory( local ) ) ) {
      result = bf::system_complete(file).string();
      return true;
    }
  } catch (bf::filesystem_error /*err*/) {
  }

   std::string locationToDownloadTo = "."; //will replace with first search location 

  // iterate through search list
  vector<string> spv;
  split(spv, search_list, boost::is_any_of( path_separator), boost::token_compress_on);
  for (vector<string>::const_iterator itr = spv.begin();
       itr != spv.end(); ++itr ) {

   if( itr->find("http//")==0 && file_type==PR_regular_file ) { //only http download files, not directories
      //try to do an http download to the local location
      //restore the proper http protocal (had to remove for sake of env var splitting) 
      std::string addr = "http://"; addr += itr->substr(6,itr->length());
      bf::path lp = locationToDownloadTo/file;
      bf::path lpd = lp; lpd.remove_filename();
      std::cout << "PathResolver    INFO   Attempting http download of " << addr << "/" << file.string() << " to " << lp << std::endl;
      
      if(!is_directory(lpd)) {
         std::cout << "PathResolver    INFO   Creating directory: " << lpd << std::endl;
         if(!boost::filesystem::create_directories(lpd)) {
            std::cerr << "PathResolver   ERROR  Unable to create directories to write file to : " << lp << std::endl;
            throw std::runtime_error("Unable to download calibration file");
         }
      }
      std::string fileToDownload = addr + "/" + file.string();
      if(!TFile::Cp(fileToDownload.c_str(),(locationToDownloadTo+"/"+file.string()).c_str())) {
         std::cerr << "PathResolver    ERROR  Unable to download file : " << fileToDownload << std::endl;
      } else {
         std::cout << "PathResolver    INFO   Successfully downloaded." << std::endl;
         itr = spv.begin(); //reset to first element, which is where we downloaded to
      }
   } else if(locationToDownloadTo==".") {
      //prefer first non-pwd location for downloading to. This should be the local InstallArea in cmt
      locationToDownloadTo=*itr;
   }

   //std::cout << "searching path: " << *itr << std::endl;

    bf::path fp = *itr / file;

    try {
      if ( ( file_type == PR_regular_file && is_regular_file( fp ) ) ||
           ( file_type == PR_directory && is_directory( fp ) ) ) {
        result = bf::system_complete(fp).string();
        return true;
      }
    } catch (bf::filesystem_error /*err*/) {
    }


    // if recursive searching requested, drill down
    if (search_type == PathResolver::RecursiveSearch &&
        is_directory( bf::path(*itr) ) ) {

      bf::recursive_directory_iterator end_itr;
      try {
        for ( bf::recursive_directory_iterator ritr( *itr );
              ritr != end_itr; ++ritr) {

          // skip if not a directory
          if (! is_directory( bf::path(*ritr) ) ) { continue; }

          bf::path fp2 = bf::path(*ritr) / file;
          if ( ( file_type == PR_regular_file && is_regular_file( fp2 ) ) ||
               ( file_type == PR_directory && is_directory( fp2 ) ) ) {
            result = bf::system_complete( fp2 ).string();
            return true;
          }
        }
      } catch (bf::filesystem_error /*err*/) {
      }
    }

  }

  return found;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

string
PathResolver::find_file(const std::string& logical_file_name,
              const std::string& search_path,
              SearchType search_type) {
   //std::cout << "finding file: " <<logical_file_name << " in path=" << search_path << std::endl;

  std::string path_list;

#ifdef XAOD_STANDALONE
   const char* envVarVal = gSystem->Getenv(search_path.c_str());
   if(envVarVal == NULL) {
      std::cout << "PathResolver    ERROR   " << search_path.c_str() << " environment variable not defined!" << std::endl;
      path_list = ""; //this will allow search in pwd ... maybe we should throw exception though!
   }
   else { path_list = envVarVal; }
#else
  System::getEnv(search_path, path_list);
#endif

  return (find_file_from_list (logical_file_name, path_list, search_type));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

std::string
PathResolver::find_file_from_list (const std::string& logical_file_name,
                                   const std::string& search_list,
                                   SearchType search_type)
{
  std::string result("");

  bf::path lfn( logical_file_name );

  /* bool found = */
  PR_find (lfn, search_list, PR_regular_file, search_type, result);

  // The following functionality was in the original PathResolver, but I believe
  // that it's WRONG. It extracts the filename of the requested item, and searches
  // for that if the preceding search fails. i.e., if you're looking for "B/a.txt",
  // and that fails, it will look for just "a.txt" in the search list.

  // if (! found && lfn.filename() != lfn ) {
  //   result = "";
  //   PR_find (lfn.filename(), search_list, PR_regular_file, search_type, result);
  // }

  return (result);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

string PathResolver::find_directory (const std::string& logical_file_name,
                                     const std::string& search_path,
                                     SearchType search_type)
{
  std::string path_list = gSystem->Getenv(search_path.c_str());

  return (find_directory_from_list (logical_file_name, path_list, search_type));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

string
PathResolver::find_directory_from_list (const std::string& logical_file_name,
                                        const std::string& search_list,
                                        SearchType search_type)
{
  std::string result;

  if (!PR_find (logical_file_name, search_list, PR_directory, search_type, result))
  {
    result = "";
  }

  return (result);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PathResolver::SearchPathStatus
PathResolver::check_search_path (const std::string& search_path)
{
  std::string path_list = gSystem->Getenv(search_path.c_str());
  if ( path_list.empty() )
    return (EnvironmentVariableUndefined);

  vector<string> spv;
  boost::split( spv, path_list, boost::is_any_of( path_separator ), boost::token_compress_on);
  vector<string>::iterator itr=spv.begin();

  try {
    for (; itr!= spv.end(); ++itr) {
      bf::path pp(*itr);
      if (!is_directory(pp)) {
        return (UnknownDirectory);
      }
    }
  } catch(bf::filesystem_error /*err*/) {
    return (UnknownDirectory);
  }

  return ( Ok );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

std::string PathResolverFindXMLFile (const std::string& logical_file_name)
{
  return PathResolver::find_file (logical_file_name, "XMLPATH");
}

std::string PathResolverFindDataFile (const std::string& logical_file_name)
{
  return PathResolver::find_file (logical_file_name, "DATAPATH");
}

std::string PathResolverFindCalibFile (const std::string& logical_file_name)
{
  return PathResolver::find_file (logical_file_name, "CALIBPATH");
}

