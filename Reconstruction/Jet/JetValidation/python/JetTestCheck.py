#!/usr/bin/env python

# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration


"""
This module allows a fast comparison of the ntuple
produced by JetTest.

It reads in 2 TFile, containing similar TTree.
It get all branches from the trees containing the 
chain 'Jet'.

Then it compares tree1.branchA with tree2.branchA for each
branch and for each event. 
The summary for a branch is 'ok' if the maximum relative 
difference found is <MAX_REL_DIFF
"""

__version__ = 'JetTest_module'
__author__  = 'Pierre-Antoine Delsart (delsart@in2p3.fr), \
               Pier-Olivier DeViveiros(viveiros@physics.utoronto.ca), \
               Sergei Chekanov  (chakanau@hep.anl.gov)'
__doc__     = 'Module for jet comparison'


from ROOT import TTree, TFile
import ROOT
import re,os,sys
re_bname = re.compile("([a-zA-Z0-9]+)_.*")

import numpy
def rel_diff(v1,v2):
    try:
        return (v1 - v2)/float(min(abs(v1),abs(v2)))
    except ZeroDivisionError:
        return 0
def _gettypename(self):
    return "SimpleBr"
ROOT.TBranch.GetTypeName = _gettypename
def fullDebug_on( self, r , val='' , val2=''):
    print self.name , self.b1.GetTypeName(), r, val , val2
def fullDebug_off( self, r ,val='', v2='' ):
    pass
fullDebug = fullDebug_off


# ROOT <-> numpy 
cppToNumpyType = {
    'vector<float>'  : numpy.float32,           
    'vector<vector<float> >'  : numpy.float32,           
    'vector<int>'    : numpy.int32,
    'vector<vector<int> >'    : numpy.int32,
    'vector<double>' : numpy.float64, # ???
    'vector<double>' : numpy.bool ,
    }






def splitVarName_default( fullname ):
    res = re_bname.search(fullname)
    if not res : 
        base =""
    else:
        base = res.group(1)    
    vname = bn[len(base):]
    return base, vname

class VarPairBase(object):
    tree1 = None
    tree2 = None

    draw_selection = ""
    draw_option = ""
    draw_histospec = None


    validVariable = True
    
    @staticmethod
    def buildPair(name, b1,b2, MAX_REL_DIFF, isMeV = False):
        if isinstance(b1, ROOT.TBranchElement):
            concreteType = VarPairVector
            if 'vector<vector' in b1.GetTypeName():
                ## ROOT.vector("vector<float>")
                ## ROOT.vector("vector<int>")
                
                concreteType = VarPairVectorVector
            elif isMeV:
                concreteType = VarPairVectorMeV
        else:
            concreteType = VarPairScalar
        return concreteType(name, b1, b2, MAX_REL_DIFF)
    
    def __init__(self, name, b1,b2, MAX_REL_DIFF):
        self.name = name
        self.b1 = b1
        self.b2 = b2
        self.validVariable = (b1!=None) and (b2!=None)
        if not self.validVariable:
            return
        self.bname1 = self.b1.GetName()
        self.bname2 = self.b2.GetName()

        self.MAX_REL_DIFF = MAX_REL_DIFF
        self.initEventSummaries(0)
        self.initBranch()
        self.initEventSummaries(self.b1.GetTree().GetEntries())
        self.diff_size_event = -1
        self.nvalues = 0

        self.nbadDiff = 0

    def resetBranchPointer(self, one, t, evt):
        if one:
            self.b1 = t.GetBranch(self.bname1)
            self.b1.GetEntry( evt )
            self.v1 = getattr(t, self.bname1)

        else:
            self.b2 = t.GetBranch(self.bname2)
            self.b2.GetEntry( evt )
            self.v2 = getattr(t, self.bname2)        

    def initEventSummaries(self, nevt):
        self.max_rel_diff = numpy.zeros(nevt)
        self.max_values   = numpy.zeros(nevt)
        self.num_errors = 0

    def checkRes(self, evtNum , relDiff, a1 ):
        m = relDiff.max()
        if m < self.MAX_REL_DIFF :
            return True
        # save error info
        maxInd = relDiff.argmax()
        self.max_rel_diff[evtNum] = m
        self.max_values[evtNum]   = a1[maxInd]
        self.num_errors += sum( relDiff > self.MAX_REL_DIFF)

        return False
            


    def summary(self, silent=False, verbose=False):
        name = self.name.rjust(21)
        name = name+' '+ ('['+str(self.nvalues)+']').ljust(9)
        maxDiff = min(1 ,self.max_rel_diff.max() )
        error = 0
        statusStr = ''
        if maxDiff > self.MAX_REL_DIFF :
            ind = self.max_rel_diff.argmax()
            error += 1
            statusStr = statusStr+' max diff= %7.3f %% of % 10.3f |   at evt %4d (occured %5d / %5d) '%( maxDiff*100,self.max_values[ind] ,ind,self.num_errors,self.nvalues)            
        if self.diff_size_event> -1:
            error+= 1
            statusStr = statusStr+" vec size differed [evt "+str(self.diff_size_event)+"]" 
        if error == 0:
            statusStr = 'ok'
        if not silent:
            if error >0 or verbose:
                print '%40s'%(self.name,), statusStr

                
        return (error, self.nvalues, self.num_errors)

    def draw(self,canvas=None):
        if canvas : canvas.cd()
        bname = self.b1.GetName()
        if self.draw_histospec:
            pass # not supported yet
        else:
            hname = '>>'+bname+"H"
            self.tree1.Draw(bname+hname+'1',self.draw_selection, self.draw_option+'goff')
            h1 = self.tree1.GetHistogram()
            self.tree2.Draw(bname+hname+'2',self.draw_selection, self.draw_option+'goff')
            h2 = self.tree1.GetHistogram()        
            
        h2.SetLineColor(ROOT.kRed)

        self.histos = (h1,h2)
        h1.Draw()
        h2.Draw("same")
        

