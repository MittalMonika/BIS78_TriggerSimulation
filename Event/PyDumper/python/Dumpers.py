# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

##
# $Id: Dumpers.py,v 1.32 2009-05-22 18:34:31 ssnyder Exp $
#
# @file PyDumper/python/Dumpers.py
# @author sss
# @date Jul 2007
# @brief python library to dump various EDM classes
#
# The code in this file is used to dump EDM classes to text.
# It should be usable from both ARA and Athena.
#

from __future__ import with_statement

__doc__ = """python library to dump various EDM classes"""
__version__ = "$Revision: 1.32 $"
__author__ = "Scott Snyder, Sebastien Binet"

### imports
from contextlib import contextmanager
import math
import sys
from math import \
     log as math_log,\
     sqrt as math_sqrt,\
     hypot as math_hypot

from AthenaPython import PyAthena
import ROOT
import PyCintex

# not available due to a reflex bug.
etcone10 = 0
nucone10 = 8


### helper methods ------------------------------------------------------------
def asinh (x):
    arg = math_sqrt (x*x+1) - x
    if arg == 0:
        return 1e30  # protect against loss of precision.
    return - math_log (arg)

def toiter (beg, end):
    while beg != end:
        yield beg.__deref__()
        beg.__preinc__()
    return

def toiter1 (c):
    return toiter (c.begin(), c.end())

def nolist (f):
    f.nolist = 1
    return f

def tonone (x):
    if not x: return None
    return x

@contextmanager
def signalstate (o, state):
    if hasattr(o, 'setSignalState'):
        old = o.signalState()
        o.setSignalState (state)
        yield
        o.setSignalState (old)
    else:
        sh = PyAthena.SignalStateHelper (o)
        sh.setSignalState (state)
        yield
        sh.releaseObject
    return


### library methods ------------------------------------------------------------

def dump_H3V (v, f):
    print >> f, "(%f %f %f)" % \
          (v.perp(),v.eta(),v.phi()),
    return

def dump_HLV (v, f):
    m2 = v.e()**2 - v.px()**2 - v.py()**2 - v.pz()**2
    if m2 < 0:
        m = - math_sqrt (-m2)
    else:
        m = math_sqrt (m2)
    #if abs(m-int(m)-0.5) < 1e-4: m += 0.01

    pt = math_hypot (v.px(), v.py())
    if pt < 1e-3:
        if v.pz() > 0:
            eta = 100
        else:
            eta = -100
    else:
        eta = asinh (v.pz() / pt)

    print >> f, "(%f %f %f %f)" % \
          (v.perp(),eta,v.phi(), m),
    return


def dump_Fourvec (v, f, parens=1):
    ParticleImpl_EEtaPhiM = getattr(PyAthena,'ParticleImpl<NavigableTerminalNode,P4ImplEEtaPhiM,ParticleEvent::Base>')

    #vmom = v.momentumBase() if hasattr(v, 'momentumBase') else None
    vmom = None
    if isinstance (v,    PyAthena.P4EEtaPhiMBase) or \
       isinstance (v,    ParticleImpl_EEtaPhiM) or \
       isinstance (vmom, PyAthena.P4BaseEEtaPhiM) or \
       isinstance (v, PyAthena.xAOD.IParticle):
        m = v.m()
        eta = v.eta()
        pt = v.pt()
    elif isinstance (v,    PyAthena.P4PtEtaPhiMBase) or \
         isinstance (vmom, PyAthena.P4BasePtEtaPhiM):
        m = v.m()
        eta = v.eta()
        pt = v.pt()
    elif isinstance (v,    PyAthena.P4IPtCotThPhiMBase) or \
         isinstance (vmom, PyAthena.P4BaseIPtCotThPhiM):
        m = v.m()
        ipt = v.iPt()
        cotth = v.cotTh()

        # Special case: avoid div-by-zero for null vector.
        if ipt == 0 and m == 0 and cotth == 0 and v.phi() == 0:
            pt = 0
            eta = 0
        else:
            aux = math_hypot(1, cotth)
            eta = math_log ((aux-cotth) / (aux+cotth))
            pt = 1 / ipt
    else: # PxPyPzE-based
        pt = v.pt()
        
        # Calculating m from px,py,pz,e is prone to FP significance
        # problems, as m may be small compared to momenta.
        # We can get different results from m() in debug/opt builds.
        # Do the calculation in python instead for consistency.
        m2 = v.e()**2 - v.px()**2 - v.py()**2 - v.pz()**2
        if m2 < 0:
            m = - math_sqrt (-m2)
        else:
            m = math_sqrt (m2)

        # Same thing shows up for eta.
        pt = math_hypot (v.px(), v.py())
        if pt == 0:
            if v.pz() > 0:
                eta = 100
            else:
                eta = -100
        else:
            eta = asinh (v.pz() / pt)

    #if abs(m-int(m)-0.5) < 1e-4: m += 0.01
    if parens: print >> f, '(',
    print >> f, "%f %f %f %f" % (pt, eta, v.phi(), m),
    if parens: print >> f, ')',
    return

def dump_Threevec (v, f):
    print >> f, "(%f %f %f)" % (v.x(), v.y(), v.z()),
    return

def dump_Twovec (v, f):
    print >> f, "(%f %f)" % (v.x(), v.y()),
    return

def dump_AmgMatrix (m, f):
    print >> f, '[',
    for r in range(m.rows()):
        print >> f, '[',
        for c in range(1, m.cols()):
            v = m(r,c)
            if abs(v) < 1e-38: v = 0
            print >> f, '%6.3g' % v,
        print >> f, ']',
    print >> f, ']',
    return

def dump_AmgVector (m, f):
    print >> f, '[',
    for r in range(m.rows()):
        v = m(r)
        if abs(v) < 1e-38: v = 0
        print >> f, '%6.3g' % v,
    print >> f, ']',
    return

def dump_ParticleBase (e, f):
    dump_Fourvec (e, f, 0)
    orig = e.originLink()
    if e.hasCharge():
        charge = "%f" % e.charge()
    else:
        charge = "N/A"
    print >> f, " %1d %1d %4d %s" % \
          (e.dataType(), e.hasPdgId(), e.pdgId(), charge),
    if orig.isValid():
        print >> f, "%1d"%orig.index(),
        dump_Threevec (e.origin().position(), f)
    else:
        print >> f, "(no orig)", 
    return


def dump_ParticleImpl (p, f):
    dump_Fourvec (p, f, 0)
    if  p.hasCharge():
        c = p.charge()
        if c == 0: c = 0 # change -0 -> 0
        c = '%f' % c
    else:
        c = 'N/A'
    print >> f, ' %d %s %5d ' % \
          (p.dataType(), c, p.pdgId()),
    if p.origin():
        dump_Threevec (p.origin().position(), f)
    else:
        print >> f, "(no orig)",
    return


def dump_EMConvert (d, f):
    print >> f, d.convTrackMatch(), d.convAngleMatch(),
    return

def dump_EMBremFit (d, f):
    ip = d.bremInvpT()
    iperr = d.bremInvpTerr()
    if ip==-999: ip = 0
    if iperr==-999: iperr = 0
    br = d.bremRadius()
    bx = d.bremX()
    if br == -999: br = 0
    if bx == -999: bx = 0
    nt = d.bremNTRT()
    nr = d.bremNR()
    if nt == -999: nt = 0
    if nr == -999: nr = 0
    d0 = d.bremD0()
    phi0 = d.bremPhi0()
    if d0 == -999: d0 = 0
    if phi0 == -999: phi0 = 0
    print >> f, "%f %f %f %f %f %2d %2d %2d %2d %f %f" %\
          (ip*1e6, br, bx, iperr*1e6,
           d.bremClusterRadius(), d.bremFitStatus(), d.bremErrorFlag(),
           nt, nr, d0, phi0),
    print >> f, d.bremZ0(), d.bremDzDr(), d.bremNZ(), d.bremFitChi2(),
    return

def dump_EMTrackMatch (d, f):
    print >> f, "%f %f %4d %4d" % \
          (d.deltaEta(2), d.deltaPhi(2),
           d.trackNumber(), d.tracksInBroadWindow()),
    print >> f, d.trackRefit(), d.isPrimary(),
    return

def dump_EMShower (d, f):
    weta1 = d.weta1()
    if weta1 < -10: weta1 = 0
    pos7 = d.pos7()
    if pos7 < -10: pos7 = 0
    # FIXME: backward compat
    if not hasattr(d, 'etconoisedR04Sig2'):
        d.etconoisedR04Sig2 = d.etconoise20
        d.etconoisedR04Sig3 = d.etconoise30
    print >> f, d.e011(), d.e033(), d.ethad1(), \
          d.ethad(), d.ehad1(), d.f1(), d.f3(), d.f1core(), d.f3core(),\
          d.e233(), d.e235(), d.e255(), d.e237(), d.e277(), d.e333(), \
          d.e335(), d.e337(), d.e377(), weta1, d.weta2(), d.e2ts1(), \
          d.e2tsts1(), d.fracs1(), d.widths1(), d.widths2(), \
          d.poscs1(), d.poscs2(), d.asy1(), d.pos(), pos7, d.barys1(),\
          d.wtots1(), d.emins1(), d.emaxs1(), d.etcone(), d.etcone20(), \
          d.etcone30(), d.etcone40(), \
          d.etconoisedR04Sig2(), d.etconoisedR04Sig3(), \
          d.ecore(),
    if hasattr (d, 'e131'):
        print >> f, d.e131(), d.e1153(), d.iso(),
    else:
        print >> f, d.e132(), d.e1152(), d.r33over37allcalo(),
    return

# Sometimes, we can get denormal floats in the files we read.
# However, when reading, the denormals-as-zero flag can be set
# if we've loaded any library compiled with -ffast-math.
def uf(x):
    if abs(x) < 1e-38: return 0
    return x

def dump_EMErrorDetail (d, f):
    print >> f, d.linkIndex(),
    print >> f, '\n     ',\
          uf(d.EMconvertedphoton_perigee_d0()),\
          uf(d.EMconvertedphoton_perigee_z0()),\
          uf(d.EMconvertedphoton_perigee_phi0()),\
          uf(d.EMconvertedphoton_perigee_theta()),\
          uf(d.EMconvertedphoton_perigee_eta()),\
          uf(d.EMconvertedphoton_perigee_momentum()),\
          uf(d.EMconvertedphoton_perigee_Covd0d0 ()),\
          uf(d.EMconvertedphoton_perigee_Covd0z0 ()),\
          uf(d.EMconvertedphoton_perigee_Covd0phi ()),\
          uf(d.EMconvertedphoton_perigee_Covd0theta ()),\
          uf(d.EMconvertedphoton_perigee_Covz0z0   ()),\
          uf(d.EMconvertedphoton_perigee_Covz0phi  ()),\
          uf(d.EMconvertedphoton_perigee_Covz0theta ()),\
          uf(d.EMconvertedphoton_perigee_Covphiphi ()),\
          uf(d.EMconvertedphoton_perigee_Covphitheta ()),\
          uf(d.EMconvertedphoton_perigee_Covthetatheta ()),
    print >> f, '\n     ',\
          uf(d.EMtrack_perigee_d0    ()),\
          uf(d.EMtrack_perigee_phi0  ()),\
          uf(d.EMtrack_perigee_z0    ()),\
          uf(d.EMtrack_perigee_theta ()),\
          uf(d.EMtrack_perigee_eta    ()),\
          uf(d.EMtrack_perigee_momentum ()),\
          uf(d.EMtrack_perigee_Covd0d0         ()),\
          uf(d.EMtrack_perigee_Covd0z0         ()),\
          uf(d.EMtrack_perigee_Covd0phi        ()),\
          uf(d.EMtrack_perigee_Covd0theta      ()),\
          uf(d.EMtrack_perigee_Covz0z0         ()),\
          uf(d.EMtrack_perigee_Covz0phi        ()),\
          uf(d.EMtrack_perigee_Covz0theta      ()),\
          uf(d.EMtrack_perigee_Covphiphi       ()),\
          uf(d.EMtrack_perigee_Covphitheta     ()),\
          uf(d.EMtrack_perigee_Covthetatheta   ()),
    print >> f, '\n     ',\
          uf(d.EMphoton_eta ()),\
          uf(d.EMphoton_phi0 ()),\
          uf(d.EMphoton_Eclus ()),\
          uf(d.EMphoton_Covetaeta      ()),\
          uf(d.EMphoton_Covetaphi     ()),\
          uf(d.EMphoton_CovetaEclus      ()),\
          uf(d.EMphoton_Covphiphi    ()),\
          uf(d.EMphoton_CovphiEclus    ()),\
          uf(d.EMphoton_CovEclusEclus ()),
    print >> f, '\n     ',\
          uf(d.EMtrack_comb_Covd0d0         ()),\
          uf(d.EMtrack_comb_Covd0z0         ()),\
          uf(d.EMtrack_comb_Covd0phi        ()),\
          uf(d.EMtrack_comb_Covd0eta      ()),\
          uf(d.EMtrack_comb_Covd0P     ()),\
          uf(d.EMtrack_comb_Covz0z0         ()),\
          uf(d.EMtrack_comb_Covz0phi        ()),\
          uf(d.EMtrack_comb_Covz0eta      ()),\
          uf(d.EMtrack_comb_Covz0P     ()),\
          uf(d.EMtrack_comb_Covphiphi       ()),\
          uf(d.EMtrack_comb_Covphieta     ()),\
          uf(d.EMtrack_comb_CovphiP    ()),\
          uf(d.EMtrack_comb_Covetaeta   ()),\
          uf(d.EMtrack_comb_CovetaP  ()),\
          uf(d.EMtrack_comb_CovPP ()),
    return


def dump_EMTrackFit (d, f):
    print >> f, '\n     ',\
          d.track_perigee_d0    (),\
          d.track_perigee_phi0  (),\
          d.track_perigee_qOverP(),\
          d.track_perigee_z0    (),\
          d.track_perigee_theta (),\
          d.track_perigee_Covd0d0         (),\
          d.track_perigee_Covd0z0         (),\
          d.track_perigee_Covd0phi        (),\
          d.track_perigee_Covd0theta      (),\
          d.track_perigee_Covd0qOverP     (),\
          d.track_perigee_Covz0z0         (),\
          d.track_perigee_Covz0phi        (),\
          d.track_perigee_Covz0theta      (),\
          d.track_perigee_Covz0qOverP     (),\
          d.track_perigee_Covphiphi       (),\
          d.track_perigee_Covphitheta     (),\
          d.track_perigee_CovphiqOverP    (),\
          d.track_perigee_Covthetatheta   (),\
          d.track_perigee_CovthetaqOverP  (),\
          d.track_perigee_CovqOverPqOverP (),
    print >> f, '\n     ',\
          d.track_LastM_loc1    (),\
          d.track_LastM_loc2    (),\
          d.track_LastM_phi     (),\
          d.track_LastM_theta   (),\
          d.track_LastM_qOverP  (),\
          d.bremTrackAuthor   (),\
          d.hasBrem           (),\
          d.bremRadius      (),\
          d.bremRadiusErr   (),\
          d.bremDeltaZ      (),\
          d.bremDeltaZerr   (),\
          d.bremMaterialTraversed   (),\
          d.bremFitChi2(),\
          d.bremFitStatus   (),\
          d.linkIndex         (),
    return

def dump_egamma (e, f):
    PyCintex.loadDictionary ('ElectronPhotonSelectorToolsDict')
    dump_ParticleImpl (e, f)
    print >> f, "\n      %d %d %d" % (e.author(), e.isem(), e.isemse()),
    if e.clusterElementLink().isValid():
        print >>f,  '\n      cl: %2d:' % e.clusterElementLink().index(),
        dump_Fourvec (e.cluster(), f)
    for i in range(e.nTrackParticles()):
        tp = e.trackParticleElementLink(i)
        if tp.isValid():
            print >>f,  '\n      tp: %2d:' %tp.index(),
            dump_Fourvec (e.trackParticle(i), f)
    for i in range(e.nConversions()):
        cv = e.conversionElementLink(i)
        if cv.isValid():
            print >>f,  '\n      cv: %2d:' % cv.index(),
            dump_Threevec (e.conversion(i).recVertex().position(), f)
    for i in range (e.nDetails()):
        if e.detailElementLink(i).isValid():
            d = e.detail(i)
            print >> f, '\n      %s: ' % d.className(),
            if isinstance (d, PyAthena.EMConvert):
                dump_EMConvert (d, f)
            elif isinstance (d, PyAthena.EMBremFit):
                dump_EMBremFit (d, f)
            elif isinstance (d, PyAthena.EMTrackMatch):
                dump_EMTrackMatch (d, f)
            elif isinstance (d, PyAthena.EMShower):
                dump_EMShower (d, f)
            elif isinstance (d, PyAthena.EMErrorDetail):
                dump_EMErrorDetail (d, f)
            elif isinstance (d, PyAthena.EMTrackFit):
                dump_EMTrackFit (d, f)
            else:
                print >> f, d,
        else:
            print >> f, '\n      Detail link %d not valid; skipped.' % i,
    pid = PyAthena.egammaPID
    print >> f, '\n      pid: %g %g %f %g %f %f' % \
          (e.egammaID(pid.ElectronWeight), e.egammaID(pid.BgWeight),
           e.egammaID(pid.NeuralNet), e.egammaID(pid.Hmatrix),
           e.egammaID(pid.IsolationLikelihood_jets),
           e.egammaID(pid.IsolationLikelihood_HQDelectrons)),
    return

