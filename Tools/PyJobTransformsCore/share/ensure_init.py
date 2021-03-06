#!/usr/bin/env python
# usage: ensure_init.py [options] [files] [options] [files]
# Makes sure that the file __init__.py exists in the last directory specified
# with the -s=<dir> option in the arguments. Any other arguments will be ignored.
# If the files does not exist, a dummy one will be created.
# If no -s=<dir> option is given, nothing is done, and program exits with error code 1.
import sys,os,glob
if len(sys.argv) <= 1: sys.exit(1)
srcdir = None
action = False
for args in sys.argv[1:]:
    for arg in args.split():
        if arg.startswith('-s='):
            srcdir=arg[3:]
            if srcdir and os.path.exists(srcdir) and glob.glob( os.path.join(srcdir,'*.py') ):
                # an __init__.py file must be there
                initfile = os.path.join(srcdir,'__init__.py')
                if not os.path.exists(initfile):
                    # make an (almost) empty file
                    action = True
                    print "Creating file %s" % initfile
                    init = file(initfile,'w')
                    init.write("# Auto-generated by %s%s" % (' '.join(sys.argv),os.linesep) )
                    init.close()
                else:
                    print "__init__.py found in %s" % srcdir

if not action:
    print "No action needed"