class VarPairVector(VarPairBase):
    maxVecSize = -1

    def checkxAODAuxStore(self, one ):
        
        name, b, tree = (self.bname1, self.b1, self.tree1) if one else (self.bname2, self.b2, self.tree2)
        if 'Aux.' in name and not name.endswith('Aux.') :
            from xAODAuxStoreHelper import initStaticStoreBranch
            # this properly extracts the vector<> object from the AuxStore
            # and make it available in the tree
            initStaticStoreBranch(tree,b) 
            
    def initBranch(self):
        b1, b2 = self.b1, self.b2
        #print 'initBranch  ', self, self.name, self.b2

        b1.GetEntry(0)
        b2.GetEntry(0)

        # if branches comes from an xAOD static aux store apply special
        # trick to make them available.
        self.checkxAODAuxStore(one=True)
        self.checkxAODAuxStore(one=False)
        
        
        self.v1 = getattr(self.tree1, self.bname1)
        self.v2 = getattr(self.tree2, self.bname2)
        if( type(self.v1) != type(self.v2) ):
            self.validVariable = False
            # raise Exception            
            return

        tname = b1.GetTypeName()
        nptype = cppToNumpyType.get( tname , None)
        if nptype is None :
            #print "  !!! Unknown type for variable ", b1.GetName(), tname
            self.validVariable = False
            return
        npa = numpy.ndarray

        # prepare functions which will convert to numpy array
        if self.maxVecSize==-1:
            def getarray(v):
                return npa(v.size(),dtype=nptype,buffer=v.begin().base())
        else:
            m = self.maxVecSize
            def getarray(v):
                return npa(min(m,v.size()),dtype=nptype,buffer=v.begin().base())
            
        self.getarray = getarray

    def getEntry(self,evt1 , evt2):
        self.b1.GetEntry(evt1), self.b2.GetEntry(evt2)
        return self.getarray(self.v1) , self.getarray(self.v2)

    def relDiff(self, a1, a2):
        return 2*abs(a1-a2)/numpy.maximum( abs(a1+a2), 1e-10) # maximum : avoids division by 0

    def compare(self, evt1, evt2):
        #print 'compare ', self.bname1
        a1, a2 = self.getEntry(evt1, evt2)
        n = min( len(a1) , len(a2) )
        #print "comparing ",self.name, n, len(a1) , len(a2), '|| ', self.v1[0], self.v2[0], '|| ', a1[0], a2[0]
        sizematch = True
        if len(a1) != len(a2):
            sizematch = False
            fullDebug( self, 'differing length : '+str(len(a1)), len(a2) )
            if self.diff_size_event == -1:
                self.diff_size_event = evt1
            a1 = a1[:n]
            a2 = a2[:n]
        if n == 0:
            return sizematch
        self.nvalues += n
        rd = self.relDiff(a1,a2) 
        #print evt1, '    ', rd, abs(a1+a2)

        result = self.checkRes(evt1, rd, a1 )
        if not result:
            fullDebug( self, rd, a1, a2 )
            self.nbadDiff+=1
        return result and sizematch