def dump_Electron (e, f):
    dump_egamma (e, f)
    return

def dump_Photon (e, f):
    dump_egamma (e, f)
    return


def dump_Muon (m, f):
    dump_ParticleImpl (m, f)
    print >> f, ' %1d' % m.author(),
    if m.inDetTrackLink().isValid():
        print >>f,  '\n      idtp: %2d:' % m.inDetTrackLink().index(),
        dump_Fourvec (m.inDetTrackParticle(), f)
    # This isn't in 13.X.0 xxx
    if hasattr(m, 'muonSpTrackLink') and m.muonSpTrackLink().isValid():
        print >>f,  '\n      sptp: %2d:' % m.muonSpTrackLink().index(),
        dump_Fourvec (m.muonSpectrometerTrackParticle(), f)
    if m.muonExtrapTrackLink().isValid():
        print >>f,  '\n      mxtp: %2d:' % m.muonExtrapTrackLink().index(),
        dump_Fourvec (m.muonExtrapolatedTrackParticle(), f)
    if m.innerExtrapTrackLink().isValid():
        print >>f,  '\n      ixtp: %2d:' % m.innerExtrapTrackLink().index(),
        dump_Fourvec (m.innerExtrapolatedTrackParticle(), f)
    if m.combinedTrackLink().isValid():
        print >>f,  '\n      cmtp: %2d:' % m.combinedTrackLink().index(),
        dump_Fourvec (m.combinedMuonTrackParticle(), f)
    if m.clusterLink().isValid():
        print >>f,  '\n      cl: %2d:' % m.clusterLink().index(),
        dump_Fourvec (m.cluster(), f)
    print >> f, '\n      a: %f %f %d %f %d %f %d' % \
          (m.matchChi2(), m.matchChi2OverDoF(), m.matchNumberDoF(),
           m.fitChi2(), m.fitNumberDoF(), m.fitChi2OverDoF(),
           m.bestMatch()),
    print >> f, '\n      b: %d %d %d %d %d %d %d %d %d %d %d %d' % \
          (m.numberOfBLayerHits(),
           m.numberOfPixelHits(),
           m.numberOfSCTHits(),
           m.numberOfTRTHits(),
           m.numberOfTRTHighThresholdHits(),
           m.numberOfBLayerSharedHits(),
           m.numberOfPixelSharedHits(),
           m.numberOfPixelHoles(),
           m.numberOfSCTSharedHits(),
           m.numberOfSCTHoles(),
           m.numberOfTRTOutliers(),
           m.numberOfTRTHighThresholdOutliers()),
    print >> f, '\n      c: %d %d %d %d %d %d %d %d %d %d %d %d %d %d' % \
          (m.numberOfMDTHits(),
           m.numberOfMDTHoles(),
           m.numberOfCSCEtaHits(),
           m.numberOfCSCEtaHoles(),
           m.numberOfCSCPhiHits(),
           m.numberOfCSCPhiHoles(),
           m.numberOfRPCEtaHits(),
           m.numberOfRPCEtaHoles(),
           m.numberOfRPCPhiHits(),
           m.numberOfRPCPhiHoles(),
           m.numberOfTGCEtaHits(),
           m.numberOfTGCEtaHoles(),
           m.numberOfTGCPhiHits(),
           m.numberOfTGCPhiHoles()),
    print >> f, '\n      d: %d %d %f %f %f %f %f %f' % \
          (m.numberOfGangedPixels(),
           m.numberOfOutliersOnTrack(),
           m.segmentDeltaEta(),
           m.segmentChi2OverDoF(),
           m.annBarrel(),
           m.annEndCap(),
           m.innAngle(),
           m.midAngle()),
    print >> f, '\n      e: %f %f %d %d' % \
          (m.parameter(etcone10),
           m.parameter(nucone10),
           m.alsoFoundByLowPt(),
           m.alsoFoundByCaloMuonId()),
    print >> f, '\n      f:', [i for i in m.associatedEtaDigits()],
    print >> f, '\n      h:', [i for i in m.associatedPhiDigits()],
    print >> f, '\n      i:', [i for i in m.segmentEtaDigits()],
    print >> f, '\n      j:', [i for i in m.segmentPhiDigits()],
    caloe = m.caloEnergyLoss()
    if caloe:
        print >> f, '\n      k: %d %f %f %f %f' %\
              (caloe.energyLossType(),
               caloe.caloMuonIdTag(),
               caloe.caloLRLikelihood(),
               caloe.fsrCandidateEnergy(),
               caloe.etCore())
        print >> f, '\n      k:',
        for d in caloe.depositInCalo():
            print >> f, " (%d %f %f)" % \
                  (d.subCaloId(), d.energyDeposited(), d.muonEnergyLoss())
          
    return


def dump_TauDetail (t, f):
    nm = t.className()
    if nm.startswith('Analysis::'):
        nm = nm[10:]
    print >> f, '%s:' % nm,
    return
    
def dump_Tau1P3PDetails (t, f):
    dump_TauDetail (t, f)
    print >> f, ' %f %1d %f %f %1d %1d %f %f %f %f' % \
          (t.emRadius(),
           t.numStripCells(),
           t.etChrgHAD(),
           t.etIsolHAD(),
           t.nAssocTracksCore(),
           t.nAssocTracksIsol(),
           t.etEMAtEMScale(),
           t.etChrgEM(),
           t.etNeuEM(),
           t.etChrgEM01Trk(0)),
    print >> f, '\n        ',
    print >> f, ' %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f' % \
          (t.isolationFraction(),
           t.stripWidth2(),
           t.etIsolEM(),
           t.massTrk3P(),
           t.rWidth2Trk3P(),
           t.signD0Trk3P(),
           t.z0SinThetaSig(),
           t.etChrgHADoverPttot(),
           t.etIsolFrac(),
           t.etEflow(),
           t.etHadAtEMScale(),
           t.etEMCL(),
           t.etResNeuEM(),
           t.trFlightPathSig(),
           t.etResChrgEMTrk(0)),
    print >> f, '\n        ',
    dump_HLV (t.sumEM(), f)
    if t.secVertex():
        dump_Threevec (t.secVertex().position(), f)
    if t.cellEM012Cluster():
        dump_Fourvec (t.cellEM012Cluster(), f)
    for i in range(t.numPi0()):
        print >> f, '\n        pi: ',
        dump_Fourvec (t.pi0 (i), f)
    return
  

def dump_TauRecDetails (t, f):
    dump_TauDetail (t, f)
    # numTrack, trackCaloEta, trackCaloPhi
    print >> f, '%f %f %f %f %2d %f %f %f' % \
          (t.emRadius(),
           t.isolationFraction(),
           t.centralityFraction(),
           t.stripWidth2(),
           t.numStripCells(),
           t.etEMCalib(),
           t.etHadCalib(),
           t.leadingTrackPT()),
    print >> f, '\n        ',
    for i in range(t.numTrack()):
        print >> f, '(%f %f) ' % (t.trackCaloEta(i), t.trackCaloPhi(i)),
    print >> f, '\n        ',
    dump_HLV (t.sumEM(), f)
    if t.secVertex():
        dump_Threevec (t.secVertex().position(), f)
    return

    
def dump_Tau1P3PExtraDetails (t, f):
    dump_TauDetail (t, f)
    print >> f, t.sumPTTracks(),
    # Can't unpack cells in ARA; don't dump:
    #  closestEtaTrkVertCell
    #  closestPhiTrkVertCell
    #  closestEtaTrkCell
    for tr in range(t.ntrack()):
        for s in range(t.nsamp()):
            print >> f, '\n        ', tr, s, \
                  t.etaTrackCaloSamp(tr,s), t.phiTrackCaloSamp(tr,s),
    return


def dump_TauRecExtraDetails (t, f):
    dump_TauDetail (t, f)
    print >> f, t.numEMCells(), t.stripET(), t.emCentralityFraction(), \
          t.etHadAtEMScale(), t.etEMAtEMScale(), t.energy(), t.emEnergy(),\
          t.sumPTTracks(), t.seedType(), tonone(t.analysisHelper()),
    return


def dump_TauPi0Candidate (t, f):
    dump_HLV (t.hlv(), f)
    print >> f, ' [',
    for c in t.pi0ClusterVec():
        print >> f, c.clusterLink().index(), ', ',
    print >> f, ']',
    return


def dump_TauPi0Cluster (t, f):
    print >> f, t.clusterLink().index(), \
          t.deltaPhi(), \
          t.deltaTheta(), \
          t.engFracEM(), \
          t.engFracMAX(), \
          t.engFracCore(), \
          t.secondEngDens(), \
          t.LATERAL(), \
          t.LONGITUDINAL(), \
          t.EM1CoreFrac(), \
          t.asymmetryInEM1WRTTrk(), \
          t.NHitsInEM1(), \
          t.NPosECells_PS(), \
          t.NPosECells_EM1(), \
          t.NPosECells_EM2(), \
          t.firstEtaWRTClusterPosition_EM1(), \
          t.firstEtaWRTClusterPosition_EM2(), \
          t.secondEtaWRTClusterPosition_EM1(), \
          t.secondEtaWRTClusterPosition_EM2(), \
          t.BDTScore(),
    return
          


def dump_TauPi0Details (t, f):
    dump_TauDetail (t, f)
    dump_HLV (t.visTauhlv(), f)
    print >> f, '  candidates',
    for c in t.pi0CandidateVector():
        print >> f, '\n    ',
        dump_TauPi0Candidate (c, f)
    print >> f, '  clusters',
    for c in t.pi0ClusterVector():
        print >> f, '\n    ',
        dump_TauPi0Cluster (c, f)
    return


def dump_TauCommonDetails (t, f):
    dump_TauDetail (t, f)
    print >> f, t.ipZ0SinThetaSigLeadTrk(), t.etOverPtLeadTrk(), \
          t.leadTrkPt(), t.ipSigLeadTrk(), t.ipSigLeadLooseTrk(), \
          t.etOverPtLeadLooseTrk(), t.leadLooseTrkPt(), t.chrgLooseTrk(), \
          t.massTrkSys(), t.trkWidth2(),
    print >> f, '\n        ',
    print >> f, t.trFlightPathSig(), t.etEflow(), t.seedCalo_EMRadius(), \
          t.seedCalo_hadRadius(), t.seedCalo_etEMAtEMScale(), \
          t.seedCalo_etHadAtEMScale(), t.seedCalo_isolFrac(), \
          t.seedCalo_centFrac(), t.seedCalo_stripWidth2(), t.seedCalo_nStrip(),
    print >> f, '\n        ',
    print >> f, t.seedCalo_etEMCalib(), t.seedCalo_etHadCalib(), \
          t.seedCalo_eta(), t.seedCalo_phi(), t.seedCalo_nIsolLooseTrk(), \
          t.seedTrk_EMRadius(), t.seedTrk_isolFrac(), \
          t.seedTrk_etChrgHadOverSumTrkPt(), t.seedTrk_isolFracWide(), \
          t.seedTrk_etHadAtEMScale(),
    print >> f, '\n        ',
    print >> f, t.seedTrk_etEMAtEMScale(), t.seedTrk_etEMCL(),\
          t.seedTrk_etChrgEM(), t.seedTrk_etNeuEM(), \
          t.seedTrk_etResNeuEM(), t.seedTrk_hadLeakEt(), \
          t.seedTrk_sumEMCellEtOverLeadTrkPt(), t.seedTrk_secMaxStripEt(), \
          t.seedTrk_stripWidth2(), t.seedTrk_nStrip(),
    print >> f, '\n        ',
    print >> f, t.seedTrk_etChrgHad(), t.seedTrk_nOtherCoreTrk(),\
          t.seedTrk_nIsolTrk(), t.seedTrk_etIsolEM(), t.seedTrk_etIsolHad(),
    print >> f, '\n        %d loose tracks' % t.nLooseTrk(),
    for (i,el) in enumerate(t.looseTrk()):
        print >> f, '\n          ', el.index(), el.dataID(), \
              t.seedTrk_etChrgEM01Trk(i), t.seedTrk_etResChrgEMTrk(i),
    print >> f, '\n        %d pi0s' % t.nPi0(),
    for el in t.pi0LinkVec():
        # nb. indices may vary from run to run.  list isn't sorted?
        print >> f, '\n          ', el.dataID(), el.e(),
    print >> f, '\n        pi0 sum:',
    dump_HLV (t.sumPi0Vec(), f)
    cl = t.cellEM012ClusterLink()
    print >> f, '\n        cluster:',
    if cl.isValid():
        print >> f, cl.index(), cl.dataID(),
    else:
        print >> f, '(invalid)'
    print >> f, '\n        secvertex:',
    if t.secVtx():
        dump_RecVertex (t.secVtx(), f)
    else:
        print >> f, None,
    return


def dump_vvelccell (c, f):
    print >> f, '[',
    for v in c:
        print >> f, '[',
        for el in v:
            if el.isValid():
                print >> f, '(', el.dataID(), el.index(), ') ',
            else:
                print >> f, '(invEL) ',
        print >> f, ']',
    print >> f, ']',
    return

    
def dump_vvdouble (c, f):
    print >> f, '[',
    for v in c:
        print >> f, '[',
        for d in v:
            print >> f, d, ',',
        print >> f, ']',
    print >> f, ']',
    return

    
def dump_TauCommonExtraDetails (t, f):
    dump_TauDetail (t, f)
    print >> f, t.sumPtLooseTrk(), t.sumPtTrk(), t.seedCalo_nEMCell(), \
          t.seedCalo_stripEt(), t.seedCalo_EMCentFrac(), \
          t.seedCalo_sumCellEnergy(), t.seedCalo_sumEMCellEnergy(),
    print >> f, '\n    ',
    dump_vvelccell (t.closestEtaTrkVertCell(), f)
    print >> f, '\n        ',
    dump_vvelccell (t.closestPhiTrkVertCell(), f)
    print >> f, '\n        ',
    dump_vvelccell (t.closestEtaTrkCell(), f)
    print >> f, '\n        ',
    dump_vvelccell (t.closestPhiTrkCell(), f)
    print >> f, '\n        ',
    dump_vvdouble (t.etaTrkCaloSamp(), f)
    print >> f, '\n        ',
    dump_vvdouble (t.phiTrkCaloSamp(), f)
    print >> f, '\n        ',
    dump_vvdouble (t.etaLooseTrkCaloSamp(), f)
    print >> f, '\n        ',
    dump_vvdouble (t.phiLooseTrkCaloSamp(), f)
    return


def dump_TauJet (t, f):
    dump_ParticleImpl (t, f)
    print >> f, ' %1d' % (t.author(),),
    # xxx numberOfTracks doesn't seem to be initialized in reco --- don't dump.
    if t.clusterLink().isValid():
        print >>f,  '\n      cl: %2d:' % t.clusterLink().index(),
        dump_Fourvec (t.cluster(), f)
    if t.cellClusterLink().isValid():
        # Note: cluster container is unsorted.  Means that indices
        # are unstable.  So don't print them.
        print >>f,  '\n      ce: ',
        dump_Fourvec (t.cellCluster(), f)
    if t.jetLink().isValid():
        print >>f,  '\n      je: %2d:' % t.jetLink().index(),
        j = t.jet()
        if j.e() == 0 and j.phi() == 0 and j.eta() == 0:
            with signalstate (j, PyAthena.P4SignalState.JETEMSCALE):
                dump_Fourvec (j, f)
        else:
            dump_Fourvec (j, f)
    for i in range (t.numTrack()):
        el = t.trackLinkVector()[i]
        if el.isValid():
            print >>f,  '\n      t%d: %2d:' % (i,el.index()),
            dump_Fourvec (t.track(i), f)
    pid = t.tauID()
    TJP = PyAthena.TauJetParameters
    if pid:
        print >>f,  '\n      pid: %f %f %f %d %d %d' % \
              (pid.discriminant(0),
               pid.discriminant(1),
               pid.discriminant(2),
               pid.flag (TJP.ElectronFlag),
               pid.flag (TJP.EgammaFlag),
               pid.flag (TJP.MuonFlag)),
        if hasattr (TJP, 'TauCutLoose'):
            flags = [TJP.TauCutLoose,
                     TJP.TauCutMedium,
                     TJP.TauCutTight,
                     TJP.ElectronVeto,
                     TJP.MuonVeto,
                     TJP.TauLlhTight,
                     TJP.TauLlhMedium,
                     TJP.TauLlhLoose,
                     ]
            if hasattr (TJP, 'TauRecOld'):
                flags.append (TJP.TauRecOld)
            if hasattr (TJP, 'Tau1P3POld'):
                flags.append (TJP.Tau1P3POld)
            print >>f,  '\n          ', [pid.isTau(ff) for ff in flags],
        else:
            print >>f,  '\n          ', pid.isTau(),
    for i in range (t.nDetails()):
        print >> f, '\n      ',
        if t.detailElementLink(i).isValid():
            d = t.detail(i)
            if (hasattr (PyAthena.Analysis, 'Tau1P3PDetails') and
                isinstance (d, PyAthena.Analysis.Tau1P3PDetails)):
                dump_Tau1P3PDetails (d, f)
            elif (hasattr (PyAthena.Analysis, 'TauRecDetails') and
                  isinstance (d, PyAthena.Analysis.TauRecDetails)):
                dump_TauRecDetails (d, f)
            elif (hasattr (PyAthena.Analysis, 'Tau1P3PExtraDetails') and
                  isinstance (d, PyAthena.Analysis.Tau1P3PExtraDetails)):
                dump_Tau1P3PExtraDetails (d, f)
            elif (hasattr (PyAthena.Analysis, 'TauRecExtraDetails') and
                  isinstance (d, PyAthena.Analysis.TauRecExtraDetails)):
                dump_TauRecExtraDetails (d, f)
            elif (hasattr (PyAthena.Analysis, 'TauPi0Details') and
                  isinstance (d, PyAthena.Analysis.TauPi0Details)):
                dump_TauPi0Details (d, f)
            elif isinstance (d, PyAthena.Analysis.TauCommonDetails):
                dump_TauCommonDetails (d, f)
            elif isinstance (d, PyAthena.Analysis.TauCommonExtraDetails):
                dump_TauCommonExtraDetails (d, f)
            else:
                print >> f, t.detailName(i), d, '(not dumped)',
    print >> f, '\n      p: %f %f %f' % \
          (t.parameter (0),
           t.parameter (3),
           t.parameter (6)),
    return


