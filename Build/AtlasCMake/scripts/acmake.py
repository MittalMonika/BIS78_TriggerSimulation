#!/usr/bin/env python

# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
#
# $Id: acmake.py 768404 2016-08-17 13:12:02Z elmsheus $
#
# Helper script for replacing the most useful features of cmt that we used
# in the ATLAS software development.
#

# Basic import(s):
import os
import os.path
import subprocess
import re

## C(++) style main function
def main():

    # Set up the command line parser:
    import optparse
    parser = optparse.OptionParser( description = "Lightweight replacement for "
                                    "the most used CMT features with CMake",
                                    version = "$Revision: 768404 $",
                                    usage = "%prog [command] [options]" )
    parser.add_option( "-r", "--revision", type="string", dest="revision",
                       action="store", default=None,
                       help="Tag/branch of the package to check out" )
    ( options, command ) = parser.parse_args()

    # Make sure that only one command was given to the script:
    if not len( command ):
        parser.error( "At least one command is required" )
        return 1

    # Handle the different commands:
    if command[ 0 ] == "co" or command[ 0 ] == "checkout":
        if len( command ) != 2:
            parser.error( "You need to specify a package after 'co'" )
            return 1
        if hasattr( options, "revision" ):
            return checkout( command[ 1 ], options.revision )
        else:
            return checkout( command[ 1 ] )
    elif command[ 0 ] == "sv" or command[ 0 ] == "show_versions":
        if len( command ) != 2:
            parser.error( "You need to specify a package after 'show_versions'" )
            return 1
        return show_versions( command[ 1 ] )
    elif command[ 0 ] == "sc" or command[ 0 ] == "show_clients":
        if len( command ) != 2:
            parser.error( "You need to specify a package after 'show_clients'" )
            return 1
        return show_clients( command[ 1 ] )
    else:
        parser.error( "Command '%s' not recognised" % command[ 0 ] )
        return 1

    # Return gracefully:
    return 0

## Function to check out a package from SVN
#
# This function emulates the behaviour of "cmt co" for easily checking
# out packages from the offline repository. (Or whatever repository is
# set up using the SVNROOT environment variable.)
#
# @param package Full package path [e.g. Control/AthContainers]
# @param tag Optional tag/branch name to check out
#
def checkout( package, tag = None ):

    # Get the SVNROOT environment variable, if it exists. If it doesn't fall
    # back on the offline SVN server.
    svnroot = os.getenv( "SVNROOT", "svn+ssh://svn.cern.ch/reps/atlasoff" )

    # The default is that we check out the trunk of the package:
    svnpath = "%s/%s/trunk" % ( svnroot, package )

    # fnull is needed in a few places here:
    FNULL = open( os.devnull, "w" )

    # If we received a tag, look for it in the "tags", "branches" and
    # "devbranches" subdirectories:
    if tag:
        tagFound = False
        for subdir in [ "tags", "branches", "devbranches" ]:
            # Check if the path exists:
            if subprocess.call( [ "svn", "info",
                                  "%s/%s/%s/%s" % ( svnroot, package, subdir,
                                                    tag ) ],
                                stdout = FNULL, stderr = FNULL ) != 0:
                continue
            # It does exist.
            tagFound = True
            svnpath = "%s/%s/%s/%s" % ( svnroot, package, subdir, tag )
            break
        # Check whether the tag was found:
        if not tagFound:
            print( "Couldn't find tag \"%s\" for package \"%s\"" %
                   ( tag, package ) )
            return 1
        pass

    # Check out the package:
    if subprocess.call( [ "svn", "co", svnpath, package ] ) != 0:
        print( "Failed to check out %s" % svnpath )
        return 1

    # Decide about the package's version. The default is "<pkgName>-trunk".
    version = "%s-trunk" % os.path.basename( package )
    if tag:
        # If the user asked for an explicit version, just use that:
        version = tag
    else:
        # If not, then construct a version name using the trunk's revision.
        info = subprocess.Popen( [ "svn", "info", svnpath ],
                                 stdout = subprocess.PIPE,
                                 stderr = FNULL ).communicate()[ 0 ]
        # Look for the revision in the info:
        m = re.search( "Revision: ([0-9]+)", info )
        if m:
            version = "%s-r%s" % ( os.path.basename( package ), m.group( 1 ) )
            pass
        pass

    # Now that we know the version, write it into a file called "version.cmake":
    vfile = open( os.path.join( package, "version.cmake" ), "w" )
    vfile.write( version )
    vfile.close()

    # For backwards compatibility, if the package has a "cmt" subdirectory, then
    # write the version into "cmt/version.cmt" as well.
    if os.path.exists( os.path.join( package, "cmt" ) ):
        vfile = open( os.path.join( package, "cmt", "version.cmt" ), "w" )
        vfile.write( version )
        vfile.close()
        pass

    # Return gracefully:
    return 0

