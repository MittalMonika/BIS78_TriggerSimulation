#!/usr/bin/env python
# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration                   
import sys, glob
import ROOT

ROOT.gStyle.SetOptStat(0)

ACCEPTANCE = 3.173927e-01

import argparse
parser = argparse.ArgumentParser()
parser.add_argument('infile', type=str, help='input HIST file')
parser.add_argument('--out', type=str, help='output ROOT file')

args = parser.parse_args()

infilename = args.infile
infile = ROOT.TFile.Open(infilename, 'READ')

rundir = None
for k in infile.GetListOfKeys():
    if k.GetName().startswith('run_'):
        rundir = k.GetName()
        break
if not rundir:
    print 'Cannot find run directory in input file'
    sys.exit(1)
else:
    print 'Found rundir', rundir

lbdirs = []
for k in infile.Get(rundir).GetListOfKeys():
    if k.GetName().startswith('lb_'):
        lbdirs.append(k.GetName())

print 'Now to dump'
lbnums = sorted([int(_[3:]) for _ in lbdirs])

effcyt = ROOT.TH1F('effcyt', 'Trigger efficiency', lbnums[-1]-lbnums[0]+1, lbnums[0]-0.5, 
               lbnums[-1]+0.5)
effcyr = ROOT.TH1F('effcyr', 'Loose muon reco efficiency', lbnums[-1]-lbnums[0]+1, lbnums[0]-0.5, 
               lbnums[-1]+0.5)
effcya = ROOT.TH1F('effcya', 'Combined acc x efficiency', lbnums[-1]-lbnums[0]+1, lbnums[0]-0.5, 
               lbnums[-1]+0.5)
effcydir = ROOT.TH1F('effcydir', 'Direct acc x efficiency', lbnums[-1]-lbnums[0]+1, lbnums[0]-0.5, 
               lbnums[-1]+0.5)

from array import array
fout = ROOT.TFile(args.out if args.out else '%s_all.root' % rundir[4:], 'RECREATE')
o_run = array('I', [0])
o_lb = array('I', [0])
o_lbwhen = array('d', [0., 0.])
o_z_one = array('f', [0.])
o_z_two = array('f', [0.])
o_trigeff = array('f', [0.])
o_trigeffstat = array('f', [0.])
o_recoeff = array('f', [0.])
o_recoeffstat = array('f', [0.])
o_alleff = array('f', [0.])
o_alleffstat = array('f', [0.])
o_ae = array('f', [0.])
o_aestat = array('f', [0.])
tl = ROOT.TTree( 'lumitree', 'Luminosity tree' )
tl.Branch('run', o_run, 'run/i')
tl.Branch('lb', o_lb, 'lb/i')
tl.Branch('lbwhen', o_lbwhen, 'lbwhen[2]/D')
tl.Branch('z_one', o_z_one, 'z_one/F')
tl.Branch('z_two', o_z_two, 'z_two/F')
tl.Branch('trigeff', o_trigeff, 'trigeff/F')
tl.Branch('trigeffstat', o_trigeffstat, 'trigeffstat/F')
tl.Branch('recoeff', o_recoeff, 'recoeff/F')
tl.Branch('recoeffstat', o_recoeffstat, 'recoeffstat/F')
tl.Branch('alleff', o_alleff, 'alleff/F')
tl.Branch('alleffstat', o_alleffstat, 'alleffstat/F')
tl.Branch('ae', o_ae, 'ae/F')
tl.Branch('aestat', o_aestat, 'aestat/F')