def dump_BaseTagInfo (info, f):
    print >> f, ' '.join (['%f'%x for x in info.tagLikelihood()]), \
          info.weight(), info.isValid(),
    return


def dump_TruthInfo (info, f):
    dump_BaseTagInfo (info, f)
    print >> f, ' %s %f %d ' % \
          (info.jetTruthLabel(),
           info.deltaRMinTo ('b'),
           info.BHadronPdg()),
    dump_Threevec (info.BDecayVertex(), f)
    return


def dump_SLTrueInfo (info, f):
    print >> f, '%d %d %d %d %d %d ' % \
          (info.barcode(),
           info.pdgId(),
           info.pdgIdMother(),
           info.FromB(),
           info.FromD(),
           info.FromGH(),),
    dump_H3V (info.momentum(), f)
    print >> f, ' ',
    dump_H3V (info.prodvtx(), f)
    return

def dump_SoftLeptonTruthInfo (info, f):
    dump_BaseTagInfo (info, f)
    for i in range (info.numSLTrueInfo()):
        print >> f, '\n        ',
        dump_SLTrueInfo (info.getSLTrueInfo(i), f)
    return


def dump_SecVtxInfo (info, f):
    dump_BaseTagInfo (info, f)
    print >> f, '\n        %d %d %d %d %f %f %f %f %f' % \
          (info.fitType(),
           info.numSelTracksForFit(),
           info.mult(),
           info.NumberOfG2TrackVertices(),
           info.distance(),
           info.rphidistance(),
           info.probability(),
           info.mass(),
           info.energyFraction()),
    return dump_Threevec (info.secVertexPos().position(), f)


def dump_IPTrackInfo (info, f):
    dump_Fourvec (info.track(), f)
    print >> f, ' %d %s %d  %f %f %f %f %f %f %f' % \
          (info.trackGrade().gradeNumber(),
           info.trackGrade().gradeString(),
           info.isFromV0(),
           info.d0Value(),
           info.d0Significance(),
           info.z0Value(),
           info.z0Significance(),
           info.trackWeight2D(),
           info.trackWeight3D(),
           info.trackProbJP()),
    return

def dump_IPInfoPlus (info, f):
    dump_BaseTagInfo (info, f)
    print >> f, ' %d' % info.numTrackInfo(),
    for i in range(info.numTrackInfo()):
        print >> f, '\n        ',
        dump_IPTrackInfo (info.getTrackInfo(i), f)
    return


def dump_IPInfoBase (info, f):
    dump_BaseTagInfo (info, f)
    print >> f, info.nbTracks(),
    return


def dump_SVInfoBase (info, f):
    dump_BaseTagInfo (info, f)
    return


def dump_SVTrackInfo(info, f):
    tp = info.track()
    if not tp:
        print tp,
    else:
        dump_Fourvec (tp, f)
    return

    
def dump_SVInfoPlus (info, f):
    dump_BaseTagInfo (info, f)
    print >> f, info.getNGTrackInJet(), \
          info.getNGTrackInSvx(), \
          info.getN2T(), \
          info.getMass(), \
          info.getEnergyFraction(), \
          info.getNormDist(), \
          info.numTrackInfo(),
    print >> f, '\n    vert: ',
    dump_RecVertex (info.getRecSvx(), f)
    print >> f, '\n    tracks: ',
    # Track infos are unsorted.
    # Sort on pt for printing.
    ti = [info.getTrackInfo(i) for i in range(info.numTrackInfo())]
    ti.sort (cmp=lambda a, b: cmp(b.track().pt(), a.track().pt()))
    for tt in ti:
        print >> f, '\n      %d: ' % i,
        dump_SVTrackInfo (tt, f)
    return


def dump_MultiSVInfoPlus (info, f):
    dump_BaseTagInfo (info, f)
    print >> f, info.getNGTrackInJet(), \
          info.getNGTrackInSvx(), \
          info.getN2T(), \
          info.getNormDist(), \
          info.numVtxInfo(),
    print >> f, '\n    verts: ',
    for i in range (info.numVtxInfo()):
        print >> f, '\n    %d: ' % i,
        vx = info.getVtxInfo(i)
        print >> f, vx.getMass(), vx.getPt(), vx.getEta(), vx.getPhi(),\
              vx.getEnergyFraction(), vx.getNormDist(), vx.numTrackInfo(),
        print >> f, '\n      vert: ',
        dump_RecVertex (vx.getRecSvx(), f)
        print >> f, '\n      tracks: ',
        # Track infos are unsorted.
        # Sort on pt for printing.
        ti = [vx.getTrackInfo(i) for i in range(vx.numTrackInfo())]
        ti.sort (cmp=lambda a, b: cmp(b.track().pt(), a.track().pt()))
        for tt in ti:
            print >> f, '\n      %d: ' % i,
            dump_SVTrackInfo (tt, f)
    return


def dump_JetProbInfoBase (info, f):
    dump_BaseTagInfo (info, f)
    print >> f, info.nbTracks(),
    return


def dump_SETrackInfo (info, f):
    dump_Fourvec (info.electron(), f)
    print >> f, '%f %f' % (info.d0Value(), info.pTrel()),\
          [x for x in info.tagLikelihood()],
    return


def dump_SoftElectronInfo (info, f):
    dump_BaseTagInfo (info, f)
    print >> f, ' '.join(['%f'%x for x in info.vectorTrackProb()]), \
          info.nTrackProb(), info.numTrackInfo(),
    for i in range(info.numTrackInfo()):
        print >> f, '\n        ',
        dump_SETrackInfo (info.getTrackInfo(i), f)
    return


def dump_JetFitterTagInfo (info, f):
    dump_BaseTagInfo (info, f)
    print >> f, '%d %d %d %f %f %f %f %f' % \
          (info.nVTX(),
           info.nSingleTracks(),
           info.nTracksAtVtx(),
           info.energyFraction(),
           info.mass(),
           info.significance3d(),
           info.deltaphi(),
           info.deltaeta()),
    return


def dump_TrackCountingInfo (info, f):
    dump_BaseTagInfo (info, f)
    print >> f, '%d %f %f %f %f' % \
          (info.nTracks(),
           info.d0sig_2nd(),
           info.d0sig_abs_2nd(),
           info.d0sig_3rd(),
           info.d0sig_abs_3rd()),
    return


def dump_AtlfInfo (info, f):
    dump_BaseTagInfo (info, f)
    print >> f, info.isBTagged(), info.isTauTagged(), info.isTau1PTagged(), \
          info.isTau3PTagged(), info.LightHypoCalFactor(), \
          info.TauHypoCalFactor(), info.Tau1P3PHypoCalFactor(), \
          info.BHypoCalFactor(), \
          info.deltaRMinTo("b"), \
          info.deltaRMinTo("c"), \
          info.deltaRMinTo("t"),
    return


def dump_SMTrackInfo (info, f):
    dump_ParticleImpl (info.muon(), f)
    print >> f, info.d0Value(), info.pTrel(), list(info.tagLikelihood()),
    return


def dump_SoftMuonInfo (info, f):
    dump_BaseTagInfo (info, f)
    for i in range (info.numTrackInfo()):
        print >> f, "      ti ",
        dump_SMTrackInfo (info.getTrackInfo(i), f)
    return


def dump_GbbNNTagInfo (info, f):
    dump_BaseTagInfo (info, f)
    print >> f, info.nMatchingTracks(), info.trkJetWidth(), \
          info.trkJetMaxDeltaR(),
    return


def dump_JetConstituent (info, f):
    print >> f, [info.energyInSample(i) for i in range(8)],\
         info.energyInCryostat(), info.wtCryo(), info.jet(),
#       const Jet* jet() const; //!< get jet directly without token      
#       double energyInSample(CaloSampling::CaloSample ) const;
#       double energyInCryostat( ) const;
#       double getJetWeight(const Jet* the_jet) const ;
#       double getJetWeight(const JetCollection* theContainer,
# 			  index_type& theIndex) const;
#       double wtCryo () const { return m_wtCryoLocal; }
    return


def dump_TrackConstituents (info, f):
    tr = info.tracks()
    ROOT.SetOwnership (tr, True)
    for t in tr:
        print >> f, '\n          tr: ',
        dump_Fourvec (t, f)
        print >> f, info.getTrackWeight(t),
    return


def dump_ElectronConstituent (info, f):
    if info.electron():
        dump_Fourvec (info.electron(), f)
        print >> f, info.getElectronWeight (info.electron()),
    else:
        print >> f, None,
    return


def dump_MuonConstituent (info, f):
    if info.muon():
        dump_Fourvec (info.muon(), f)
        print >> f, info.getMuonWeight (info.muon()),
    else:
        print >> f, None,
    return

def dump_ParticleJet (j, f):
    dump_ParticleBase (j, f)
    print >> f, '\n      %s %f %f' % \
          (j.jetAuthor(),
           j.weight(),
           j.lhSig()),
    print >> f, [x for x in j.combinedLikelihood()],
    ti = j.jetTagInfoVector()
    for info in ti:
        print >> f, '\n      %s: ' % info.infoType(),
        if isinstance (info, PyAthena.Analysis.TruthInfo):
            dump_TruthInfo (info, f)
        elif isinstance (info, PyAthena.Analysis.SoftLeptonTruthInfo):
            dump_SoftLeptonTruthInfo (info, f)
        elif isinstance (info, PyAthena.Analysis.SecVtxInfo):
            dump_SecVtxInfo (info, f)
        elif isinstance (info, PyAthena.Analysis.IPInfoPlus):
            dump_IPInfoPlus (info, f)
        elif isinstance (info, PyAthena.Analysis.IPInfoBase):
            dump_IPInfoBase (info, f)
        elif isinstance (info, PyAthena.Analysis.SVInfoBase):
            dump_SVInfoBase (info, f)
        elif isinstance (info, PyAthena.Analysis.JetProbInfoBase):
            dump_JetProbInfoBase (info, f)
        elif isinstance (info, PyAthena.Analysis.SoftElectronInfo):
            dump_SoftElectronInfo (info, f)
        elif isinstance (info, PyAthena.Analysis.JetFitterTagInfo):
            dump_JetFitterTagInfo (info, f)
        elif isinstance (info, PyAthena.Analysis.BaseTagInfo):
            dump_BaseTagInfo (info, f)
        else:
            print >> f, info,

    for c in j.constituentKeys():
        print >> f, '\n      %s: ' % c,
        info = j.constituent (c)
        if isinstance (info, PyAthena.Analysis.JetConstituent):
            dump_JetConstituent (info, f)
        elif isinstance (info, PyAthena.Analysis.TrackConstituents):
            dump_TrackConstituents (info, f)
        elif isinstance (info, PyAthena.Analysis.ElectronConstituent):
            dump_ElectronConstituent (info, f)
        elif isinstance (info, PyAthena.Analysis.MuonConstituent):
            dump_MuonConstituent (info, f)
        else:
            print >> f, info,
    return


def dump_TruthParticle (p, f):
    dump_ParticleImpl (p, f)
    poltheta = p.polarization().theta()
    if poltheta == 0: poltheta = 0  # Fold -0 -> 0
    print >> f, ' %3d %3d %f %f %d %d' % \
          (p.barcode(),
           p.status(),
           poltheta,
           p.polarization().phi(),
           p.nParents(),
           p.nDecay()),
    print >> f, '\n        ', p.hasEtIsol(),
    dump_HLV (p.genParticle().momentum(), f)
    if p.nParents()>0:
        print >> f, '\n        p0:',
        # Note: ordering of entries in the parent/child lists is unpredictable.
        # Here, we'll pick the one with the smallest barcode.
        mother = None
        for i in range(p.nParents()):
            if not mother or p.genMother(i).barcode() < mother.barcode():
                mother = p.genMother(i)
                mni = i
        dump_HLV (mother.momentum(), f)
        dump_Fourvec (p.mother(mni), f)
    if p.nDecay()>0:
        print >> f, '\n        c0:',
        child = None
        for i in range(p.nDecay()):
            if not child or p.genChild(i).barcode() < child.barcode():
                child = p.genChild(i)
                mni = i
        dump_HLV (child.momentum(), f)
        dump_Fourvec (p.child(mni), f)
    return


def dump_TrackParticleTruth (p, f):
    print >> f, '%4d %f' % (p.barcode(), p.probability()),
    gp = p.particleLink().cptr()
    if gp:
        dump_HLV (gp.momentum(), f)
    else:
        print >> f, ' (no part)',
    return


@nolist
def dump_TrackParticleTruthCollection (c, f):
    print >> f, ' ', c.trackParticleContainerLink().isValid()
    elclass = PyAthena.ElementLink (PyAthena.Rec.TrackParticleContainer)
    for i in range (c.size()):
        print >> f, '  ',
        el = elclass()
        cont = PyAthena.Rec.TrackParticleContainer()
        el.toIndexedElement (cont, i)
        dump_TrackParticleTruth (c[PyAthena.Rec.TrackParticleTruthKey(el)], f)
        f.write ('\n')
    return


def dump_SubDetHitStatistics (s, f):
    print >> f, [s[i] for i in range(s.NUM_SUBDETECTORS)],
    return


def dump_TruthTrajectory (t, f):
    print >> f, [l.barcode() for l in t],
    return


@nolist
def dump_DetailedTrackTruthCollection (c, f):
    print >> f, ' ', c.trackCollectionLink().key(), c.size(),
    acls=PyAthena.PyDumper.PySTLAdaptor(PyAthena.DetailedTrackTruthCollection)
    for p in toiter1(acls(c)):
        print >>f, '\n   %3d' % p.first.index(), ' common ',
        dump_SubDetHitStatistics (p.second.statsCommon(), f),
        print >>f, '\n      ', ' track ',
        dump_SubDetHitStatistics (p.second.statsTrack(), f),
        print >>f, '\n      ', ' truth ',
        dump_SubDetHitStatistics (p.second.statsTruth(), f),
        print >>f, '\n      ', ' trajectory ',
        dump_TruthTrajectory (p.second.trajectory(), f),
    return
              
        



def dump_FitQuality (info, f):
    print >> f, "%f %d" % \
          (info.chiSquared(),
           info.numberDoF()),
    return


def dump_TrackSummary (info, f):
    print >> f, [info.get(i) for i in range(32)], \
          [info.isHit(i) for i in range(21)],
    return


def dump_Surface (info, f):
    dump_Threevec (info.center(), f)
    dump_Threevec (info.normal(), f)
    dump_Threevec (info.globalReferencePoint(), f)
    print >> f, '\n          tf',
    dump_AmgMatrix (info.transform().matrix(), f)
#    print >> f, '\n          de', info.associatedDetectorElement(),
    print >> f, '\n          ly', tonone (info.associatedLayer()),
    print >> f, '\n          bd', info.bounds().__class__.__name__,
    print >> f, '\n          id', \
          info.associatedDetectorElementIdentifier().getString(),
    return

def dump_PerigeeSurface (info, f):
    dump_Surface (info, f)
    return


def dump_PlaneSurface (info, f):
    dump_Surface (info, f)
    return


def dump_StraightLineSurface (info, f):
    dump_Surface (info, f)
    return




def dump_ParametersBase (info, f):
    dump_AmgVector (info.parameters(), f)
    dump_Threevec (info.momentum(), f)
    print >> f, info.pT(), info.eta(),
    dump_Threevec (info.position(), f)
    dump_Twovec (info.localPosition(), f)
    print >> f, "%f" % (info.charge(),),
    if info.covariance():
        print >> f, '\n          cov',
        dump_AmgMatrix (info.covariance(), f)
    print >> f, '\n          sf',
    if not info.associatedSurface():
        print >> f, None,
    elif isinstance (info.associatedSurface(), PyAthena.Trk.PerigeeSurface):
        dump_PerigeeSurface (info.associatedSurface(), f)
    elif isinstance (info.associatedSurface(), PyAthena.Trk.PlaneSurface):
        dump_PlaneSurface (info.associatedSurface(), f)
    elif isinstance (info.associatedSurface(), PyAthena.Trk.StraightLineSurface):
        dump_StraightLineSurface (info.associatedSurface(), f)
    else:
        print >> f, info.associatedSurface(),
    return


