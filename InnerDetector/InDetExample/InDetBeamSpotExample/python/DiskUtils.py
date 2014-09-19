# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

import os, re, fnmatch, commands

# StorageManager to deal with listing, copying, reading files from different storage systems
from collections import namedtuple
StorageManager = namedtuple('StorageManager', ['name', 'prefix', 'cp', 'ls', 'longls'])
CastorMgr = StorageManager(name='castor', prefix='root://castoratlas/', cp='xrdcp', ls='nsls %s', longls='nsls -l %s')
RFIOMgr = StorageManager(name='rfio', prefix='rfio:', cp='rfcp', ls='rfdir %s', longls='rfdir %s')
#EOSMgr = StorageManager(name='eos', prefix='root://eosatlas/', cp='xrdcp', ls='/afs/cern.ch/project/eos/installation/0.1.0-22d/bin/eos.select ls', longls='/afs/cern.ch/project/eos/installation/0.1.0-22d/bin/eos.select ls -l')
#EOSMgr = StorageManager(name='eos', prefix='root://eosatlas/', cp='xrdcp', ls='eos ls', longls='eos ls -l')
EOSMgr = StorageManager(name='eos', prefix='root://eosatlas/', cp='xrdcp', ls='/bin/sh -l -c "eos ls %s"', longls='/bin/sh -l -c "eos ls -l %s"')    
UnixMgr = StorageManager(name='unix', prefix='', cp='cp', ls='ls %s', longls='ls -l %s')    

# Can also use 'xrd castoratlas ls'/'xrd eosatlas ls' but always does a long listing and so much slower than 'nsls'/'eos ls'
# source /afs/cern.ch/project/eos/installation/pro/etc/setup.sh for eos aliases until installed by default

def _hasWildcard(name) :
    """
    Return true if the name has a UNIX wildcard (*,?,[,])
    """
    if ( name.count('*') > 0 or name.count('?') > 0 or
         name.count('[') > 0 or name.count(']') > 0 ) :
        return True
    else :
        return False

def rationalise(path):
    """
    Rationalise a path, removing prefix and esuring single leading slash
    """
    for p in ('root://castoratlas/', 'root://eosatlas/', 'rfio:', 'castor:'):
        if path.startswith(p):
            path = path[len(p):]
            if path.startswith('//'):
                path = path[1:]
            if not path.startswith('/'):
                path = '/'+path
            break

    return path

def storageManager(name):
    """
    Return SotrageManager to deal with listing, copying and reading files from various storage systems
    """
    name = rationalise(name)
    if name.startswith('/castor/'):
        return CastorMgr
    elif name.startswith('/eos/'):
        return EOSMgr
    else:
        return UnixMgr

def filelist(files, prefix=None):
    """
    lists CASTOR/EOS name server directory/file entries.
    If path is a directory, filelist lists the entries in the directory;
    they are sorted alphabetically.

    `files` specifies the CASTOR/EOS pathname.
    `prefix` specifies the prefix one wants to prepend to the path found.
             (e.g. prefix='root://castoratlas/' or 'root://eosatlas//')
             if prefix=True it will determin the prefix based on the pathname

    ex:
    >>> filelist('/castor/cern.ch/atlas/*')
    >>> filelist('/castor/cern.ch/atl*/foo?[bar]/*.pool.root.?')
    >>> filelist('/eos/atlas/*', prefix='root://eosatlas/')
    >>> filelist('/castor/cern.ch/atlas/*', prefix=True)
    """

    path, fname = os.path.split(files)
    path = rationalise(path)

    if _hasWildcard(path):
        paths = ls(path)
        return sum([ls(os.path.join(p,fname))
                    for p in paths], [])

    mgr = storageManager(path)

    sc, flist = commands.getstatusoutput(mgr.ls % path)
    if sc: # command failed
        print flist
        return []
        
    flist = flist.split()
    if not (os.path.basename(files) in ['', '*']): # no need to filter
        pattern = fnmatch.translate(os.path.basename(files))
        flist = filter(lambda x: re.search(pattern, x), flist)

    if prefix:
        if isinstance(prefix, basestring):
            return [os.path.join(prefix+path, p) for p in flist]
        else:
            return [os.path.join(mgr.prefix+path, p) for p in flist]
    else:
        return [os.path.join(path, p) for p in flist]

def ls(path, longls=False):
    """
    Simple list of files

    `longls` specifies long listing format
    """

    path = rationalise(path)
    mgr = storageManager(path)

    if longls:
        return commands.getoutput(mgr.longls % path)
    else:
        return commands.getoutput(mgr.ls % path)

def cp(src, dest='.'):
    src = rationalise(src)
    dest = rationalise(dest)
    srcmgr = storageManager(src)
    destmgr = storageManager(dest)

    cp = 'cp'
    if srcmgr.cp == 'xrdcp' or destmgr.cp == 'xrdcp': cp = 'xrdcp'
    
    return os.system('%s %s%s %s%s' %(cp, srcmgr.prefix, src, destmgr.prefix, dest))
