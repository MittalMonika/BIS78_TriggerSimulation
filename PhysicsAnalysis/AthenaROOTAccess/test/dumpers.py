# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

##
# $Id: dumpers.py,v 1.42 2009-05-02 05:01:29 ssnyder Exp $
#
# @file AthenaROOTAccess/test/dumpers.py
# @author sss
# @date Jul 2007
# @brief Test for AthenaROOTAccess.
#

onlytypes = []
onlykeys = []

import ROOT
import sys
import math
from AthenaROOTAccess.persTypeToTransType import persTypeToTransType
from AthenaROOTAccess.transBranchRemap    import transBranchRemap
from PyDumper import Dumpers

rand_keys = ["CTP_Decision", "LVL1_ROI", "TrigDecision", "McEventInfo",
             "MyTrigInDetTrackTruthMap", "HLTResult_EF", "HLTResult_L2"]

known_missing = ["egDetailContainer",
                 "Analysis::TauDetailsContainer",
                 "CaloCellLinkContainer",
                 "CaloClusterCellLinkContainer",
                 "CaloCompactCellContainer",
                 "TileCellVec",
                 "Trk::TrackCollection_tlp1",
                 "Trk::TrackCollection_tlp2",
                 "Trk::TrackCollection_tlp3",
                 "Trk::TrackCollection_tlp4",
                 "Trk::TrackCollection_tlp5",
                 "Trk::SegmentCollection_tlp1",
                 "Trk::SegmentCollection_tlp2",
                 "ConvertedMboySegments",
                 "MooreSegments_",
                 "MuGirlSegments",
                 "MuonCaloEnergyContainer",
                 "JetKeyDescriptor",
                 "JetMomentMap",
                 "CaloCellContainer",
                 "TrigTauClusterDetailsContainer",
                 "BCM_RDO_Container_p0",
                 ]

_skipBranches = []

##############################################################################


class DHE:
    def __init__ (self, tree, perstype, key):
        self.tree = tree
        self.perstype = perstype
        self.key = key
        self.transtype = persTypeToTransType (perstype) if perstype else None
        self.dumped = 0
        return


def try_autokey (tree, d, dhe, keyprint):
    if not keyprint.startswith ('HLTAutoKey'):
        return None
    try:
        return tree.getViaDL (d[0], keyprint)
    except AttributeError:
        return None


def tree_getter (tree, d, dhe, keyprint):
    try:
        return getattr (tree, keyprint)
    except AttributeError:
        res = try_autokey (tree, d, dhe, keyprint)
        if res:
            return res
        return None
    return


def tree_find_type (dhe, tree):
    keys = [dhe.key]
    for k, v in transBranchRemap.items():
        if dhe.key == k[0]:
            keys.append (v)
    for k in keys:
        obj = getattr (tree, k, None)
        if obj:
            clsname = obj.__class__.__name__
            if clsname.startswith ('ROOT.'):
                clsname = clsname[5:]
            dhe.transtype = clsname
            break
    return

        

_typedefs = {'AthenaHitsVector<TrackRecord>' : 'TrackRecordCollection',
             'DataVector<Trk::Segment>'      : 'Trk::SegmentCollection',
             'TileContainer<TileMu>'         : 'TileMuContainer',
             'DataVector<xAOD::BTagging_v1>' : 'xAOD::BTaggingContainer_v1',
             'DataVector<xAOD::CaloCluster_v1>':'xAOD::CaloClusterContainer_v1',
             'DataVector<xAOD::Electron_v1>' : 'xAOD::ElectronContainer_v1',
             'DataVector<xAOD::Photon_v1>'   : 'xAOD::PhotonContainer_v1',
             'DataVector<xAOD::Muon_v1>'     : 'xAOD::MuonContainer_v1',
             'DataVector<xAOD::MuonSegment_v1>':'xAOD::MuonSegmentContainer_v1',
             'DataVector<xAOD::Jet_v1>'      : 'xAOD::JetContainer_v1',
             'DataVector<xAOD::TauJet_v1>'   : 'xAOD::TauJetContainer_v1',
             'DataVector<xAOD::TrackParticle_v1>':'xAOD::TrackParticleContainer_v1',
             'DataVector<xAOD::Vertex_v1>'   : 'xAOD::VertexContainer_v1',
             'DataVector<xAOD::TruthEvent_v1>':'xAOD::TruthEventContainer_v1',
             'DataVector<xAOD::TruthParticle_v1>':'xAOD::TruthParticleContainer_v1',
             'DataVector<xAOD::TruthVertex_v1>':'xAOD::TruthVertexContainer_v1',
             'DataVector<xAOD::PFO_v1>'      : 'xAOD::PFOContainer_v1',
             'DataVector<xAOD::MuonRoI_v1>'  : 'xAOD::MuonRoIContainer_v1',
             'DataVector<xAOD::TrigBphys_v1>': 'xAOD::TrigBphysContainer_v1',
             'DataVector<xAOD::TrigEMCluster_v1>':'xAOD::TrigEMClusterContainer_v1',
             'DataVector<xAOD::TrigElectron_v1>':'xAOD::TrigElectronContainer_v1',
             'DataVector<xAOD::TrigPhoton_v1>':'xAOD::TrigPhotonContainer_v1',
             'DataVector<xAOD::TrigMissingET_v1>':'xAOD::TrigMissingETContainer_v1',
             'DataVector<xAOD::EmTauRoI_v1>':'xAOD::EmTauRoIContainer_v1',
             'DataVector<xAOD::JetRoI_v1>':'xAOD::JetRoIContainer_v1',
             'DataVector<xAOD::MuonRoI_v1>':'xAOD::MuonRoIContainer_v1',
             }