## Function checking the version(s) of a package in the release
#
# This is a replacement for the "cmt show versions ..." command. Listing the
# versions of a given package in the currently configured release.
#
# @param package Full package path, or just the package's name
#
def show_versions( package ):

    # Get the CMAKE_PREFIX_PATH environment variable. As we'll be searching
    # in that.
    ppath = os.getenv( "CMAKE_PREFIX_PATH" )
    if not ppath:
        print( "ERROR: No release seems to be set up" )
        return 1

    # Print some header:
    print( "Available version(s) of package %s:" % package )

    # Keep track of which directories were already visited:
    releaseDirectories = []

    # Iterate over the release directories:
    for rdir in ppath.split( ":" ):
        # Skip it if we already looked at this directory:
        if rdir in releaseDirectories:
            continue
        # Look for a file called "packages.txt" in this directory:
        pfilename = os.path.join( rdir, "packages.txt" )
        if not os.access( pfilename, os.R_OK ):
            continue
        # Open the file:
        pfile = open( pfilename, "r" )
        # Look for the package:
        matches = re.findall( "[^\n]*%s [^\n]+" % package, pfile.read() )
        for m in matches:
            print( "  %s:\n    %s" % ( rdir, m ) )
            pass
        pass

    # Return gracefully:
    return 0

## Function collecting the packages that are the clients of the one specified
#
# This is a replacement for the "cmt show clients ..." command. Listing the
# names of the packages that publicly or privately depend on the package
# that the user specified.
#
# @param package The name of the package. Not containing it full path.
#
def show_clients( package ):

    # Get the CMAKE_PREFIX_PATH environment variable. As we'll be searching
    # in that.
    ppath = os.getenv( "CMAKE_PREFIX_PATH" )
    if not ppath:
        print( "ERROR: No release seems to be set up" )
        return 1

    # Keep track of which directories were already visited:
    releaseDirectories = []

    # Target file finding regular expression:
    target_re = re.compile( ".*-targets.cmake$" )

    # Collect the names of all the files that the code will need to look at:
    target_files = []
    for rdir in ppath.split( ":" ):
        # Skip it if we already looked at this directory:
        if rdir in releaseDirectories:
            continue
        # Look for a file called *-targets.cmake in its cmake/ subdirectory:
        cmake_dir = os.path.join( rdir, "cmake" )
        if not (os.path.isdir(cmake_dir) or os.path.exists(cmake_dir)):
            continue
        tfiles = [ f for f in os.listdir( cmake_dir )
                   if os.path.isfile( os.path.join( cmake_dir, f ) ) and
                   target_re.search( f ) ]
        if len( tfiles ) != 1:
            print( "WARNING: Multiple target files found in directory %s" % \
                       cmake_dir )
            print( "WARNING: Found files: %s" % str( target_files ) )
            pass
        # Add this to the overall list:
        target_files += [ os.path.join( cmake_dir, f ) for f in tfiles ]
        pass

    ## Sub-function collecting the clients of one given package
    #
    # @param package The name of the package to collect the clients of
    # @param public Set to <code>True</code> to look for public dependencies,
    #               or to <code>False</code> to look for private ones
    # @param recursive Set to <code>True</code> to look for (public) clients
    #                  recursively
    # @returns A set with the names of the requested clients
    #
    def collect_clients( package, public, recursive ):
        # A small sanity check:
        if not public and recursive:
            print( "ERROR: It doesn't make sense to ask for private clients "
                   "recursicely!" )
            return set([])
        # Construct the regular expression to use:
        client_re = ""
        if public:
            client_re = \
                "^set_target_properties\(\w+::(\w+)Pkg PROPERTIES[^\)]*" \
                "INTERFACE_LINK_LIBRARIES \"(?:[^\"]*[:;]+)?%sPkg[^\"]*\"" % \
                package
        else:
            client_re = \
                "^set_target_properties\(\w+::(\w+)PkgPrivate PROPERTIES[^\)]*" \
                "INTERFACE_LINK_LIBRARIES \"(?:[^\"]*[:;]+)?%sPkg[^\"]*\"" % \
                package
            pass
        # The result list:
        result = set([])
        # Loop over the files:
        for fname in target_files:
            # Open the file:
            target_file = open( fname, "r" )
            # Take the direct clients of this package:
            clients = re.findall( client_re, target_file.read(),
                                  re.MULTILINE )
            # If recursion is asked for, do that:
            if recursive:
                for client in clients:
                    for pkg in collect_clients( client, public, True ):
                        result.add( pkg )
                        pass
                    pass
                pass
            # Finally, add the direct dependencies:
            for client in clients:
                result.add( client )
                pass
            pass
        # Return the resulting set:
        return result

    # Collect and print the results:
    print( "Clients for package %s:" % package )
    direct_private = collect_clients( package, False, False )
    print( "  Direct private clients : %s" % str( list( direct_private ) ) )
    direct_public = collect_clients( package, True, False )
    print( "  Direct public clients  : %s" % str( list( direct_public ) ) )
    print( "  Indirect public clients: %s" % \
               str( list( collect_clients( package, True, True ) -
                          direct_public - direct_private ) ) )

    # Return gracefully:
    return 0

# Execute the main function:
if __name__ == "__main__":
    import sys
    sys.exit( main() )