def dump_parameters (p, f):
    if not p:
        print >> f, '(nil)',
    elif (p.__class__.__name__.startswith ('Trk::ParametersT<') or
        p.__class__.__name__.startswith ('Trk::ParametersBase<')):
        dump_ParametersBase (p, f)
    else:
        print >> f, p,
    return


def dump_TrackParticle (p, f):
    dump_Fourvec (p, f)
    print >> f, "%f %1d" % \
          (p.charge(),
           p.particleOriginType()),
    # This crashes --- needs dd.
    #if p.originalTrack():
    #    print >> f, '\n        tr', p.originalTrack(),
    print >> f, '\n        vx',
    vx = p.reconstructedVertex()
    if vx:
        dump_Threevec (vx.recVertex().position(), f)
    else:
        print >> f, tonone (vx),
    if p.fitQuality():
        print >> f, '\n        fq',
        dump_FitQuality (p.fitQuality(), f)
    if p.trackSummary():
        print >> f, '\n        ts',
        dump_TrackSummary (p.trackSummary(), f)
    print >> f, '\n        df',
    dump_parameters (p.definingParameters(), f)
    print >> f, '\n        pm',
    for x in p.trackParameters():
        print >> f, '\n'
        dump_parameters (x, f)
    return


@nolist
def dump_MissingET (m, f):
    print >> f, '%d %f %f %f %f %f' % \
          (m.getSource(),
           m.etx(),
           m.ety(),
           m.sumet(),
           m.et(),
           m.phi()),
    r = m.getRegions()
    if r:
        for i in range(3):
            print >> f, '\n   %d %f %f %f' % \
                  (i,
                   r.exReg(i),
                   r.eyReg(i),
                   r.etSumReg(i)),
    return


@nolist
def dump_MissingETSig (m, f):
    print >> f, "%f %f" % \
          (m.metx(),
           m.mety()),
    print >> f, "\n  %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f" % \
          (m.pL()     ,
           m.sigL()   ,
           m.sL()     ,
           m.d0varL() ,
           m.pT()     ,
           m.sigT()   ,
           m.sT()     ,
           m.d0varT() ,
           m.pA()     ,
           m.sigA()   ,
           m.sA()     ,
           m.d0varA() ,
           m.pB()     ,
           m.sigB()   ,
           m.sB()     ,
           m.d0varB()),
    print >> f, "\n  %f %f %f %f %f %f %f %f" % \
          (m.phiA(),
           m.phiAall() ,
           m.tA()      ,
           m.tAall()   ,
           m.phiB()    ,
           m.phiBall() ,
           m.tB()      ,
           m.tBall()),
    print >> f, "\n  %f %f %f %f" % \
          (m.met(),
           m.phi(),
           m.rho(),
           m.oblat()),
    return


def dump_MissingETSigHypo (h, f):
    if not h:
        print >> f, h,
        return
    print >> f, '\n    ',
    dump_Fourvec (h, f)
    print >> f, "%d %d %d %d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %d %d %d" % \
          (h.isLinkValid(),
           h.useLinkForNav(),
           h.type(),
           h.getType().type(),
           h.eRaw(),
           h.etaRaw(),
           h.phiRaw(),
           h.mRaw(),
           h.e(),
           h.eta(),
           h.phi(),
           h.m(),
           h.exMiss(),
           h.eyMiss(),
           h.etMiss(),
           h.phiMiss(),
           h.ex(),
           h.ey(),
           h.ez(),
           h.scale(),
           h.getConeSize(),
           h.sumEt(),
           h.getReg(),
           h.getReg(0.5),
           h.isEmpty(),
           ),
    if h.getObject():
        print >> f, '\n     ',
        dump_Fourvec (h.getObject(), f),
    for r in range(3):
        print >> f, '\n     %d %f %f %f %f %f' % \
              (r,
               h.sumEt(r),
               h.exMiss(r),
               h.eyMiss(r),
               h.etMiss(r),
               h.phiMiss(r)),
    print >> f, '\n     ',  [x for x in toiter1(h)],
    return


def dump_MissingETSigObject (m, f):
    dump_Fourvec (m, f)
    print >> f, " %d %d %d %d %d %d %d %d %d %f %f %f %f %f %f %f %f %f %f %f " % \
          (m.isValidDefault(),
           m.isValidCurrent(),
           m.isValid(),
           m.isDefault(),
           m.getType().type(),
           m.getDefaultType().type(),
           m.defaultType(),
           m.type(0),
           m.getType(0).type(),
           m.eRaw(),
           m.etaRaw(),
           m.phiRaw(),
           m.mRaw(),
           m.e(),
           m.eta(),
           m.phi(),
           m.m(),
           m.ex(),
           m.ey(),
           m.ez()),
    print >> f, '\n  %f %f %f %f %f %f %f %d %d %d' %\
           (m.exMiss(),
            m.eyMiss(),
            m.etMiss(),
            m.phiMiss(),
            m.scale(),
            m.getConeSize(),
            m.sumEt(),
            m.numHypos(),
            m.idxDefault(),
            m.idxCurrent()),
    if m.isValid():
        print >> f, '\n  ', [i for i in m.types()],
    for r in range (3):
        print >> f, '\n  %d %f %f %f %f %f' %\
              (r,
               m.sumEt(r),
               m.exMiss(r),
               m.eyMiss(r),
               m.etMiss(r),
               m.phiMiss(r)),
    print >> f, '\n  ', \
          [x for x in toiter1 (m.getExcess())],\
          [x for x in toiter1 (m.getDeficit())],
    dump_MissingETSigHypo (m.getHypo(), f)
    dump_MissingETSigHypo (m.getDefaultHypo(), f)
    dump_MissingETSigHypo (m.getHypo(0), f)
    return


@nolist
def dump_MissingEtCalo (m, f):
    dump_MissingET (m, f)
    print >> f, '\n   ', m.calibType(),
    for i in range(7):
        print >> f, '\n   %d %d %f %f %f' % \
              (i,
               m.ncellCalo(i),
               m.exCalo(i),
               m.eyCalo(i),
               m.etSumCalo(i)),
    return


@nolist
def dump_MissingEtTruth (m, f):
    dump_MissingET (m, f)
    for i in range(6):
        print >> f, '\n   %d %f %f %f' % \
              (i,
               m.exTruth(i),
               m.eyTruth(i),
               m.etSumTruth(i)),
    return


def dump_CaloCluster (c, f):
    # ??? cells, getNumberOfCells
    dump_Fourvec (c, f)
    print >> f, "%3d %f %f %f %f %f %d %d %d" % \
          (c.getClusterSize(),
           c.getBasicEnergy(),
           c.getTime(),
           c.eta0(),
           c.phi0(),
           c.energy(),
           c.inBarrel(),
           c.inEndcap(),
           c.allLocked()),
    print >> f, c.getRecoStatus().getStatusWord(),
    beg = c.beginMoment()
    end = c.endMoment()
    while beg != end:
        print >> f, '\n      mom %d %f' % (c.getMomentType(beg),
                                           c.getMomentValue(beg)),
        beg.__preinc__()
        
    for i in range (c.nSamples()):
        print >> f, '\n      %2d %d %d %d %f %f %f %f %f %f %f %f %f %f %f' % \
              (i,
               c.is_valid_sampling(i),
               c.isEMSampling(i),
               c.hasSampling(i),
               c.eSample(i),
               c.etaSample(i),
               c.phiSample(i),
               c.etasize(i),
               c.phisize(i),
               c.energy_max(i),
               c.etamax(i),
               c.phimax(i),
               c.etaBE(i),
               c.phiBE(i),
               c.energyBE(i)),
    return


@nolist
def dump_CaloClusters_sorted (l, f):
    ll = [x for x in l]
    ll.sort (cmp=lambda a, b: cmp(b.pt(), a.pt()))
    dump_list (ll, f, dump_CaloCluster)
    return


def dump_Vertex (v, f):
    dump_Threevec (v.position(), f)
    return


def dump_RecVertex (v, f):
    dump_Vertex (v, f)
    dump_FitQuality (v.fitQuality(), f)
    dump_AmgMatrix (v.covariancePosition(), f) # eigen
    return


def dump_ITrackLink (l, f):
    dump_parameters (l.parameters(), f)
    return


def dump_LinkToTrack (l, f):
    dump_ITrackLink (l, f)
    print >> f, '\n             ', l.isValid(), l.index(), l.dataID(),
    return


def dump_LinkToTrackParticleBase (l, f):
    dump_ITrackLink (l, f)
    print >> f, l.isValid(), l.index(), l.dataID(),
    return


def dump_VxTrackAtVertex (t, f):
    dump_FitQuality (t.trackQuality(), f),
    print >> f, '%f %f' % \
          (t.vtxCompatibility(),
           t.weight()),
    print >> f, tonone(t.linState()), tonone(t.ImpactPoint3dAtaPlane()),
    if t.perigeeAtVertex():
        print >> f, '\n      pv',
        dump_parameters (t.perigeeAtVertex(), f)

    # Methods below will try to dereference the link given by
    # trackOrParticleLink, which is an EL to either a Trk::Track
    # or a TrackParticle.  TrackParticle we can handle, but we
    # shouldn't try to dereference an EL to a Track, since ARA
    # can't handle that.
    tel = PyAthena.ElementLink ('DataVector<Trk::Track>')
    if not isinstance (t.trackOrParticleLink(), tel):
        print >> f, '\n      ip',
        dump_parameters (t.initialPerigee(), f)
        print >> f, '\n      pl',
        if isinstance (t.trackOrParticleLink(), PyAthena.Trk.LinkToTrack):
            dump_LinkToTrack (t.trackOrParticleLink(), f)
        elif isinstance (t.trackOrParticleLink(),PyAthena.Trk.LinkToTrackParticleBase):
            dump_LinkToTrackParticleBase (t.trackOrParticleLink(), f)
        else:
            print >> f, t.trackOrParticleLink(),
    return


def dump_MVFVxTrackAtVertex (t, f):
    dump_VxTrackAtVertex (t, f)
    print >> f, '\n      ', tonone(t.linkToVertices()),
    return

    
def dump_VxCandidate1 (v, f):
    dump_RecVertex (v.recVertex(), f)
    for t in v.vxTrackAtVertex():
        print >> f, '\n    ', t.__class__.__name__,
        if isinstance (t, PyAthena.Trk.MVFVxTrackAtVertex):
            dump_MVFVxTrackAtVertex (t, f)
        elif t.__class__ == PyAthena.Trk.VxTrackAtVertex:
            dump_VxTrackAtVertex (t, f)
        else:
            print >> f, tonone(t),
    return


def dump_MvfFitInfo (v, f):
    print >> f, tonone(v.constraintVertex()), \
          tonone(v.seedVertex()), \
          tonone(v.linearizationVertex()),
    return


def dump_MVFVxCandidate (v, f):
    dump_VxCandidate1 (v, f)
    print >> f, '\n      ', v.isInitialized(), 'fi',
    dump_MvfFitInfo (v.vertexFitInfo(), f)
    return


def dump_VxCandidate (v, f):
    print >> f, v.__class__.__name__,
    if isinstance (v, PyAthena.Trk.MVFVxCandidate):
        dump_MVFVxCandidate (v, f)
    elif (v.__class__ == PyAthena.Trk.VxCandidate or
          isinstance (v, PyAthena.Trk.V0Hypothesis)):
        dump_VxCandidate1 (v, f)
    elif isinstance (v, PyAthena.Trk.ExtendedVxCandidate):
          dump_ExtendedVxCandidate (v, f)
    else:
        print >> f, tonone(v),
        dump_VxCandidate1 (v, f)
    return


def dump_EventID (e, f):
    print >> f, '%9d %9d %9d %9d %9d %9d' % \
          (e.run_number(),
           e.event_number(),
           e.time_stamp(),
           e.time_stamp_ns_offset(),
           e.lumi_block(),
           e.bunch_crossing_id()),
    return


def dump_EventType (e, f):
    print >> f, e.typeToString(), e.user_type(), e.mc_event_weight(),
    return


def dump_TriggerInfo (e, f):
    print >> f, e.statusElement(), e.extendedLevel1ID(), e.level1TriggerType(),
    print >> f, '\n    l2:', [x for x in e.level2TriggerInfo()],
    print >> f, '\n    ef:', [x for x in e.eventFilterInfo()],
    print >> f, '\n    st:', [(x.name(), x.type(), x.obeysLumiblock()) for x in e.streamTags()],
    return


@nolist
def dump_EventInfo (e, f):
    print >> f, e.clID(),
    dump_EventID (e.event_ID(), f)
    print >> f, '\n   ',
    print >> f, e.actualInteractionsPerCrossing(), e.averageInteractionsPerCrossing(),
    for i in range(ROOT.EventInfo.nDets):
        print >> f, '\n     det', i, e.eventFlags(i), e.errorState(i),
    print >> f, '\n   ',
    dump_EventType (e.event_type(), f)
    print >> f, '\n   tg',
    dump_TriggerInfo (e.trigger_info(), f)
    return


@nolist
def dump_PileUpEventInfo (e, f):
    dump_EventInfo (e, f)
    for (i,s) in enumerate (toiter (e.beginSubEvt(), e.endSubEvt())):
        print >> f, '\n   subevt', i, s.time(), s.index(), s.BCID(), s.type(),
    return


def barcodes (beg, end, sz, f):
    #out = [x.barcode() for x in toiter (beg, end)]

    # Work around pyroot iterator comparison breakage.
    out = []
    while beg != end and sz > 0:
        sz -= 1
        x = beg.__deref__()
        beg.__preinc__()
        out.append (x.barcode())

    out.sort()
    for i in out:
        print >> f, i,
    return
def dump_GenVertex (v, f):
    print >> f, "%d %d %d %d %d" % \
          (v.id(),
           v.barcode(),
           v.parent_event().event_number(),
           v.particles_in_size(),
           v.particles_out_size()),
    dump_Threevec (v.point3d(), f)
    dump_HLV (v.position(), f)
    if v.weights().size() == 0:
        ww = []
    else:
        ww = list(v.weights())
    print >> f, [w for w in ww]
    print >> f, '(',
    barcodes(v.particles_in_const_begin(),
             v.particles_in_const_end(),
             v.particles_in_size(), f)
    print >> f, ')(',
    barcodes(v.particles_out_const_begin(),
             v.particles_out_const_end(),
             v.particles_out_size(), f)
    print >> f, ')',

    return


def dump_GenParticle (p, f):
    print >> f, "%d %d %d %d" % \
          (p.barcode(),
           p.pdg_id(),
           p.parent_event().event_number(),
           p.status(),),
    if p.production_vertex():
        print >> f, p.production_vertex().barcode(),
    else:
        print >> f, None,
    if p.end_vertex():
        print >> f, p.end_vertex().barcode(),
    else:
        print >> f, None,
    dump_HLV (p.momentum(), f)
    pol = p.polarization()
    poltheta = pol.theta()
    if poltheta == 0: poltheta = 0 # Fold -0 -> 0
    print >> f, "%f %f %f %f" % \
          (poltheta, pol.phi(), pol.normal3d().theta(), pol.normal3d().phi()),
    return


def dump_GenEvent (e, f):
    print >> f, '%d %d %f %f %f %d %d' % \
          (e.event_number(),
           e.signal_process_id(),\
           e.event_scale(), \
           e.alphaQCD(),
           e.alphaQED(),
           e.particles_size(),
           e.vertices_size()),
    print >> f, '\n   wt', [w for w in e.weights()], [i for i in e.random_states()],
    print >> f, '\n   v0',
    if e.signal_process_vertex():
        dump_GenVertex (e.signal_process_vertex(), f)
    else:
        print >> f, None,
    for v in toiter (e.vertices_begin(), e.vertices_end()):
        print >> f, '\n   v',
        dump_GenVertex (v, f)

    for p in toiter (e.particles_begin(), e.particles_end()):
        print >> f, '\n   p',
        if p:
            dump_GenParticle (p, f)
        else:
            print >> f, None,
    return


@nolist
def dump_CTP_Decision (t, f):
    print >> f, t.dump(),
    return


def dump_Muon_ROI (m, f):
    dump_Fourvec (m, f)
    print >> f, "%d %s %f %d %d %d %d %d %d %d %d" % \
          (m.getROIWord(),
           m.getThrName(),
           m.getThrValue(),
           m.getThrNumber(),
           m.getRoI(),
           m.getSectorAddress(),
           m.isFirstCandidate(),
           m.isMoreCandInRoI(),
           m.isMoreCandInSector(),
           m.getSource(),
           m.getHemisphere()),
    return


def dump_EmTau_ROI (m, f):
    dump_Fourvec (m, f)
    print >> f, "%d %f %f %f %f %f %f %d" % \
          (m.getROIWord(),
           m.getCore(),
           m.getEMClus(),
           m.getTauClus(),
           m.getEMIsol(),
           m.getHadIsol(),
           m.getHadCore(),
           m.getThrPattern()),
    for (n,v) in zip (m.getThresholdNames(), m.getThresholdValues()):
        print >> f, n, v,
    return


def dump_Jet_ROI (m, f):
    dump_Fourvec (m, f)
    print >>f, "%d %d %f %f %f" % \
          (m.getROIWord(),
           m.getThrPattern(),
           m.getET4x4(),
           m.getET6x6(),
           m.getET8x8()),
    for (n,v) in zip (m.getThresholdNames(), m.getThresholdValues()):
        print >> f, n, v,
    return