class Evdump:
    def __init__ (self, f, f_rand, onlykeys = [], onlytypes = []):
        self.f = f
        self.f_rand = f_rand
        self.onlykeys = onlykeys
        self.onlytypes = onlytypes
        self.missed_types = {}
        self.typedefs = _typedefs
        self.invtypedefs = dict((self.typedefs[t], t) for t in self.typedefs)
        for (t, v) in self.typedefs.items():
            ROOT.gROOT.GetClass (t)
            ROOT.gInterpreter.ProcessLine ("typedef %s %s;" % (t, v))

        ROOT.SetSignalPolicy(ROOT.kSignalFast)
        return

    def add_dhe (self, key, token, find_type = None):
        if (token == '_DataHeader' or
            token.find ('(DataHeader)') > 0 or
            token == 'POOLContainer_DataHeader'):
            return
        cnt = token
        i1 = cnt.find("[CNT=")
        if i1 >= 0:
            i2 = token.find(']', i1)
            if i2 < 0:
                print "Bad token format2:", token
                return
            cnt = token[i1+5:i2]
        else:
            dhe = DHE("", "", key)
            if token.startswith ('(') and token.endswith ('/)'):
                token = persTypeToTransType (token[1:-2])
            if token != 'DataHeader':
                dhe.transtype = token
                if self.invtypedefs.has_key (token):
                    token = self.invtypedefs[token]
                    #print "PETER: new token = " , token
                l = self.types.setdefault (token, [])
                l.append (dhe)
            return
        if cnt.startswith('POOLContainer_'):
            tree = cnt
            perstype = cnt[14:]
        else:
            i3 = cnt.find ('(')
            if i3 < 0:
                print "Bad token format3:", token
                return
            tree = cnt[:i3]
            i4 = cnt.find ('/', i3)
            if i4 < 0:
                #print "Bad token format4:", token
                #return
                perstype = None
            else:
                perstype = cnt[i3+1:i4]
        dhe = DHE(tree, perstype, key)
        if perstype == None:
            find_type (dhe)
        if dhe.transtype != 'DataHeader':
            l = self.types.setdefault (dhe.transtype, [])
            l.append (dhe)
        return


    def dump_event_common (self, dhes, getter, find_type = None):
        self.types = {}
        for (k, t) in dhes:
            self.add_dhe (k, t, find_type)
        print >> self.f, "=== New event ==="
        print >> self.f_rand, "=== New event ==="
        for d in Dumpers.dumpspecs:
            dhes = self.types.get (d[0])
            dumper = d[1]
            nolist = hasattr (dumper, 'nolist') and dumper.nolist
            if not dhes: continue
            dhes.sort (key = lambda x: x.key)
            for dhe in dhes:
                keyprint = dhe.key
                if keyprint in _skipBranches:
                    dhe.dumped = 1
                    continue
                newkey = transBranchRemap.get ((keyprint, d[0]))
                if newkey:
                    keyprint = newkey
                if self.onlykeys and keyprint not in self.onlykeys: continue
                if self.onlytypes and d[0] not in self.onlytypes: continue
                if (dhe.key.startswith('Atlfast') or
                    dhe.key.startswith('HLT') or
                    dhe.key.startswith('Trig') or
                    dhe.key in rand_keys):
                    fout = self.f_rand
                else:
                    fout = self.f
                print >>fout, '--> %s/%s' % (d[0], keyprint)
                obj = getter (d, dhe, keyprint)
                if obj == None:
                    pass # print >> fout, '   (None)'
                elif nolist:
                    dumper (obj, fout)
                else:
                    Dumpers.dump_list (obj, fout, dumper)
                print >> fout, ' '
                dhe.dumped = 1

        for (typ, dhes) in self.types.items():
            for dhe in dhes:
                if not dhe.dumped and not dhe.key.endswith ('Aux.'):
                    self.missed_types.setdefault (typ, {})
                    self.missed_types[typ][dhe.key] = 1
        return


    def dump_event_tree (self, dhes, tree):
        def getter (d, dhe, keyprint):
            return tree_getter (tree, d, dhe, keyprint)
        def find_type (dhe):
            return tree_find_type (dhe, tree)
        return self.dump_event_common (dhes, getter, find_type)


    def print_missed (self, f):
        if self.onlykeys: return
        if self.onlytypes: return
        tlist = self.missed_types.keys()
        tlist.sort()
        first = 1
        for typ in tlist:
            if typ in known_missing: continue
            if first:
                print >> f, '--- DHEs not dumped ---'
                first = 0
            print >> f, typ
            klist = self.missed_types[typ].keys()
            klist.sort()
            for k in klist:
                print >> f, '  ', k
        return
    