class VarPairVectorMeV(VarPairVector):
    def relDiff(self, a1, a2):
        d = abs(a1-a2)
        return numpy.where( d > 0.1 , d , 0 )/abs(a1+0.001) # maximum : avoids division by 0

    
class VarPairVectorVector(VarPairVector):

    def compare(self, evt1, evt2):        
        #print 'VarPairVectorVector compartor ', self.bname1

        self.b1.GetEntry(evt1), self.b2.GetEntry(evt2)

        
        getarray = self.getarray
        v1, v2 = self.v1, self.v2

        nOuter = min( v1.size() ,v2.size() )
        sizematch = True        
        if(v1.size() != v2.size() ):
            sizematch = False
            fullDebug( self, 'differing length : '+str(v1.size()), len(v2.size()) )
        if nOuter== 0:
            return sizematch

        result = True
        for i in xrange(nOuter):
            a1, a2 = getarray(v1[i]),getarray(v2[i])
            rd = self.relDiff(a1,a2)
            result = result  and self.checkRes(evt1, rd, a1 )

        if not result:
            self.nbadDiff+=1
        return result and sizematch
            
            

        

class VarPairScalar(VarPairBase):
    def initBranch(self):
        b1, b2 = self.b1, self.b2        
        t = b1.GetListOfLeaves()[0].GetTypeName()
        #print ' VarPairScalar  init ',b1.GetName(), t
        if t == 'Int_t':
            t = numpy.int32
        elif t=='Float_t':
            t = numpy.float32
        self.a1 = numpy.ndarray(1,dtype = t)
        self.a2 = numpy.ndarray(1,dtype = t)
        b1.GetTree().SetBranchAddress(b1.GetName(),self.a1)
        b2.GetTree().SetBranchAddress(b2.GetName(),self.a2)
    def getEntry(self,evt1, evt2):
        self.b1.GetEntry(evt1), self.b2.GetEntry(evt2)
        return self.a1, self.a2

    def checkRes(self, evtNum , relDiff, a1 ):
        if relDiff < self.MAX_REL_DIFF :
            return True
        self.max_rel_diff[evtNum] = relDiff
        self.max_values[evtNum]   = a1
        self.num_errors += 1
        return False


    def compare(self, evt1, evt2):
        a1, a2 = self.getEntry(evt1, evt2)
        rd = 2*abs(a1-a2)/numpy.maximum( a1+a2, 1e-10) # maximum : avoids division by 0
        self.nvalues+=1
        fullDebug( self, a1[0], a2[0] ) 
        return self.checkRes(evt1,rd[0], a1[0])

    def resetBranchPointer(self, one, t, evt):
        if one:
            self.b1 = t.GetBranch(self.bname1)
            self.b1.GetTree().SetBranchAddress(b1.GetName(),self.a1)
            self.b1.GetEntry( evt )
            self.b1 = b1
            #self.v1 = getattr(t, self.bname1)

        else:
            b2 = t.GetBranch(self.bname2)
            b2.GetTree().SetBranchAddress(b2.GetName(),self.a2)
            b2.GetEntry( evt )
            self.b2 = b2
            #self.v2 = getattr(t, self.bname2)        