def dump_JetET_ROI (m, f):
    print >>f, "%d %d" % \
          (m.getROIWord(),
           m.getThrPattern(),),
    print >> f, [t for t in m.getThresholds()],
    return


def dump_EnergySum_ROI (m, f):
    print >> f, "%d %d %d %f %f %f %f %f %d %d %d %d %d %d" % \
          (m.getROIWord0(),
           m.getROIWord1(),
           m.getROIWord2(),
           m.getEnergyX(),
           m.getEnergyY(),
           m.getEnergyT(),
           m.getExMiss(),
           m.getEyMiss(),
           m.getOverflowX(),
           m.getOverflowY(),
           m.getOverflowT(),
           m.getOverflows(),
           m.getThrPatSummET(),
           m.getThrPatMissET()),
    print >> f, [t for t in m.getThresholds()],
    return


@nolist
def dump_LVL1_ROI (t, f):
    for m in t.getMuonROIs():
        print >> f, '\nmu',
        dump_Muon_ROI (m, f)
    for m in t.getEmTauROIs():
        print >> f, '\nem',
        dump_EmTau_ROI (m, f)
    for m in t.getJetROIs():
        print >> f, '\njt',
        dump_Jet_ROI (m, f)
    for m in t.getJetEtROIs():
        print >> f, '\nje',
        dump_JetET_ROI (m, f)
    for m in t.getEnergySumROIs():
        print >> f, '\nes',
        dump_EnergySum_ROI (m, f)
    return


def dump_GenericResult (t, f):
    print >> f, "%d %d %d" % \
          (t.accepted(),
           t.error(),
           t.error_bits(),),
    return


@nolist
def dump_HLTResult (t, f):
    dump_GenericResult (t, f)
    print >> f, "%d %d %d %d %d %d %d %d %d %d %d %d" % \
          (t.getHLTResultClassVersion(),
           t.getLvl1Id(),
           t.isPassThrough(),
           t.getHLTStatus().code,
           t.getLvlConverterStatus().code,
           t.getHLTLevel(),
           t.isValid(),
           t.getNumOfSatisfiedSigs(),
           t.isCreatedOutsideHLT(),
           t.isHLTResultTruncated(),
           t.size(),
           t.isEmpty(),
           ),
    print >> f, [i for i in t.partSizes()],
    print >> f, [i for i in t.getNavigationResult()],
    print >> f, [i for i in t.getChainResult()],
    print >> f, [i for i in t.getNavigationResultCuts()],
    return


def dump_Lvl1Result (t, f):
    print >> f, t.isConfigured(), \
          t.isAccepted(), \
          t.anyActiveL1ItemAfterVeto(), \
          t.nItems(),
    print >> f, '\n   ', [i for i in t.itemsPassed()],
    print >> f, '\n   ', [i for i in t.itemsBeforePrescale()],
    print >> f, '\n   ', [i for i in t.itemsAfterPrescale()],
    print >> f, '\n   ', [i for i in t.itemsAfterVeto()],
    return


@nolist
def dump_TrigDecision (t, f):
    print >> f, t.masterKey(),
    print >> f, '\nl1',
    dump_Lvl1Result (t.getL1Result(), f)
    print >> f, '\nl2',
    dump_HLTResult (t.getL2Result(), f)
    print >> f, '\nef',
    dump_HLTResult (t.getEFResult(), f)
    return


def dump_HLTTriggerElement (e, f):
    print >> f, e.id(), e.name(),
    return

    
def dump_HLTSignature (s, f):
    print >> f, '(', s.signature_counter(),\
          s.logic(),\
          s.label(),
    for e in s.outputTEs():
        dump_HLTTriggerElement (e, f)
    print >> f, ')',
    return


def dump_HLTStreamTag (s, f):
    print >> f, '(', \
          s.stream(),\
          s.type(),\
          s.obeyLB(),\
          s.prescale(),\
          ')',
    return

def dump_HLTChain (c, f):
    print >> f, c.chain_name(), \
          c.chain_version(), \
          c.chain_counter(), \
          c.level(), \
          c.lower_chain_name(), \
          c.lower_chain_counter(), \
          c.prescale(), \
          c.pass_through(), \
          c.chain_hash_id(), \
          c.lower_chain_hash_id(),
    for s in c.signatureList():
        dump_HLTSignature (s, f)
    print >> f, [t.bit() for t in c.triggerTypeList()],
    for s in c.streamTagList():
        dump_HLTStreamTag (s, f)
    print >> f, [s for s in c.groupList()],
    return


def dump_HLTSequence (s, f):
    print >> f, '(',
    dump_HLTTriggerElement (s.outputTE(), f)
    dump_HLTTriggerElement (s.topoStartTE(), f)
    for t in s.inputTEs():
        dump_HLTTriggerElement (t, f)
    print >> f, [a for a in s.algorithms()], ')',
    return


@nolist
def dump_HLTAODConfigData (d, f):
    print >> f, "%d" % \
          (d.masterKey()),
    for c in d.getChainList().chains():
        print >> f, '\nch',
        dump_HLTChain (c, f)
    for c in d.getSequenceList().sequences():
        print >> f, '\nsq',
        dump_HLTSequence (c, f)
    return


def dump_L1DataBaseclass (c, f):
    print >> f, c.id(),
    return


def dump_CTPConfig (c, f):
    dump_L1DataBaseclass (c, f)
    print >> f, c,
#     Menu* menu() const;
#     void setMenu( Menu* m );   
#     PrescaleSet prescaleSet() const;
#     void setPrescaleSet( PrescaleSet m );  
#     BunchGroupSet bunchGroupSet() const;
#     void setBunchGroupSet( BunchGroupSet m );  
#     PrescaledClock prescaledClock() const;
#     void setPrescaledClock( PrescaledClock m );   
#     TriggerType triggerType() const;
#     void setTriggerType( TriggerType m );   
#     DeadTime deadTime() const;
#     void setDeadTime( DeadTime m );   
#     Random random() const;
#     void setRandom( Random m );   
    
#     int lvl1MasterTableId() const {return m_Lvl1MasterTableId;}
#     void setLvl1MasterTableId (const int& id){m_Lvl1MasterTableId=id;}
    return


@nolist
def dump_Lvl1AODConfigData (d, f):
    print >> f, d.masterKey(),
    print >> f, '\nit', [s for s in d.getItems()],
    print >> f, '\nmu', [s for s in d.getMuonThresholds()],
    print >> f, '\nem', [s for s in d.getEMTauThresholds()],
    print >> f, '\njt', [s for s in d.getJetThresholds()],
    print >> f, '\nfj', [s for s in d.getForwardJetThresholds()],
    print >> f, '\net', [s for s in d.getTotalEtThresholds()],
    print >> f, '\nje', [s for s in d.getJetEtThresholds()],
    print >> f, '\nms', [s for s in d.getMissingEtThresholds()],
    print >> f, '\nctp',
    if d.ctpConfig():
        dump_CTPConfig (d.ctpConfig(), f)
    else:
        print >> f, None,
    print >> f, '\ntcf',
    if d.thresholdConfig():
        print >> f, d.thresholdConfig(),
    else:
        print >> f, None,
    return


@nolist
def dump_Lvl1AODPrescaleConfigData (d, f):
    print >> f, d.masterKey(),
    print >> f, [i for i in d.getPrescales()],
    return


def dump_TrackRecord (t, f):
    dump_H3V (t.GetPosition(), f)
    dump_H3V (t.GetMomentum(), f)
    print >> f, t.GetEnergy(),\
          t.GetPDGCode(),\
          t.GetTime(),\
          t.GetBarCode(),\
          t.GetVolName(),
    return


def dump_DetStatus (s, f):
    print >> f, s.code(),\
          s.fullCode(),\
          s.deadFrac(),\
          s.deadThrust(),\
          s.colour()
    return


@nolist
def dump_DetStatusMap (m, f):
    for e in m:
        print >> f, e.first,
        dump_DetStatus (e.second, f)
    return


def dump_TrigInDetTrackTruth (t, f):
    print >> f, t.nrMatches(),
    if t.nrMatches() == 0: return
    print >> f, t.bestMatch().barcode(), \
          t.bestSiMatch().barcode(),\
          t.bestTRTMatch().barcode(),\
          t.nrMatches(),\
          t.nrCommonHitsBestSi(),\
          t.nrCommonHitsBestTRT(),
    print >> f, [(t.truthMatch(i).barcode(),
                  t.nrCommonHits(i),
                  t.nrCommonSiHits(i),
                  t.nrCommonTRTHits(i))
                 for i in range(t.nrMatches())],
    print >> f, [(p.first, p.second) for p in t.getFamilyTree()],
    return


def dump_TrigInDetTrackFitPar (p, f):
    print >> f, "%f %f %f %f %f %f %f %f %f %f %f %d" % \
          (p.a0,
           p.z0,
           p.phi0,
           p.eta,
           p.pT,
           p.ea0,
           p.ez0,
           p.ephi0,
           p.eeta,
           p.epT,
           p.surfaceCoordinate,
           p.surfaceType),
    if p.cov:
        print >> f, [x for x in p.cov],
    return


def dump_TrigInDetTrack (t, f):
    print >> f, "%d %f %d %d %d %d" % \
          (t.algorithmId,
           t.chi2,
           t.NStrawHits,
           t.NStraw,
           t.NStrawTime,
           t.NTRHits),
    print >> f, tonone(t.siSpacePoints), tonone(t.trtDriftCircles),
    print >> f, '\n    par',
    if t.param:
        dump_TrigInDetTrackFitPar (t.param, f)
    else:
        print >> f, None,
    print >> f, '\n    end',
    if t.endParam:
        dump_TrigInDetTrackFitPar (t.endParam, f)
    else:
        print >> f, None,
    return


class PyTrigInDetTrackFitPar:
    def __init__ (self, t):
        self.a0 = t.a0()
        self.z0 = t.z0()
        self.phi0 = t.phi0()
        self.eta = t.eta()
        self.pT = t.pT()
        self.ea0 = t.ea0()
        self.ez0 = t.ez0()
        self.ephi0 = t.ephi0()
        self.eeta = t.eeta()
        self.epT = t.epT()
        self.surfaceCoordinate = t.surfaceCoordinate()
        self.surfaceType = t.surfaceType()
        if t.cov():
            self.cov = [x for x in t.cov()]
        else:
            self.cov = None
        return
    
class PyTrigInDetTrack:
    def __init__ (self, t):
        self.algorithmId = t.algorithmId()
        self.chi2 = t.chi2()
        self.NStrawHits = t.NStrawHits()
        self.NStraw = t.NStraw()
        self.NStrawTime = t.NStrawTime()
        self.NTRHits = t.NTRHits()
        self.siSpacePoints = t.siSpacePoints()
        self.trtDriftCircles = t.trtDriftCircles()
        if t.param():
            self.param = PyTrigInDetTrackFitPar (t.param())
        else:
            self.param = None
        if t.endParam():
            self.endParam = PyTrigInDetTrackFitPar (t.endParam())
        else:
            self.endParam = None
        return


def _tmcmp (x, y):
    xbc = 0
    if x[0].nrMatches() > 0:
        xbc = x[0].bestSiMatch().barcode()
    ybc = 0
    if y[0].nrMatches() > 0:
        ybc = y[0].bestSiMatch().barcode()
    c = xbc - ybc
    if c < 0:
        return -1
    elif c > 0:
        return 1
    return cmp (x[1].chi2, y[1].chi2)


@nolist
def dump_TrigInDetTrackTruthMap (m, f):
    #print >> f, m.size(),
    #print >> f, m.tracki(0),
    tm = [(m.truthi(i), PyTrigInDetTrack(m.tracki(i)))
          for i in range(m.size()) if m.trackiLink(i).isValid()]
    tm.sort (_tmcmp)
    for (i, (truth, track)) in enumerate(tm):
        print >> f, '\n  ', i,
        dump_TrigInDetTrackTruth (truth, f)
        print >> f, '\n    ',
        dump_TrigInDetTrack (track, f)
    return


# Deal with the possibility that some of the ELs in the target set
# may be invalid.  Doesn't seem to be any way to know that other than
# by trying to do the deref.  (I.e., we can't get a hold of the EL directly.)
def safe_assocs (a, obj, coll, f):
    bv = a.beginAssociation(obj)
    ev = a.endAssociation(obj)
    res = []
    err = False
    while bv != ev:
        try:
            targ = bv.__deref__()
            res.append (targ)
        except RuntimeError, e:
            if e.args[0].find ('dereferencing invalid ElementLink') >= 0:
                err = True
            else:
                raise
        bv.__preinc__()

    if err:
        errflag = True
        for o in res:
            coll.push_back (o)
    else:
        errflag = False
        a.assocs (obj, coll)
    return errflag


@nolist
def dump_Assocs (a, f, colltype):
    bo = a.beginObject()
    eo = a.endObject()
    l = []
    while bo != eo:
        obj = a.getObject(bo)
        coll = colltype(1)
        errflag = safe_assocs (a, obj, coll, f)
        l.append ((obj, coll, errflag))
        bo.next()

    l.sort (cmp=lambda a, b: cmp(b[0].pt(), a[0].pt()))

    for obj, coll, errflag in l:
        print >> f, '\n', obj.__class__.__name__,
        dump_Fourvec (obj, f)
        print >> f, '->',
        for p in coll:
            print >> f, p.__class__.__name__,
            dump_Fourvec (p, f),
        if errflag:
            print >> f, '  [Got invalid EL error]',
    return


@nolist
def dump_INav4MomAssocs (a, f):
    return dump_Assocs (a, f, PyAthena.DataVector(PyAthena.INavigable4Momentum))


@nolist
def dump_TrackParticleAssocs (a, f):
    return dump_Assocs (a, f, PyAthena.Rec.TrackParticleContainer)


def dump_ElectronMuonTopoInfo (a, f):
    print >> f, a.RoiWord(), a.DeltaPhi(), a.DeltaR(), a.InvMass(),\
          a.ElecValid(), a.OppositeCharge(), a.VertexState(),
    return


def dump_MuonSpShower (m, f):
    print >> f, "%f %f %d %d %d %d %d %d %d" % \
          (m.eta(),
           m.phi(),
           m.numberOfTriggerHits(),
           m.numberOfInnerHits(),
           m.numberOfMiddleHits(),
           m.numberOfOuterHits(),
           m.numberOfInnerSegments(),
           m.numberOfMiddleSegments(),
           m.numberOfOuterSegments()),
    return


def dump_JetAssociationBase (a, f):
    print >> f, a.name(),
    return


def dump_ElectronAssociation (a, f):
    dump_JetAssociationBase (a, f)
    ele = a.electron()
    if ele:
        print >> f, a.getElectronWeight (ele), 
        dump_Fourvec (ele, f)
    return


def dump_PhotonAssociation (a, f):
    dump_JetAssociationBase (a, f)
    gam = a.photon()
    if gam:
        print >> f, a.getPhotonWeight (gam), 
        dump_Fourvec (gam, f)
    return


def dump_MuonAssociation (a, f):
    dump_JetAssociationBase (a, f)
    muo = a.muon()
    if muo:
        print >> f, a.getMuonWeight (muo), 
        dump_Fourvec (muo, f)
    return


def dump_TrackAssociation (a, f):
    dump_JetAssociationBase (a, f)
    print >> f, a.nTracks(),
    tr = a.tracks()
    ROOT.SetOwnership (tr, True)
    for t in tr:
        print >> f, a.getTrackWeight (t), 
        dump_Fourvec (t, f)
    return


def dump_ISvxAssociation (a, f):
    dump_JetAssociationBase (a, f)
    vi = a.vertexInfo()
    if vi:
        for v in vi.vertices():
            dump_Threevec (v.position(), f)
    return


def _infoType (i):
    return i.infoType()