from DQUtils import fetch_iovs
#rset=set(_[0] for _ in rlb)
#print rset
lblb = fetch_iovs("LBLB", runs=int(rundir[4:])).by_run
for lb in sorted(lbdirs):
    h = infile.Get('%s/%s/GLOBAL/DQTGlobalWZFinder/m_mutrigtp_matches' % (rundir, lb))
    hmo = infile.Get('%s/%s/GLOBAL/DQTGlobalWZFinder/m_muloosetp_match_os' % (rundir, lb))
    hms = infile.Get('%s/%s/GLOBAL/DQTGlobalWZFinder/m_muloosetp_match_ss' % (rundir, lb))
    hno = infile.Get('%s/%s/GLOBAL/DQTGlobalWZFinder/m_muloosetp_nomatch_os' % (rundir, lb))
    hns = infile.Get('%s/%s/GLOBAL/DQTGlobalWZFinder/m_muloosetp_nomatch_ss' % (rundir, lb))
    lbnum = int(lb[3:])
    yld = (h[2], h[3])
    ylderr = (h.GetBinError(2), h.GetBinError(3))
    #print yld, ylderr
    A, B = yld
    o_z_one[0], o_z_two[0] = yld
    if B == 0: continue
    eff = 1./(float(A)/B/2.+1.)
    inverrsq = ((1/2./B)*ylderr[0])**2+((A/2./B**2)*ylderr[1])**2
    o_trigeff[0] = eff
    o_trigeffstat[0] = (inverrsq**.5)*(eff**2)
    o_run[0], o_lb[0] = int(rundir[4:]), lbnum
    try:
        iov = lblb[int(rundir[4:])][lbnum-1]
        o_lbwhen[0], o_lbwhen[1] = iov.StartTime/1e9, iov.EndTime/1e9
    except Exception, e:
        o_lbwhen[0], o_lbwhen[1] = 0, 0
    effcyt.SetBinContent(lbnum-lbnums[0]+1, eff)
    effcyt.SetBinError(lbnum-lbnums[0]+1, o_trigeffstat[0])

    def extract(histogram):
        dbl = ROOT.Double()
        rv1 = histogram.IntegralAndError(21, 30, dbl)
        return (rv1, float(dbl))
    matchos, matchoserr = extract(hmo)
    matchss, matchsserr = extract(hms)
    nomatchos, nomatchoserr = extract(hno)
    nomatchss, nomatchsserr = extract(hns)
    print lb
    print ' ->', matchos, matchoserr
    print ' ->', matchss, matchsserr
    print ' ->', nomatchos, nomatchoserr
    print ' ->', nomatchss, nomatchsserr
    A = float(matchos-matchss)
    Aerr = (matchoserr**2+matchsserr**2)**.5
    B = float(nomatchos-nomatchss)
    Berr = (nomatchoserr**2+nomatchsserr**2)**.5
    if B == 0: Berr = 1.
    if A == 0: 
        eff = 1.
        inverrsq = 1.
    else:
        eff = 1./(1.+B/A)
        inverrsq = ((-B/A**2)*Aerr)**2+((1./A)*Berr)**2
    o_recoeff[0] = eff
    o_recoeffstat[0] = (inverrsq**.5)*(eff**2)
    effcyr.SetBinContent(lbnum-lbnums[0]+1, eff)
    effcyr.SetBinError(lbnum-lbnums[0]+1, o_recoeffstat[0])

    o_ae[0] = ACCEPTANCE*(1-(1-o_trigeff[0])**2)*(o_recoeff[0])**2
    o_aestat[0] = ACCEPTANCE*((o_recoeff[0]**2*2*(1-o_trigeff[0])*o_trigeffstat[0])**2+(2*o_recoeff[0]*(1-(1-o_trigeff[0])**2)*o_recoeffstat[0])**2)**.5
    o_alleff[0] = (1-(1-o_trigeff[0])**2)*(o_recoeff[0])**2
    o_alleffstat[0] = ((o_recoeff[0]**2*2*(1-o_trigeff[0])*o_trigeffstat[0])**2+(2*o_recoeff[0]*(1-(1-o_trigeff[0])**2)*o_recoeffstat[0])**2)**.5
    effcya.SetBinContent(lbnum-lbnums[0]+1, o_ae[0])
    effcya.SetBinError(lbnum-lbnums[0]+1, o_aestat[0])


    tl.Fill()
tl.Write()
print 'Done'

c1 = ROOT.TCanvas()
effcya.SetMarkerStyle(21)
effcya.SetMarkerColor(ROOT.kBlue)
effcya.GetYaxis().SetRangeUser(0.27,0.29)
effcya.Draw('PE')
c1.Print('%s_combined_efficiency.eps' % rundir)
fout.WriteTObject(effcya)
fout.Close()

sumweights = infile.Get('%s/GLOBAL/DQTDataFlow/m_sumweights' % rundir)
ctr = infile.Get('%s/GLOBAL/DQTGlobalWZFinder/m_Z_Counter_mu' % rundir)
if sumweights:
    for ibin in xrange(1,sumweights.GetNbinsX()+1):
        o_lb[0] = int(sumweights.GetBinCenter(ibin))
        ctrbin = ctr.FindBin(o_lb[0])
        print ibin, o_lb[0], sumweights[ibin], ctr[ctrbin]
        if sumweights[ibin] == 0: continue
        p = ctr[ctrbin]/sumweights[ibin]
        o_alleff[0]=p
        try:
            o_alleffstat[0]=(p*(1-p))**.5*(sumweights.GetBinError(ibin)/sumweights[ibin])
        except ValueError:
            o_alleffstat[0]=(sumweights.GetBinError(ibin)/sumweights[ibin])
        effcydir.SetBinContent(effcydir.FindBin(o_lb[0]), p)
        effcydir.SetBinError(effcydir.FindBin(o_lb[0]), o_alleffstat[0])

    effcya.GetYaxis().SetRangeUser(0.27,0.31)
    effcya.Draw('PE')
    effcydir.SetMarkerStyle(20)
    effcydir.SetMarkerColor(ROOT.kRed)
    effcydir.Draw('SAME,PE')
    leg=ROOT.TLegend(0.65, 0.7, 0.89, 0.89)
    leg.AddEntry(effcya, 'Predicted A#epsilon', 'PE')
    leg.AddEntry(effcydir, 'Actual A#epsilon', 'PE')
    leg.Draw()
    c1.Print('%s_tp_comparison.eps' % rundir)

    effcyrat=effcydir.Clone()
    effcyrat.Divide(effcya)
    effcyrat.SetTitle('MC Correction Factor')
    effcyrat.SetXTitle('<#mu>')
    effcyrat.Draw('PE')
    effcyrat.Fit('pol1')
    c1.Print('%s_tp_correction.eps' % rundir)