# class VarScalarAndVec(VarPairBase):
#     def initBranch(self):
#         b1, b2 = self.b1, self.b2
#         t = b1.GetTitle()[-1]
#         if t == 'I':
#             t = numpy.int32
#         elif t=='F':
#             t = numpy.float32
#         self.a1 = numpy.ndarray(1,dtype = t)
#         self.a2 = numpy.ndarray(1,dtype = t)
#         b1.GetTree().SetBranchAddress(b1.GetName(),self.a1)
#         b2.GetTree().SetBranchAddress(b2.GetName(),self.a2)
#     def getEntry(self,evt1, evt2):
#         self.b1.GetEntry(evt1), self.b2.GetEntry(evt2)
#         return self.a1, self.a2
#     def compare(self, evt1, evt2):
#         a1, a2 = self.getEntry(evt1, evt2)
#         rd = 2*abs(a1-a2)/numpy.maximum( a1+a2, 1e-10) # maximum : avoids division by 0
#         self.nvalues+=1
#         return self.checkRes(evt1,rd[0], a1[0], 0)

    


def replace_Jets_by(r):
    return lambda x : x.replace('Jets',r)

class VarComparator(object):
   
    """Manage lists of VarPair objects associated to 1 or 2 trees """
    
    splitVarName = splitVarName_default
    def __init__(self,input1, input2=None, MAX_REL_DIFF=0.01, treename="JetTestTree", treename2=""):

        self.files = []
        if treename2 == "": treename2 = treename
        def setupTree( inputf , tn):
            if isinstance(inputf, ROOT.TTree ) :
                return inputf
            if inputf is None :
                return self.t1
            if not os.path.exists(inputf):
                print " Can't find ", inputf, '. Exit'
                sys.exit(1)
            f = TFile( inputf )
            tree = f.Get(tn)
            self.files.append(f)
            return tree
        self.t1 = setupTree(input1, treename)
        self.t2 = setupTree(input2, treename2)

        self.d_branches = {}
        self.MAX_REL_DIFF = MAX_REL_DIFF 
    

    def close(self):
        for f in self.files:
            f.Close()
        self.files = []
        self.d_branches = {}

    def init_trees(self,base_vars=[], branch_filter = lambda s : 'Jet' in s,
                   branch_replace = lambda s:s ,
                   completeBranchList=None,
                   additionalBranches=[],
                   checkMissing=True):
        """Initializes the comparator to compare the 2 files given in constructor
        It reads all vars in the tree which starts with the strings in base_vars

        If base_vars is empty, then it reads all branches whose names passes branch_filter(bname)==True

        branch_replace is a function used to adapt the branch name in t1 to the corresponding name in t2.
        this allows to compare branches with subtlely different  name

        completeBranchList : if non None, this is a the final list of branches to be read
        
        """
            
        t1, t2 = self.t1, self.t2
        t1.LoadTree(0)
        t2.LoadTree(0)

        VarPairBase.tree1 = t1
        VarPairBase.tree2 = t2

        # create a branch list builder  function according to arguments
        if completeBranchList is not None:
            branch_list_builder = lambda fullList : set(completeBranchList)
        elif base_vars==[]:
            branch_list_builder = lambda fullList : set( b for b in fullList if branch_filter(b) )
        else:
            branch_list_builder = lambda fullList : set( b for b in fullList if any(b.startswith(v) for v in base_vars) )

        

        if checkMissing:
            selected_br1 = branch_list_builder( [b.GetName() for b in t1.GetListOfBranches()] )
            selected_br2 = branch_list_builder( [b.GetName() for b in t2.GetListOfBranches()] )

            selected_br = selected_br1.intersection(selected_br2)    

            for b in sorted(selected_br1 - selected_br2):
                print " Missing branches in tree 2 : ", b 
            for b in sorted(selected_br2 - selected_br1):
                print " Missing branches in tree 1 : ", b
        else:
            print base_vars , branch_filter
            selected_br = branch_list_builder( [b.GetName() for b in t1.GetListOfBranches()] )

        selected_br.update( additionalBranches)
        print ' Selected branches = ', selected_br
        
        d_branches = self.d_branches
        splitVarName = self.splitVarName
        comparedBranches = []
        for bn in sorted(selected_br):
            base, vname = splitVarName( bn)
            if vname is None :
                continue

            l = d_branches.setdefault(base , [] )

            def checkBranch(t,n):
                b = t.GetBranch(n)
                if b == None : 
                    print "!!!!!!!!! ",n,'branch does not exists !'
                return b
            # guess is the value is in MeV
            isMeV = any ( bn.endswith( end ) for end in ('_e','_m','_pt', '.e','.m','.pt','Pt') )
            #print ' building ', bn, branch_replace(bn)
            v= VarPairBase.buildPair(vname,  checkBranch(t1,bn) , checkBranch(t2,branch_replace(bn)), self.MAX_REL_DIFF , isMeV = isMeV)
            if v.validVariable:                
                l.append( v )
                setattr(self, bn.replace('.','_') , v )
                comparedBranches +=[bn]
            else:
                print "Don't know how to compare branch, ignoring : ", bn
        return d_branches



    def compare_event(self,evt1, evt2,stopOnError=False):
        ok = True
        for jet, l_var in self.d_branches.iteritems():
            if stopOnError:
                for v in l_var:
                    if not v.compare(evt1, evt2):
                        v.summary()
                        return False
            else:
                for v in l_var:
                    ok =  v.compare(evt1, evt2) and ok #, doPrint = self.verbose)
        return ok

    def resetBranches(self,one, evt):
        tree = self.t1 if one else self.t2
        for jet, l_var in self.d_branches.iteritems():
            for v in l_var:
                v.resetBranchPointer(one, tree, evt)

    def full_compare(self,stopOnError=False,debug=True, maxEvent=10e10, verbose=False):
        global fullDebug, fullDebug_on, fullDebug_off
        ok = True
                
        fullDebug = fullDebug_on if debug else fullDebug_off
        t1, t2 = self.t1, self.t2
        nEvt = min(maxEvent,self.t1.GetEntries(), self.t2.GetEntries() )
        for i in xrange(nEvt):
            # support for TChain
            evti1 = t1.LoadTree(i)
            evti2 = t2.LoadTree(i)
            # in case of TChain, one might need to reset branches.
            if i!= evti1: self.resetBranches(True, evti1)
            if i!= evti2: self.resetBranches(False, evti2)

            ok = ok & self.compare_event(evti1,evti2, stopOnError)
            #print 'Event --->',i, ok, stopOnError
            if not ok and stopOnError:
                break

        if not stopOnError or ok :
            print
            err, values , wrongvalues = 0,0,0
            nvar = 0
            for jet, l_var in self.d_branches.iteritems():
                nvar += len(l_var)
                print 'Summary for ',jet, ' num var =', len(l_var),' **********************'
                #print 'Variable with errors'.rjust(22)
                for v in l_var:
                    nerr , nvalues, nwrongvalues = v.summary(verbose=verbose)
                    err += bool(nerr)
                    values += nvalues
                    wrongvalues +=nwrongvalues
                print '*******************************************'
            print
            print "Total Number of differing variable =",err, " / ", nvar
            print "Total Number of differing values   =",wrongvalues , " / " , values 
            return err
        
        if not ok:
            return -999
        return 0
        #return ok

def Draw(typ, exp, sel='',evt=None, *args):
    print exp.replace('X1',typ)
    if evt:
        if len(args)==0 : args=['']
        args += [1,evt]
    return t1.Draw(exp.replace('X1',typ), sel.replace('X1',typ) , *args)

def Scan(typ, exp, sel='', evt=None, *args):
    print exp.replace('X1',typ)
    if evt:
        if len(args)==0 : args=['']
        args += [1,evt]
    return t1.Scan(exp.replace('X1',typ), sel.replace('X1',typ) , *args)