def dump_Jet (j, f):
    if j.e() == 0 and j.phi() == 0 and j.eta() == 0:
        with signalstate (j, PyAthena.P4SignalState.JETEMSCALE):
            dump_ParticleImpl (j, f)
    else:
        dump_ParticleImpl (j, f)
    print >> f, '\n      %s %f' % \
          (j.jetAuthor(),
           j.getFlavourTagWeight(),),
    # ELs to towers will always be invalid (towers aren't actually saved...)
    tower_constituents_p = False
    if j.firstConstituent() != j.lastConstituent():
        if isinstance (j.getContainer(j.firstConstituent()),
                       PyAthena.CaloTowerContainer):
            tower_constituents_p = True
        elif j.firstConstituent().__deref__():
            ss = j.constituentSignalState()
            j.setConstituentSignalState (PyAthena.P4SignalState.CALIBRATED)
            dump_HLV (j.constituent_sum4Mom(), f)
            j.setConstituentSignalState (ss)
    print >> f, [x for x in j.combinedLikelihood()],
    print >> f, '\n      moms ',
    for mk in j.getMomentKeys():
        if mk in ['Timing', 'LArQuality']: continue
        mom = j.getMoment (mk, False)
        if  mom != 0:
            print >> f, mk, j.getMoment (mk, True),
    if hasattr(j, 'getJetTime'):
        print >> f, '\n      timing ', j.getJetTime(), \
              ' qual ', j.getJetQuality(),
    else:
        print >> f, '\n      timing ', j.getMoment('Timing',True), \
              ' qual ', j.getMoment('LArQuality',True),
    print >> f, '\n      assoc ',
    for ak in j.getAssociationKeys():
        ass = j.getAssociationBase(ak)
        if not ass: continue
        print >> f, '\n         ', ak,
        if isinstance (ass, PyAthena.Analysis.ElectronAssociation):
            dump_ElectronAssociation (ass, f)
        elif isinstance (ass, PyAthena.Analysis.PhotonAssociation):
            dump_PhotonAssociation (ass, f)
        elif isinstance (ass, PyAthena.Analysis.MuonAssociation):
            dump_MuonAssociation (ass, f)
        elif isinstance (ass, PyAthena.Analysis.TrackAssociation):
            dump_TrackAssociation (ass, f)
        elif isinstance (ass, PyAthena.Analysis.ISvxAssociation):
            dump_ISvxAssociation (ass, f)
        else:
            print >> f, ass,
    ti = j.jetTagInfoVector()
    if ti:
        print >> f, '\n      tag info:',
        #ROOT.SetOwnership (ti, True)
        ti = list(ti)
        ti.sort (key=_infoType)
        for info in ti:
            if not info:
                print >> f, '\n      (null)',
                continue
            print >> f, '\n      %s %s: %f: ' \
                  % (info.infoType(), info.__class__.__name__, j.getFlavourTagWeight (info.infoType())),
            if isinstance (info, PyAthena.Analysis.TruthInfo):
                dump_TruthInfo (info, f)
            elif isinstance (info, PyAthena.Analysis.SoftLeptonTruthInfo):
                dump_SoftLeptonTruthInfo (info, f)
            elif isinstance (info, PyAthena.Analysis.SecVtxInfo):
                dump_SecVtxInfo (info, f)
            elif isinstance (info, PyAthena.Analysis.IPInfoPlus):
                dump_IPInfoPlus (info, f)
            elif isinstance (info, PyAthena.Analysis.IPInfoBase):
                dump_IPInfoBase (info, f)
            elif isinstance (info, PyAthena.Analysis.SVInfoBase):
                dump_SVInfoBase (info, f)
            elif isinstance (info, PyAthena.Analysis.SVInfoPlus):
                dump_SVInfoPlus (info, f)
            elif isinstance (info, PyAthena.Analysis.MultiSVInfoPlus):
                dump_MultiSVInfoPlus (info, f)
            elif isinstance (info, PyAthena.Analysis.JetProbInfoBase):
                dump_JetProbInfoBase (info, f)
            elif isinstance (info, PyAthena.Analysis.SoftElectronInfo):
                dump_SoftElectronInfo (info, f)
            elif isinstance (info, PyAthena.Analysis.JetFitterTagInfo):
                dump_JetFitterTagInfo (info, f)
            elif (hasattr (PyAthena.Analysis, 'TrackCountingInfo') and
                  isinstance (info, PyAthena.Analysis.TrackCountingInfo)):
                dump_TrackCountingInfo (info, f)
            elif isinstance (info, PyAthena.Analysis.AtlfInfo):
                dump_AtlfInfo (info, f)
            elif isinstance (info, PyAthena.Analysis.SoftMuonInfo):
                dump_SoftMuonInfo (info, f)
            elif info.__class__ is PyAthena.Analysis.BaseTagInfo:
                dump_BaseTagInfo (info, f)
            elif info.__class__ is PyAthena.Analysis.GbbNNTagInfo:
                dump_GbbNNTagInfo (info, f)
            else:
                print >> f, info,
    if tower_constituents_p:
        print >> f, '\n      (not dumping tower constituents)',
    else:
        print >> f, '\n      constituents:',
        for c in toiter (j.firstConstituent(), j.lastConstituent()):
            print >> f, '\n        ',
            if not c:
                print >> f, tonone(c),
            else:
                print >> f, c.__class__.__name__,
                print >> f, j.getWeight (c),
                dump_Fourvec (c, f)
    return


def dump_ExtendedVxCandidate (c, f):
    dump_VxCandidate1 (c, f)
    print >> f, '\n          em',
    if c.fullCovariance():
        dump_AmgMatrix (c.fullCovariance(), f)
    else:
        print >>f, None,
    return


def dump_V0Hypothesis (h, f):
    if h == None:
        print >> f, None,
        return
    print >> f, 'V0Hypothesis', h.positiveTrackID(), \
          h.negativeTrackID(), h.hypothesisID(),
    print >> f, '\n     ',
    dump_ExtendedVxCandidate (h, f)
    return


def dump_V0Candidate (v, f):
    print >> f, "V0Candidate", len(v.v0Hypothesis()),
    for h in v.v0Hypothesis():
        print >> f, '\n     ',
        dump_V0Hypothesis (h, f)
    return


def dump_TrigVertex (v, f):
    dump_Threevec (v, f)
    print >> f, v.algorithmId(), v.chi2(), v.ndof(), v.mass(), \
          v.massVariance(), v.energyFraction(), v.nTwoTracksSecVtx(),
    print >> f, '\n   cov: ', [v.cov()[i] for i in range(6)],
    if v.tracks() and v.tracks().size() > 0:
        for (i, t) in enumerate (v.tracks()):
            print >> f, '\n   t%d'%i,
            p = t.param()
            print >> f, p.a0(), p.z0(), p.phi0(), p.eta(), p.pT(),
    if v.getMotherTrack():
        print >> f, '\n   mother: ',
        dump_TrigInDetTrackFitPar (v.getMotherTrack(), f)
    return


def dump_clist (l, f):
    print >> f, '[',
    last = None
    n = 0
    for i in l:
        if i != last:
            if n > 0:
                if n == 1:
                    print >> f, '%f, ' % last,
                else:
                    print >> f, '%d*%f, ' % (n, last),
            last = i
            n = 1
        else:
            n = n + 1
    if n > 0:
        if n == 1:
            print >> f, '%f, ' % last,
        else:
            print >> f, '%d*%f, ' % (n, last),
    print >> f, ']',
            
def dump_TrigTrackCounts (t, f):
    print >> f, 'z0_pt:',
    dump_TrigHisto2D (t.z0_pt(), f)
    print >> f, '\neta_phi:',
    dump_TrigHisto2D (t.eta_phi(), f)
    return
@nolist
def dump_TrigTrackCounts_nolist (t, f):
    dump_TrigTrackCounts (t, f)
    return


def dump_TrigTau (t, f):
    dump_Fourvec (t, f)
    print >> f, t.roiId(), t.Zvtx(), t.err_Zvtx(), t.etCalibCluster(), \
          t.simpleEtFlow(), t.nMatchedTracks(),
    print >> f, '\n   ', tonone(t.tauCluster()), \
          tonone(t.trackCollection()), \
          tonone(t.tracksInfo()),
    return
@nolist
def dump_TrigTau_nolist (t, f):
    dump_TrigTau (t, f)
    return


def dump_TrigCaloCluster (c, f):
    print >> f, c.rawEnergy(), c.rawEt(), c.rawEta(), c.rawPhi(), c.RoIword(),\
          c.nCells(), c.quality(),
    print >> f, '\n    ', [c.rawEnergy(i) for i in range(25)],
    return


def dump_TrigEMCluster (c, f):
    dump_TrigCaloCluster (c, f)
    print >> f, '\n    ', c.energy(), c.et(), c.eta(), c.phi(),\
          c.e237(), c.e277(), c.fracs1(), c.weta2(), \
          c.ehad1(), c.Eta1(), c.emaxs1(), c.e2tsts1(),
    print >> f, '\n    ', [c.energyInSample(i) for i in range(25)],
    return
@nolist
def dump_TrigEMCluster_nolist (c, f):
    dump_TrigEMCluster (c, f)
    return


def dump_TrigElectron (p, f):
    dump_Fourvec (p, f)
    print >> f, p.isValid(), p.roiId(),
    print >> f, '\n   ', p.trackAlgo(), p.trackIndx(), p.charge(),\
          p.Zvtx(),
    print >> f, '\n   ', p.err_Pt(), p.err_eta(), p.err_phi(), p.err_Zvtx(),
    print >> f, '\n   ', p.trkClusDeta(), p.trkClusDphi(), p.EtOverPt(),
    return


def dump_TrigPhoton (p, f):
    dump_Fourvec (p, f)
    print >> f, p.isValid(), p.roiId(),
    print >> f, '\n   ', p.Et(), p.HadEt(), p.energyRatio(), p.rCore(),\
          p.dPhi(), p.dEta(),
    print >> f, '\n   ',
    if p.cluster():
        dump_TrigEMCluster (p.cluster(), f)
    return


@nolist
def dump_TrigInDetTrackCollection (t, f):
    print >> f, ' ROI ', t.RoI_ID()
    t2 = [PyTrigInDetTrack(tt) for tt in t]
    dump_list (t2, f, dump_TrigInDetTrack)
    return


def dump_TrigEFBjet (j, f):
    dump_Fourvec (j, f)
    print >> f, j.isValid(), j.roiId(),
    print >> f, '\n   ', j.prmVtx(), j.xComb(), j.xIP1D(), j.xIP2D(), \
          j.xIP3D(), j.xCHI2(), j.xSV(), j.xMVtx(), j.xEVtx(), j.xNVtx(),
    print >> f, '\n   ', tonone(j.TrackCollection()), \
          tonone(j.PrmVertexCollection()), \
          tonone(j.SecVertexCollection()),
    return


def dump_TrigEFBphys (j, f):
    print >> f, j.roiId(), j.particleType(), j.eta(), j.phi(),\
          j.mass(), j.fitmass(), j.fitchi2(), j.fitndof(),\
          j.fitx(), j.fity(), j.fitz(),
    if j.pSecondDecay():
        print >> f, '\n     second:',
        dump_TrigEFBphys (j.pSecondDecay(), f)
    for t in j.trackVector():
        print >> f, '\n     tv:',
        if t.isValid():
            print >> f, t.dataID(), t.index(),
        else:
            print >> f, '(invalid)',
    return


def dump_TrigL2Bjet (j, f):
    dump_Fourvec (j, f)
    print >> f, j.isValid(), j.roiId(),
    print >> f, '\n   ', j.prmVtx(), j.xComb(), j.xIP1D(), j.xIP2D(), \
          j.xIP3D(), j.xCHI2(), j.xSV(), j.xMVtx(), j.xEVtx(), j.xNVtx(),
    print >> f, '\n   ', \
          tonone(j.TrackCollection()), \
          tonone(j.PrmVertexCollection()), \
          tonone(j.SecVertexCollection()),
    return


def dump_TrigL2Bphys (j, f):
    print >> f, j.roiId(), j.particleType(), j.eta(), j.phi(),\
          j.mass(), j.fitmass(), j.fitchi2(), j.fitndof(),\
          j.fitx(), j.fity(), j.fitz(),
    if j.pSecondDecay():
        print >> f, '\n     second:',
        dump_TrigL2Bphys (j.pSecondDecay(), f)
    # ??? Straightforward iteration fails if jets are also dumped,
    #     for 15.2.0 samples.  Why?
    #for t in j.trackVector():
    tv = j.trackVector()
    for i in range(tv.size()):
        t = tv[i]
        print >> f, '\n     tv:', t.dataID(), t.index(),
    return


def dump_TrigMissingET (m, f):
    print >> f, '  ', m.ex(), m.ey(), m.ez(), m.sumEt(), m.sumE(), m.et(),\
          m.e(), m.RoIword(), m.getFlag(), m.getNumOfComponents(),
    for ic in range(m.getNumOfComponents()):
        print >> f, '\n   ', m.getNameOfComponent(ic),\
              m.getExComponent(ic), m.getEyComponent(ic),\
              m.getEzComponent(ic), m.getSumEtComponent(ic),\
              m.getSumEComponent(ic), m.getComponentCalib0(ic),\
              m.getComponentCalib1(ic), m.getSumOfSigns(ic),\
              m.getUsedChannels(ic), m.getStatus(ic),
    print >> f, ' '
    return
@nolist
def dump_TrigMissingET_nolist (m, f):
    dump_TrigMissingET (m, f)
    return


def dump_TrigRoiDescriptor (d, f):
    print >> f, '  ', d.roiId(), d.l1Id(), d.roiWord(), \
          d.isFullscan(), d.version(), \
          d.eta(), d.phi(), d.zed(), \
          d.etaPlus(), d.phiPlus(), d.zedPlus(), \
          d.dzdrPlus(), d.dzdrMinus(), d.drdzPlus(), d.drdzMinus(), \
          d.composite(), d.manageConstituents(), d.size(),
    return
@nolist
def dump_TrigRoiDescriptor_nolist (d, f):
    dump_TrigRoiDescriptor (d, f)
    return


def dump_Trig3Momentum (m, f):
    print >> f, m.e(), m.eta(), m.phi(), m.inCone(), m.caloSample(),
    return


def dump_TrigT2Jet (j, f):
    print >> f, '  ',
    dump_Fourvec (j, f)
    print >> f, '  ', j.e(), j.ehad0(), j.eem0(), j.eta(), j.phi(), j.m(),\
          j.RoIword(),
    if j.grid():
        for m in j.grid():
            print >> f, '\n   ',
            dump_Trig3Momentum (m, f)
    print >> f, ' '
    return
@nolist
def dump_TrigT2Jet_nolist (j, f):
    dump_TrigT2Jet (j, f)
    return


def dump_TrigTauClusterDetails (t, f):
    if not t: return
    for s in range(4):
        print >> f, '\n    EM%d: '% s,
        print >> f, t.EMRadius(s), t.EMenergyWidth(s), t.EMenergyMedium(s),\
              t.EMenergyWide(s), t.EMenergyNarrow(s),
    for s in range(3):
        print >> f, '\n    HAD%d: '% s,
        print >> f, t.HADRadius(s), t.HADenergyWidth(s), t.HADenergyMedium(s),\
              t.HADenergyWide(s), t.HADenergyNarrow(s),
    return

def dump_TrigTauCluster (t, f):
    print >> f, '  ',
    dump_TrigCaloCluster (t, f)
    print >> f, '\n   ', t.energy(), t.et(), t.EMCalibEnergy(), t.EMenergy(), \
          t.HADenergy(), t.eta(), t.phi(), t.IsoFrac(), t.numStripCells(), \
          t.stripWidth(), t.eCalib(), t.eEMCalib(), t.EMRadius2(),
    for s in range(25):
        print >> f, '\n    %2d: '% s,
        print >> f, t.energy(s),
    dump_TrigTauClusterDetails (t.clusterDetails(), f)
    print >> f, ' '
    return
@nolist
def dump_TrigTauCluster_nolist (t, f):
    dump_TrigTauCluster (t, f)
    return


def dump_TrigHisto (h, f):
    print >> f, "x:", h.nbins_x(), h.min_x(), h.max_x(), '[',
    for x in h.contents():
        print >> f, x,
    print >>f, ']',
    return
def dump_TrigHisto1D (h, f):
    dump_TrigHisto (h, f)
    return
def dump_TrigHisto2D (h, f):
    print >> f, "y:", h.nbins_y(), h.min_y(), h.max_y(),
    dump_TrigHisto (h, f)
    return

def dump_TrigSpacePointCounts (c, f):
    print >> f, c.sctSpEndcapC(), c.sctSpBarrel(), c.sctSpEndcapA(), '[',
    for id in c.droppedSctModules():
        print >> f, id.getString(),
    print >>f, ']',
    print >> f, '\n   clus endcap c: ',
    dump_TrigHisto2D (c.pixelClusEndcapC(), f)
    print >> f, '\n   clus barrel: ',
    dump_TrigHisto2D (c.pixelClusBarrel(), f)
    print >> f, '\n   clus endcap a: ',
    dump_TrigHisto2D (c.pixelClusEndcapA(), f)
    return
@nolist
def dump_TrigSpacePointCounts_nolist (c, f):
    dump_TrigSpacePointCounts (c, f)
    return


def dump_TrigTauTracksInfo (t, f):
    print >> f, '  ',
    dump_Fourvec (t, f)
    print >> f, '  ', t.roiId(), t.nCoreTracks(), t.nSlowTracks(), \
          t.nIsoTracks(),\
          t.charge(), t.leadingTrackPt(), t.sumPtRatio(), t.scalarPtSumCore(),\
          t.scalarPtSumIso(), t.ptBalance(),
    dump_Fourvec (t.threeFastestTracks(), f)
    if t.trackCollection():
        print >> f, '\n   ',
        dump_TrigInDetTrackCollection (t.trackCollection(), f)
    print >> f, ' '
    return
@nolist
def dump_TrigTauTracksInfo_nolist (t, f):
    dump_TrigTauTracksInfo (t, f)
    return


def dump_MuonFeature (m, f):
    print >> f, '  ', m.roiId(), m.saddress(), m.pt(), m.radius(), \
          m.eta(), m.phi(), m.dir_phi(), m.zeta(), m.dir_zeta(), m.beta(),\
          m.sp1_r(), m.sp1_z(), m.sp1_slope(), \
          m.sp2_r(), m.sp2_z(), m.sp2_slope(), \
          m.sp3_r(), m.sp3_z(), m.sp3_slope(), \
          m.br_radius(), m.br_sagitta(), m.ec_alpha(), m.ec_beta(),\
          m.dq_var1(), m.dq_var2(), m.algoId(),
    return
@nolist
def dump_MuonFeature_nolist (m, f):
    dump_MuonFeature (m, f)
    return


def dump_MuonFeatureDetails (m, f):
    print >> f, \
          m.extension_capacity(), \
          m.max_rob_capacity(), \
          m.max_csm_capacity(), \
          m.max_lvl1_emu_capacity(), \
          m.max_rpc_hits_capacity(), \
          m.max_tgc_hits_capacity(), \
          m.max_mdt_hits_capacity(), \
          m.id(), m.te_id(), m.error(),
    print >> f, '\n  ',\
          m.lvl1_id(), m.lumi_block(), m.muondetmask(), \
          m.roi_id(), m.roi_system(), m.roi_subsystem(), \
          m.roi_sector(), m.roi_number(), m.roi_threshold(), \
          m.roi_eta(), m.roi_phi(), \
          m.rpc_pad_error(), m.tgc_rdo_error(),
    print >> f, '\n  ',\
          m.rpc1_x(), m.rpc1_y(), m.rpc1_z(), \
          m.rpc2_x(), m.rpc2_y(), m.rpc2_z(), \
          m.rpc3_x(), m.rpc3_y(), m.rpc3_z(),
    print >> f, '\n  ',\
          m.tgc_Mid1_eta(), m.tgc_Mid1_phi(), m.tgc_Mid1_r(), m.tgc_Mid1_z(),\
          m.tgc_Mid2_eta(), m.tgc_Mid2_phi(), m.tgc_Mid2_r(), m.tgc_Mid2_z(),\
          m.tgc_Mid_rho_chi2(), m.tgc_Mid_rho_N(), \
          m.tgc_Mid_phi_chi2(), m.tgc_Mid_phi_N(),
    print >> f, '\n  ',\
          m.tgc_Inn_eta(),     m.tgc_Inn_phi(),\
          m.tgc_Inn_r(),       m.tgc_Inn_z(),\
          m.tgc_Inn_rho_std(), m.tgc_Inn_rho_N(),\
          m.tgc_Inn_phi_std(), m.tgc_Inn_phi_N(),\
          m.tgc_PT(),
    print >> f, '\n  ',\
          m.mdt_Inner_slope(), \
          m.mdt_Inner_intercept(), \
          m.mdt_Inner_Z(), \
          m.mdt_Inner_R(), \
          m.mdt_Inner_fit_chi(), \
          m.mdt_Middle_slope(), \
          m.mdt_Middle_intercept(), \
          m.mdt_Middle_Z(), \
          m.mdt_Middle_R(), \
          m.mdt_Middle_fit_chi(), \
          m.mdt_Outer_slope(), \
          m.mdt_Outer_intercept(), \
          m.mdt_Outer_Z(), \
          m.mdt_Outer_R(), \
          m.mdt_Outer_fit_chi(),
    print >> f, '\n  ',\
          m.Address(), m.Sagitta(), m.Radius(), m.Slope(), m.Intercept(),\
          m.Alpha(), m.Beta(), m.DeltaR(), m.Speed_over_c(), \
          m.PhiMap(), m.Phi(), m.PhiDir(), m.Pt(), m.Charge(),
    print >> f, '\n  ',\
          m.eta_pivot_lay0(),\
          m.eta_pivot_lay1(),\
          m.eta_low_0_lay0(),\
          m.eta_low_1_lay0(),\
          m.eta_low_0_lay1(),\
          m.eta_low_1_lay1(),\
          m.eta_high_0_lay0(),\
          m.eta_high_1_lay0(),\
          m.eta_high_0_lay1(),\
          m.eta_high_1_lay1(),\
          m.phi_pivot_lay0(),\
          m.phi_pivot_lay1(),\
          m.phi_low_0_lay0(),\
          m.phi_low_1_lay0(),\
          m.phi_low_0_lay1(),\
          m.phi_low_1_lay1(),\
          m.phi_high_0_lay0(),\
          m.phi_high_1_lay0(),\
          m.phi_high_0_lay1(),\
          m.phi_high_1_lay1(),
    print >> f, '\n  ', list(m.rob_id()),
    print >> f, '\n  ', list(m.csm_id()),
    print >> f, '\n  ', list(m.csm_size()),
    print >> f, '\n  ', list(m.csm_error()),
    print >> f, '\n  ', list(m.removed_rob_id()),
    print >> f, '\n  ', list(m.removed_csm_id()),
    print >> f, '\n  ', list(m.lvl1_emulation()),
    print >> f, '\n  ', list(m.pad_hit_onlineId()),
    print >> f, '\n  ', list(m.pad_hit_code()),
    print >> f, '\n  ', list(m.pad_hit_x()),
    print >> f, '\n  ', list(m.pad_hit_y()),
    print >> f, '\n  ', list(m.pad_hit_z()),
    print >> f, '\n  ', list(m.pad_hit_r()),
    print >> f, '\n  ', list(m.pad_hit_p()),
    print >> f, '\n  ', list(m.tgc_Inn_rho_hit_phi()),
    print >> f, '\n  ', list(m.tgc_Inn_rho_hit_r()),
    print >> f, '\n  ', list(m.tgc_Inn_rho_hit_z()),
    print >> f, '\n  ', list(m.tgc_Inn_rho_hit_width()),
    print >> f, '\n  ', list(m.tgc_Inn_rho_hit_in_seg()),
    print >> f, '\n  ', list(m.tgc_Inn_phi_hit_phi()),
    print >> f, '\n  ', list(m.tgc_Inn_phi_hit_r()),
    print >> f, '\n  ', list(m.tgc_Inn_phi_hit_z()),
    print >> f, '\n  ', list(m.tgc_Inn_phi_hit_width()),
    print >> f, '\n  ', list(m.tgc_Inn_phi_hit_in_seg()),
    print >> f, '\n  ', list(m.tgc_Mid_rho_hit_phi()),
    print >> f, '\n  ', list(m.tgc_Mid_rho_hit_r()),
    print >> f, '\n  ', list(m.tgc_Mid_rho_hit_z()),
    print >> f, '\n  ', list(m.tgc_Mid_rho_hit_width()),
    print >> f, '\n  ', list(m.tgc_Mid_rho_hit_in_seg()),
    print >> f, '\n  ', list(m.tgc_Mid_phi_hit_phi()),
    print >> f, '\n  ', list(m.tgc_Mid_phi_hit_r()),
    print >> f, '\n  ', list(m.tgc_Mid_phi_hit_z()),
    print >> f, '\n  ', list(m.tgc_Mid_phi_hit_width()),
    print >> f, '\n  ', list(m.tgc_Mid_phi_hit_in_seg()),
    print >> f, '\n  ', list(m.mdt_onlineId()),
    print >> f, '\n  ', list(m.mdt_offlineId()),
    print >> f, '\n  ', list(m.mdt_tube_r()),
    print >> f, '\n  ', list(m.mdt_tube_z()),
    print >> f, '\n  ', list(m.mdt_tube_residual()),
    print >> f, '\n  ', list(m.mdt_tube_time()),
    print >> f, '\n  ', list(m.mdt_tube_space()),
    print >> f, '\n  ', list(m.mdt_tube_sigma()),
    print >> f, '\n  ', list(m.extension0()),
    print >> f, '\n  ', list(m.extension1()),
    print >> f, '\n  ', list(m.extension2()),
    print >> f, '\n  ', list(m.extension3()),
    print >> f, '\n  ', list(m.extension4()),
    print >> f, '\n  ', list(m.extension5()),
    print >> f, '\n  ', list(m.extension6()),
    print >> f, '\n  ', list(m.extension7()),
    print >> f, '\n  ', list(m.extension8()),
    print >> f, '\n  ', list(m.extension9()),
    return


def dump_TileMuFeature (m, f):
    print >> f, '  ', m.eta(), m.phi(), m.quality(), list(m.enedep()), 
    return


def dump_TileTrackMuFeature (m, f):
    dump_Fourvec (m, f)
    print >> f, m.PtTR_Trk(), m.EtaTR_Trk(), m.PhiTR_Trk(), m.Typ_IDTrk(),
    print >> f, '\n   ', m.pt(), m.eta(), m.phi(), m.m(),
    print >> f, '\n   ', m.TileMuLink().isValid(),
    if m.TileMuLink().isValid():
        print >> f, m.TileMuLink().dataID(), m.TileMuLink().index()
    print >> f, '\n   ', m.IDScanLink().isValid(),
    if m.IDScanLink().isValid():
        print >> f, m.IDScanLink().dataID(), m.IDScanLink().index()
    return


def dump_CombinedMuonFeature (m, f):
    print >> f, '  ',
    dump_Fourvec (m, f)
    print >> f, m.pt(), m.eta(), m.phi(), m.m(), m.sigma_pt(),
    if m.IDTrackLink().isValid() and m.IDTrack():
        print >> f, '\n  id',
        dump_TrigInDetTrack (PyTrigInDetTrack(m.IDTrack()), f)
    if m.muFastTrackLink().isValid() and m.muFastTrack():
        print >> f, '\n  fast',
        dump_MuonFeature (m.muFastTrack(), f)
    else:
        print >> f, ' '
    return
@nolist
def dump_CombinedMuonFeature_nolist (m, f):
    dump_CombinedMuonFeature (m, f)
    return


def dump_TrigMuonEF (m, f):
    dump_Fourvec (m, f)
    print >> f, m.MuonCode(), m.RoINum(), m.charge(),
    return


def dump_TileMu (m, f):
    print >> f, m.eta(), m.phi(), m.quality(), list(m.enedep()),
    return


def dump_CaloClusterMomentStore (m, f):
    beg = m.begin()
    end = m.end()
    while beg != end:
        print >> f, '\n     ', beg.getMomentType(), beg.getMoment().getValue(),
        beg.next()
    return


def dump_CaloSamplingData (s, f):
    nsamp = s.getNumberOfSamplings()
    for i in range(min(10, s.getNumberOfVariableTypes())):
        if s.contains(i):
            slist = [s.retrieveData(i, samp) for samp in range(nsamp)]
            print >> f, '\n     ', i, slist,
    return


def dump_CaloShower (s, f):
    print >> f, '   moments:',
    dump_CaloClusterMomentStore (s.getMomentStore(), f)
    print >> f, '\n   sampling data:',
    dump_CaloSamplingData (s.getSamplingStore(), f)
    return


@nolist
def dump_CaloTowerContainer (t, f):
    # Note: container contents aren't saved...
    print >> f, t.neta(), t.etamin(), t.deta(), \
          t.nphi(), t.phimin(), t.dphi()
    return


@nolist
def dump_CaloTopoTowerContainer (t, f):
    # Note: container contents aren't saved...
    dump_CaloTowerContainer (t, f)
    print >> f, '  ', t.GetMinimumCellEnergy(), \
          t.GetMinimumClusterEnergy(), \
          t.GetUseCellWeights(), \
          t.GetUseNoiseTool(), \
          t.GetUsePileUpNoise(), \
          t.GetNoiseSigma(), \
          t.GetCellESignificanceThreshold(), \
          t.GetCaloSelection(), \
          list(t.GetCaloIndices())
    def dl(l):
        print >> f, '  ',
        if l.isValid(): print >> f, l.dataID()
        else: print >> f, '(null)'
    dl(t.GetClusters())
    dl(t.GetTowers())
    dl(t.GetCells())
    if t.GetCellToClusterMap():
        print >> f, '  ', GetCellToClusterMap().size()
    else:
        print >> f, '  (null)'
    return


def dump_CaloCalibrationHit (t, f):
    print >> f, t.cellID().getString(), t.particleID(), \
          [t.energy(i) for i in range(4)],
    return


def dump_TrigT2MbtsBits (t, f):
    if hasattr(t, 'triggerEnergies'):
        print >> f, list(t.triggerEnergies()), list(t.triggerTimes()),
    else:
        print >> f, t.mbtsWord(), list(t.triggerTimes()),
    return


def dump_CosmicMuon (m, f):
    # Not really implemented.
    # The AODs i saw had containers of these, which were always empty.
    # Print as the address, so that if we do see one with contents,
    # we'll see a miscompare and know to fill this in.
    print >> f, id(m),
    #print >> f, m.p(), m.theta(), m.phi(), m.t(), m.radius(),\
    #      m.isIngoing, m.NRpcPairs(), m.NTgcPairs(), m.NMdtHits(), \
    #      m.NMdtSegs(),
#   void getRefPoint(double vec[3]) const;
#   double residual(double z, double r) const;
#   double longPos(double z, double r) const;
#   void perigee(double point[3], double x0=0, double y0=0, double z0=0) const;
#   void perigee2D(double point[2], double x0=0, double y0=0) const;
#   double dca3D(double x0=0.0, double y0=0.0, double z0=0.0) const;
#   double dcaXY(double x0=0.0, double y0=0.0) const;
#   double dca2D_Z(double x0=0.0, double y0=0.0) const;
    return


def dump_MdtTrackSegment (t, f):
    # Not really implemented.
    # The AODs i saw had containers of these, which were always empty.
    # Print as the address, so that if we do see one with contents,
    # we'll see a miscompare and know to fill this in.
    print >> f, id(t),
    return


def dump_IsoMuonFeature (m, f):
    dump_Fourvec (m, f)
    print >> f, '\n      ', m.getEtInnerConeEC(), m.getEtOuterConeEC(), \
          m.getEtInnerConeHC(), m.getEtOuterConeHC(),
    print >> f, '\n      ', m.getNTracksCone(), m.getSumPtTracksCone(),\
          m.getPtMuTracksCone(),
    print >> f, '\n      ', m.getRoiIdMu(), m.getPtMu(), m.getEtaMu(),\
          m.getPhiMu(), m.getLArWeight(), m.getTileWeight(),
    return


def dump_TrigMuonEFTrack (t, f):
    if not t:
        print None,
        return
    dump_Fourvec (t, f)
    print >> f, t.charge(),
    return


def dump_TrigMuonEFInfo (t, f):
    print >> f, t.RoINum(),
    print >> f, '\n    spectrometer: ',
    dump_TrigMuonEFTrack (t.SpectrometerTrack(), f)
    print >> f, '\n    extrapolated: ',
    dump_TrigMuonEFTrack (t.ExtrapolatedTrack(), f)
    print >> f, '\n    combined: ',
    dump_TrigMuonEFTrack (t.CombinedTrack(), f)
    return


def dump_RingerRings (r, f):
    print >> f, list(r.rings()),
    return


def dump_TrigTrtHitCounts (p, f):
    print >> f, '  barrel',
    dump_TrigHisto1D (p.barrel(), f)
    print >> f, '\n      endcapA',
    dump_TrigHisto1D (p.endcapA(), f)
    print >> f, '\n      endcapC',
    dump_TrigHisto1D (p.endcapC(), f)
    return


def dump_TrigRNNOutput (p, f):
    print >> f, list(p.output()),
    #if p.isValid():
    #    cl = p.cluster()
    #    print >> f, '  [', cl.rawEnergy(), cl.rawEta(), cl.rawPhi(), ']',
    return


def dump_InDetLowBetaCandidate (p, f):
    if p == None:
        print >> f, '(null)',
        return
    if hasattr (p, 'getTRTInverseBeta'):
        print >> f, p.getTRTCorrBitsOverThreshold(), \
              p.getTRTInverseBeta(), \
              p.getTRTInverseBetaError(), \
              p.getTRTNLastBits(),
    else:
        print >> f, p.getTRTCorrBitsOverThreshold(), \
              p.getTRTTrailingEdge(), \
              p.getTRTTrailingEdgeError(), \
              p.getTRTNLastBits(),
        others = (p.getTRTdEdx(), \
                  p.getTRTLikelihoodBeta(), \
                  p.getTRTLikelihoodError(), \
                  p.getTRTHighTbits(),)
        if max(others)!=0 or min(others) != 0:
            for o in others: print >> f, o,
    return


@nolist
def dump_LArNoisyROSummary (p, f):
    print >> f, '  noisy febs', [id.getString() for id in p.get_noisy_febs()]
    # Dictionary problem here --- punt for now.
    #print >> f, '  noisy preamps', \
    #      [(p.first.getString(),p.second) for p in p.get_noisy_preamps()]
    return


def dump_SkimDecision (p, f):
    print >> f, p.getName(), p.isAccepted(),
    return


@nolist
def dump_RawInfoSummaryForTag (p, f):
    for a in dir(p):
        if a.startswith ('get'):
            print >> f, a[3:]+':', getattr(p, a)()
    return


@nolist
def dump_MissingETComposition (m, f):
    for p in toiter1 (m):
        print >> f, '  ', p.__class__.__name__,
        if not p:
            print >> f, '(null)',
            continue
        dump_Fourvec (p, f)
        try:
            w = m.getWeight (p)
            print >> f, ' [', w.wet(), w.wpx(), w.wpy(), w.statusWord(), ']',
        except RuntimeError:
            pass
        print >> f, ''
    return

@nolist
def dump_ChamberT0s (m, f):
    for p in m.getAllT0s():
        print >> f, '   ', p.first.getString(), p.second
    return


def dump_TrigMuonClusterFeature (m, f):
    dump_Fourvec (m, f)
    print >> f, m.getNRoi(), m.getNTRK(), m.getNJet(),
    return


def dump_TrigPassBits (b, f):
    print >> f, [b.isPassing(i) for i in range(b.size())],
    return


def dump_TrigVertexCounts (v, f):
    print >> f, list(v.vtxNtrks()), list(v.vtxTrkPtSqSum()),
    return

def dump_TrigMuonEFIsolation (m, f):
    print >> f, m.sumTrkPtCone02(), m.sumTrkPtCone03(), m.trackPosition(),
    return


def dump_TrigPassFlags (m, f):
    # The interface changed, sigh...
    if hasattr (m, 'getNumObjects'):
        for i in range (m.getNumObjects()):
            print >> f, '\n', i, list(m.getFlag(i,j) for j in range(m.getNumFlags(i))),
    else:
        for i in range (m.size()):
            print >> f, '\n', i, list(m.getFlagBit(i,j) for j in range(m.flagSize())),
    return


def dump_eflowObject (e, f):
    dump_Fourvec (e, f)
    print >> f, e.eflowType(), e.isValid(), e.d0(), e.z0(), \
          e.getPassEOverPCheck(), e.getIsSubtracted(), \
          e.getIsDuplicated(), e.getCaloRecoStatus().getStatusWord(),
    for i in range(e.numTrack()):
        print >> f, '\n    tk: ',
        dump_Fourvec (e.track(i), f)
    for i in range(e.numClus()):
        print >> f, '\n    cl: ',
        dump_Fourvec (e.clus(i), f)
    if e.muonLink().isValid():
        print >> f, '\n    mu: ',
        dump_Fourvec (e.muon(), f)
    if e.conversionLink().isValid():
        print >> f, '\n    cv: ',
        dump_Fourvec (e.conversion(), f)
    return


@nolist
def dump_eflowObjectContainer (c, f):
    print >> f, c.missET(), c.sumET(), c.pTx(), c.pTy(), c.numEf(), \
          c.numPhot(), c.numEle(), c.numMuo(), c.isValid(),\
          c.circularity(), c.thrust(), c.oblateness()
    dump_list (c, f, dump_eflowObject)
    return

@nolist
def dump_RecoTimingObj (c, f):
    print >> f, list (c)
    return


@nolist
def dump_TrigConfKeys (p, f):
    print >> f, p.smk(), p.l1psk(), p.hltpsk(),
    return



def format_int(x): return '%d'%x
def format_float(x): return '%.1f'%x
def format_float_vector(v): return ''.join ([format_float(x) for x in v])
def format_el(x):
    if x.isDefaultIndex():
        return '(null)'
    return '%s[%d]' % (x.dataID(), x.index())
char_accessor_ = getattr (ROOT, 'SG::AuxElement::ConstAccessor<char>')
class char_accessor:
    def __init__ (self, name):
        self.ac = char_accessor_ (name)
        return
    def __call__ (self, x):
        return ord(self.ac(x))
uchar_accessor_ = getattr (ROOT, 'SG::AuxElement::ConstAccessor<unsigned char>')
class uchar_accessor:
    def __init__ (self, name):
        self.ac = uchar_accessor_ (name)
        return
    def __call__ (self, x):
        return ord(self.ac(x))
tlist = ['int',
         'short',
         'unsigned int',
         'unsigned short',
         'unsigned long',
         'unsigned long long',
         'std::vector<float>',
         'float',
         'double']
accessors = {
    'char' : char_accessor,
    'unsigned char' : uchar_accessor,
    }
for t in tlist:
    aname = 'SG::AuxElement::ConstAccessor<' + t
    if t[-1] == '>': aname += ' '
    aname += '>'
    accessors[t] = getattr (ROOT, aname)


def format_obj (x, name=None):
    if type(x) == type(1.5):
        return format_float (x)
    if type(x) == type(1):
        return format_int (x)
    tname = type(x).__name__
    if tname.startswith ('ROOT.'):
        tname = tname[5:]
    if tname.startswith ('ElementLink<'):
        return format_el (x)
    if tname.startswith ('std::vector<') or tname.startswith ('vector<'):
        ipos = tname.find('<')
        tname2 = tname[ipos+1:]
        if tname2.startswith('char,') or tname2.startswith ('unsigned char,'):
            l = ', '.join ([str(ord(xx)) for xx in x])
        else:
            l = ', '.join ([format_obj(xx, name) for xx in list(x)])
        return '[' + l + ']'
    if type(x) == PyAthena.xAOD.CaloClusterBadChannelData_v1:
        return '<BadChannel: %6.3f/%6.3f/%2d: %04x>' % \
               (x.eta(), x.phi(), x.layer(), x.badChannel())
    return str(x)


def generic_dump_auxitem (x, auxid, f):
    reg=ROOT.SG.AuxTypeRegistry.instance()
    tname = reg.getTypeName (auxid)
    ac = ROOT.SG.AuxElement.TypelessConstAccessor (reg.getName(auxid))
    try:
        buf = ac(x)
    except TypeError:
        print >> f, '<unavailable>',
        return
    try:
        obj = ROOT.TPython.ObjectProxy_FromVoidPtr (buf, tname)
    except TypeError:
        print >> f, '<unknown %s>'%tname,
        return
    print >> f, format_obj(obj, reg.getName(auxid)),
    return


def dump_auxitem (x, auxid, f = sys.stdout):
    reg=ROOT.SG.AuxTypeRegistry.instance()
    tname = reg.getTypeName (auxid)
    ac_cl = accessors.get (tname)
    #print x, auxid, reg.getName(auxid)
    if ac_cl:
        ac = ac_cl(reg.getName(auxid))
        try:
            val = ac(x)
        except TypeError:
            # Can happen due to schema evolution
            val = '<unavailable>'
        print >> f,  format_obj(val),
    else:
        generic_dump_auxitem (x, auxid, f)
    return


def dump_auxdata (x, exclude=[], f = sys.stdout):
    reg=ROOT.SG.AuxTypeRegistry.instance()
    auxids = list(x.getAuxIDs())
    auxids = [(reg.getName(id), id) for id in auxids]
    auxids.sort()
    #print >> f, auxids,
    for name, auxid in auxids:
        if name in exclude: continue
        print >> f,  name + ': ',
        dump_auxitem (x, auxid, f)
        print >> f, '\n    ',
    return


def dump_xAOD(o, f):
    print >> f, o.__class__.__name__, '\n    ',
    dump_auxdata (o, f=f)
    return


@nolist
def dump_xAODObject(o, f):
    dump_xAOD(o, f)
    return


def dump_list (l, f, dumper):
    for x in l:
        print >> f, '  ',
        dumper (x, f)
        f.write ('\n')
    return


##############################################################################

"""The list of tuples (cpp_type, dumper_fct)
"""
dumpspecs = [
    ["ElectronContainer",                    dump_Electron],
    ["PhotonContainer",                      dump_Photon],
    ["Analysis::MuonContainer",              dump_Muon],
    ["Analysis::TauJetContainer",            dump_TauJet],
    ["TruthParticleContainer",               dump_TruthParticle],
    ["ParticleJetContainer",                 dump_ParticleJet],
    ["Rec::TrackParticleContainer",          dump_TrackParticle],
    ["MissingET",                            dump_MissingET],
    ["MissingEtCalo",                        dump_MissingEtCalo],
    ["MissingEtTruth",                       dump_MissingEtTruth],
    ["CaloClusterContainer",                 dump_CaloClusters_sorted],
    ["VxContainer",                          dump_VxCandidate],
    ["EventInfo",                            dump_EventInfo],
    ["PileUpEventInfo",                      dump_PileUpEventInfo],
    ["McEventCollection",                    dump_GenEvent],
    ["CTP_Decision",                         dump_CTP_Decision],
    ["LVL1_ROI",                             dump_LVL1_ROI],
    ["TrigDec::TrigDecision",                dump_TrigDecision],
    ["TrigConf::HLTAODConfigData",           dump_HLTAODConfigData],
    ["TrigConf::Lvl1AODConfigData",          dump_Lvl1AODConfigData],
    ["TrigConf::Lvl1AODPrescaleConfigData",  dump_Lvl1AODPrescaleConfigData],
    ["TrackParticleTruthCollection",         dump_TrackParticleTruthCollection],
    ["DetailedTrackTruthCollection",         dump_DetailedTrackTruthCollection],
    ["Rec::MuonSpShowerContainer",           dump_MuonSpShower],
    ["HLT::HLTResult",                       dump_HLTResult],
    ["MissingETSig",                         dump_MissingETSig],
    ["INav4MomAssocs",                       dump_INav4MomAssocs],
    ["TrigInDetTrackTruthMap",               dump_TrigInDetTrackTruthMap],
    ["MissingETSigObjContainer",             dump_MissingETSigObject],
    ["DetStatusMap",                         dump_DetStatusMap],
    ["AthenaHitsVector<TrackRecord>",        dump_TrackRecord],
    ["MissingETSigHypoContainer",            dump_MissingETSigHypo],
    ["JetCollection",                        dump_Jet],
    ["V0Container",                          dump_V0Candidate],
    ["egammaContainer",                      dump_egamma],
    ["DataVector<TrigVertex>",               dump_TrigVertex],
    ["TrigVertexCollection",                 dump_TrigVertex],
    ["TrigTrackCounts",                      dump_TrigTrackCounts_nolist],
    ["TrigTau",                              dump_TrigTau_nolist],
    ["DataVector<TrigElectron>",             dump_TrigElectron],
    ["DataVector<TrigPhoton>",               dump_TrigPhoton],
    ["TrigEMCluster",                        dump_TrigEMCluster_nolist],
    ["TrigInDetTrackCollection",             dump_TrigInDetTrackCollection],
    ["DataVector<TrigEFBjet>",               dump_TrigEFBjet],
    ["DataVector<TrigEFBphys>",              dump_TrigEFBphys],
    ["DataVector<TrigL2Bjet>",               dump_TrigL2Bjet],
    ["DataVector<TrigL2Bphys>",              dump_TrigL2Bphys],
    ["TrigCaloClusterContainer",             dump_TrigCaloCluster],
    ["TrigMissingET",                        dump_TrigMissingET_nolist],
    ["TrigRoiDescriptor",                    dump_TrigRoiDescriptor_nolist],
    ["TrigT2Jet",                            dump_TrigT2Jet_nolist],
    ["TrigTauCluster",                       dump_TrigTauCluster_nolist],
    ["TrigTauTracksInfo",                    dump_TrigTauTracksInfo_nolist],
    ["TrigSpacePointCounts",                 dump_TrigSpacePointCounts_nolist],
    ["CombinedMuonFeature",                  dump_CombinedMuonFeature_nolist],
    ["MuonFeature",                          dump_MuonFeature_nolist],
    ["TrigMuonEFContainer",                  dump_TrigMuonEF],
    ["TileContainer<TileMu>",                dump_TileMu],
    ["CaloShowerContainer",                  dump_CaloShower],
    ["CaloTowerContainer",                   dump_CaloTowerContainer],
    ["CaloTopoTowerContainer",               dump_CaloTopoTowerContainer],
    ["CaloCalibrationHitContainer",          dump_CaloCalibrationHit],
    ["CombinedMuonFeatureContainer",         dump_CombinedMuonFeature],
    ["MuonFeatureContainer",                 dump_MuonFeature],
    ["TrigEFBjetContainer",                  dump_TrigEFBjet],
    ["TrigEFBphysContainer",                 dump_TrigEFBphys],
    ["TrigEMClusterContainer",               dump_TrigEMCluster],
    ["TrigElectronContainer",                dump_TrigElectron],
    ["TrigL2BjetContainer",                  dump_TrigL2Bjet],
    ["TrigL2BphysContainer",                 dump_TrigL2Bphys],
    ["TrigMissingETContainer",               dump_TrigMissingET],
    ["TrigPhotonContainer",                  dump_TrigPhoton],
    ["TrigT2JetContainer",                   dump_TrigT2Jet],
    ["TrigTauClusterContainer",              dump_TrigTauCluster],
    ["TrigTauContainer",                     dump_TrigTau],
    ["TrigTauTracksInfoCollection",          dump_TrigTauTracksInfo],
    ["TrigSpacePointCountsCollection",       dump_TrigSpacePointCounts],
    ["TrigTrackCountsCollection",            dump_TrigTrackCounts],
    ["TrigT2MbtsBitsContainer",              dump_TrigT2MbtsBits],
    ["TrigRoiDescriptorCollection",          dump_TrigRoiDescriptor],
    ["TileMuFeatureContainer",               dump_TileMuFeature],
    ["TileTrackMuFeatureContainer",          dump_TileTrackMuFeature],
    ["CosmicMuonCollection",                 dump_CosmicMuon],
    ["MdtTrackSegmentCollection",            dump_MdtTrackSegment],
    ["IsoMuonFeatureContainer",              dump_IsoMuonFeature],
    ["TrigMuonEFInfoContainer",              dump_TrigMuonEFInfo],
    ["RingerRingsContainer",                 dump_RingerRings],
    ["TrigTrtHitCountsCollection",           dump_TrigTrtHitCounts],
    ["MuonFeatureDetailsContainer",          dump_MuonFeatureDetails],
    ["TrigRNNOutputContainer",               dump_TrigRNNOutput],
    ["InDet::InDetLowBetaContainer",         dump_InDetLowBetaCandidate],
    ["LArNoisyROSummary",                    dump_LArNoisyROSummary],
    ["SkimDecisionCollection",               dump_SkimDecision],
    ["RawInfoSummaryForTag",                 dump_RawInfoSummaryForTag],
    ["MissingETComposition",                 dump_MissingETComposition],
    ["Muon::ChamberT0s",                     dump_ChamberT0s],
    ["TrigMuonClusterFeatureContainer",      dump_TrigMuonClusterFeature],
    ["TrigPassBitsCollection",               dump_TrigPassBits],
    ["TrigVertexCountsCollection",           dump_TrigVertexCounts],
    ["TrigMuonEFIsolationContainer",         dump_TrigMuonEFIsolation],
    ["eflowObjectContainer",                 dump_eflowObjectContainer],
    ["TrigPassFlagsCollection",              dump_TrigPassFlags],
    ["TrackParticleAssocs",                  dump_TrackParticleAssocs],
    ["ElectronMuonTopoInfoContainer",        dump_ElectronMuonTopoInfo],
    ["RecoTimingObj",                        dump_RecoTimingObj],

    ['DataVector<xAOD::BTagging_v1>',        dump_xAOD],
    ['DataVector<xAOD::CaloCluster_v1>',     dump_xAOD],
    ['DataVector<xAOD::Electron_v1>',        dump_xAOD],
    ['DataVector<xAOD::Photon_v1>',          dump_xAOD],
    ['DataVector<xAOD::Muon_v1>',            dump_xAOD],
    ['DataVector<xAOD::MuonSegment_v1>',     dump_xAOD],
    ['DataVector<xAOD::Jet_v1>',             dump_xAOD],
    ['DataVector<xAOD::TauJet_v1>',          dump_xAOD],
    ['DataVector<xAOD::TrackParticle_v1>',   dump_xAOD],
    ['DataVector<xAOD::Vertex_v1>',          dump_xAOD],
    ['DataVector<xAOD::TruthEvent_v1>',      dump_xAOD],
    ['DataVector<xAOD::TruthParticle_v1>',   dump_xAOD],
    ['DataVector<xAOD::TruthVertex_v1>',     dump_xAOD],
    ['DataVector<xAOD::PFO_v1>',             dump_xAOD],
    ['xAOD::EventInfo_v1',                   dump_xAODObject],
    ['xAOD::MissingETContainer_v1',          dump_xAOD],
    ['xAOD::MissingETComponentMap_v1',       dump_xAOD],
    ['DataVector<xAOD::MuonRoI_v1>',         dump_xAOD],
    ['DataVector<xAOD::TrigBphys_v1>',       dump_xAOD],
    ['DataVector<xAOD::TrigEMCluster_v1>',   dump_xAOD],
    ['DataVector<xAOD::TrigElectron_v1>',    dump_xAOD],
    ['DataVector<xAOD::TrigPhoton_v1>',      dump_xAOD],
    ['DataVector<xAOD::TrigMissingET_v1>',   dump_xAOD],
    ['DataVector<xAOD::EmTauRoI_v1>',        dump_xAOD],
    ['DataVector<xAOD::JetRoI_v1>',          dump_xAOD],
    ['DataVector<xAOD::MuonRoI_v1>',         dump_xAOD],
    ['xAOD::TrigDecision_v1',                dump_xAODObject],
    ['xAOD::TrigConfKeys_v1',                dump_TrigConfKeys],
    ['xAOD::JetEtRoI_v1',                    dump_xAODObject],
    ['xAOD::EnergySumRoI_v1',                dump_xAODObject],

    # Make some of these more compact?

]

## user-friendly interface ----------------------------------------------------
import sys
def get_dumper_fct(klass, ofile=sys.stdout):
    """helper function to automatically retrieve the suitable dumper function
    given the name of a class or the class-type
     @param `klass' a string containing the name of a C++ type or a type
     @param `ofile' a file-like instance where to dump the objects' content
    """
    if isinstance(klass, type):
        # ok
        pass
    elif isinstance(klass, str):
        klass = getattr(PyAthena,klass)
        pass
    else:
        raise TypeError('expected a type or a string')
    global dumpspecs
    dumpers = [ i for i in dumpspecs if i[0] == klass.__name__ ]
    if len(dumpers) != 1:
        raise RuntimeError('no suitable dumper function for class [%s]'%
                           klass.__name__)
    fct = dumpers[0][1]
    nolist = hasattr (fct, 'nolist') and fct.nolist
    from functools import partial as _partial
    if nolist:
        dumper = _partial(fct, f=ofile)
    else:
        dumper = _partial(dump_list, f=ofile, dumper=fct)
    return dumper
