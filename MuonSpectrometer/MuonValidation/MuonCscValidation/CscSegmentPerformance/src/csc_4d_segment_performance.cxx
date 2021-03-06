/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// csc_4d_segment_performance.cxx

// David Adams
// October, 2006
//
// Main program to loop over entries in a 4D simseg tree and find the matching
// entries in a segment tree.
//
// The classes SimsegAccessor and Segment4dAccessor used to access the trees
// are generated byt root. If changes are made, run this program with option
// -g and copy the headers to the include directory CscClusterPerformance.

// To build a exe that only generates the ROOT interface classes.
#undef  GENERATE_ONLY

#include <cstdlib>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include "TFile.h"
#include "TTree.h"
#include "TTreeIndex.h"
#ifndef GENERATE_ONLY
#include "CscSegmentPerformance/SimsegAccessor.h"
#include "CscSegmentPerformance/Segment4dAccessor.h"
#endif
#include "CscSegmentPerformance/SampleMatch.h"
#include "CLHEP/Matrix/SymMatrix.h"
#include <cmath>

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::istringstream;
using std::map;

typedef std::vector<int> EntryList;

namespace {

// Class to hold event number.
class Runevt {
  int m_run;
  int m_evt;
public:
  Runevt(int run, int evt) : m_run(run), m_evt(evt) { }
  int run() const { return m_run; }
  int evt() const { return m_evt; }
  int operator<(const Runevt& rhs) const {
    if ( run() == rhs.run() ) return evt() < rhs.evt();
    return run() < rhs.run();
  }
  int operator==(const Runevt& rhs) const {
    return run() == rhs.run() && evt() == rhs.evt();
  }
};

std::ostream& operator<<(std::ostream& str, const Runevt& rhs) {
  str << rhs.run() << ":" << rhs.evt();
  return str;
}

const int maxchamber = 32;

// Class to identify a chamber.
class Chamber {
  int m_istation;
  int m_zsec;
  int m_phisec;
public:
  static int max() { return maxchamber; }
  // Constructors.
  Chamber(int istation, int zsec, int phisec)
  : m_istation(istation), m_zsec(zsec), m_phisec(phisec) { }
  Chamber(int ichm) {
    if ( ichm > 15 ) {
      m_istation = 2;
      ichm -= 16;
    } else {
      m_istation = 1;
    }
    if ( ichm > 7 ) {
      ichm -= 8;
      m_zsec = 1;
    } else {
      m_zsec = -1;
    }
    m_phisec = ichm+1;
  }
  // Return indices.
  int istation() const { return m_istation; }
  int phisec() const { return m_phisec; }
  int zsec() const { return m_zsec; }
  // Return index in range 0-31.
  int index() {
    return (m_istation-1)*16 + (m_zsec+1)*4 + m_phisec-1;
  }
};
// Output stream.
std::ostream& operator<<(std::ostream& str, const Chamber& rhs) {
  if ( rhs.istation() == 1 ) str << "CSS";
  else str << "CSL";
  if ( rhs.zsec() < 0 ) str << "-";
  else str << "+";
  str << rhs.phisec();
  return str;
}

class RecSeg;

// Measurement dimension and indices.
const int dim = 4;
const int iy = 0;
const int iz = 1;
const int iay = 2;
const int iaz = 3;

// Class to hold simulated segments.
const int errdim = dim*(dim+1)/2;
class SimSeg {
public:
  int issg;
  int ichm;
  double vec[dim];
  double eta;
  double pt;
public:  // methods
  SimSeg() : issg(0), ichm(0), eta(0.0), pt(0.0) {
    for ( int i=0; i<dim; ++i ) vec[i] = 0.0;
  }
};

typedef std::vector<const SimSeg*> SimSegList;

// Calculate ij for error matrix.
int ij(int i, int j) {
  if ( i < j ) return j*(j+1)/2 + i;
  return i*(i+1)/2 + j;
}
  
// Class to hold reconstructed segments.
class RecSeg {
private:
  mutable double m_weight[errdim];
public:
  int irsg;
  int ichm;
  double vec[dim];
  double err[errdim];
  double chsq;
  // Constructor
  RecSeg() : irsg(-1), ichm(-1), chsq(9999.) {
    m_weight[0] = 0.0;
    for ( int i=0; i<dim; ++i ) vec[i] = 0.0;
    for ( int i=0; i<errdim; ++i ) err[i] = 0.0;
  }
  // Return inverse of error matrix
  const double* weight() const {
    if ( ! m_weight[0] ) {
      CLHEP::HepSymMatrix herr(4);
      for ( int i=0; i<dim; ++i ) {
        for ( int j=0; j<=i; ++ j ) {
          herr(i+1,j+1) = err[ij(i,j)];
        }
      }
      int ichk;
      CLHEP::HepSymMatrix hwt = herr.inverse(ichk);
      if ( ichk ) {
        cerr << "Matrix inversion failed" << endl;
        abort();
      }
      for ( int i=0; i<dim; ++i ) {
        for ( int j=0; j<=i; ++ j ) {
          m_weight[ij(i,j)] = hwt(i+1,j+1);
        }
      }
    }
    return m_weight;
  }
};

typedef std::vector<const RecSeg*> RecSegList;

// Distance from reco to sim.
class DistRecSim {
public:
  double operator()(const RecSeg& rseg, const SimSeg& sseg) const {
    double dist2 = 0.0;
    for ( int j=0; j<dim; ++j ) {
      for ( int i=0; i<dim; ++i ) {
        double resi = rseg.vec[i] - sseg.vec[i];
        double resj = rseg.vec[j] - sseg.vec[j];
        double ddist2 = resi * resj * rseg.weight()[ij(i,j)];
        dist2 += ddist2;
        //cout << "  " << i << j << " " << ddist2 << " " << dist2 << endl;
      }
    }
    double dist = sqrt(dist2);
    cout << "DistRecoSim: dist=" << dist << endl;
    return dist;
  }
};
DistRecSim recdist;

// Distance from sim to reco.
class DistSimRec {
public:
  double operator()(const SimSeg& sseg, const RecSeg& rseg) const {
    bool first = true;
    double w[4];
    if ( first ) {
      // Weight matrix.
      double dpos = 1.0;    // Expected error in position
      double len = 77;      // Distance between position measurments.
      double w00 = 1.0/dpos/dpos;
      double w11 = 4.0*len*len/dpos/dpos;
      w[0] = w00;
      w[1] = w00;
      w[2] = w11;
      w[3] = w11;
      first = false;
    }
    double dist2 = 0.0;
    for ( int i=0; i<dim; ++i ) {
      double res = rseg.vec[i] - sseg.vec[i];
      dist2 += res * res * w[i];
    }
    double dist = sqrt(dist2);
    dist = sqrt(dist);
    cout << "DistSimReco: dist=" << dist << endl;
    return dist;
  }
};
DistSimRec simdist;

typedef SampleMatch<SimSeg, RecSeg, SimSegList, RecSegList, DistSimRec, DistRecSim> Match;

}  // end unnamed namespace

int main(int narg, char* argv[]) {
  bool help = false;
  bool generate = false;
  int error = 0;
  int ndump = 0;
  string arg1;
  string arg2;
  // Read option flags.
  int iarg = 0;
  while ( ++iarg<narg && argv[iarg][0] == '-' ) {
    string opt = argv[iarg] + 1;
    if ( opt == "h" ) {
      help = true;
    } else if ( opt == "g" ) {
      generate = true;
    } else if ( opt == "d" ) {
      string sdump = argv[++iarg];
      istringstream ssdump(sdump);
      ssdump >> ndump;
    } else {
      cerr << "Uknown option: -" << opt << endl;
      error = 3;
    }
  }
  // Read the file names.
  if ( !error && !help ) {
    if ( iarg < narg ) {
      arg1 = argv[iarg++];
      if ( iarg < narg ) {
        arg2 = argv[iarg++];
      } else {
        cerr << "Second file name not found" << endl;
        error = 2;
      }
    } else {
      cout << "First file name not found" << endl;
      error = 1;
    }
  }

  if ( help ) {
    cout << "Usage: " << argv[0] << " [-g] simseg_file 4D_segment_file" << endl;
    return error;
  }

  if ( error ) return error;

  // Open simseg file.
  TFile* psfile = new TFile(arg1.c_str(), "READ");
  TTree* pstree = dynamic_cast<TTree*>(psfile->Get("csc_simseg"));
  if ( pstree == 0 ) {
    cout << "Unable to retrieve simseg tree" << endl;
    cerr << "  File: " << arg1 << endl;
    psfile->Print();
    return 3;
  }
  cout << "Simseg tree has " << pstree->GetEntries() << " entries." << endl;
  if ( generate ) {
    // This is generated in the 2D evaluator.
    //cout << "Generating class SimsegAccessor" << endl;
    //pstree->MakeClass("SimsegAccessor");
  } 

  // Open segment file.
  TFile* pcfile = new TFile(arg2.c_str(), "READ");
  TTree* pctree = dynamic_cast<TTree*>(pcfile->Get("csc_4d_segment"));
  if ( pctree == 0 ) {
    cout << "Unable to retrieve segment tree" << endl;
    cerr << "  File: " << arg2 << endl;
    pcfile->Print();
    return 4;
  }
  cout << "Segment tree has " << pctree->GetEntries() << " entries." << endl;
  if ( generate ) {
    cout << "Generating class Segment4dAccessor" << endl;
    pctree->MakeClass("Segment4dAccessor");
    return 0;
  } 

#ifndef GENERATE_ONLY
#define MAXENT 200
#define MAXSTRIP 10

  // Create result tree.
  // ..event
  int run;
  int evt;
  int ient;                  // Number of entries for each event.
  int ntruth;                // # truth for this event
  int nreco;                 // # reco for this event
  int nfound;                // # reco matched to truth for this event
  int nlost;                 // # truth not matched to reco for this event
  int nfake;                 // # reco not matched to truth for this event
  // ..status
  int found[MAXENT];         // True if segment was found
  // ..truth
  float yh[MAXENT];          // Simulation position in local coordinates.
  float zh[MAXENT];
  float ayh[MAXENT];         // Simulation direction atan(dx/dy) in local coordinates.
  float azh[MAXENT];
  float eta[MAXENT];
  float pt[MAXENT];
  // ..reco
  float y[MAXENT];           // Segment paramters. 
  float z[MAXENT];
  float ay[MAXENT];
  float az[MAXENT];
  float dy[MAXENT];          // Segment errors.
  float dz[MAXENT];
  float day[MAXENT];
  float daz[MAXENT];
  float eyz[MAXENT];         // Segment error correlations
  float eyay[MAXENT];
  float eyaz[MAXENT];
  float ezay[MAXENT];
  float ezaz[MAXENT];
  float eayaz[MAXENT];
  float chsq[MAXENT];        // Reconstruction chi-square.
  float drec[MAXENT];        // Reconstructed to simulated distance (error matrix weight)
  float dsim[MAXENT];        // Simulated to reconstructed distance (fixed weight)
  int zsec[MAXENT];          // Z-sector (-1, +1)
  int istation[MAXENT];      // station (1=CSS, 2=CSL)
  int phisec[MAXENT];        // phi-sector (1-8)
  int sector[MAXENT];        // zsec*(2*phisec-istation+1)
  TFile* pfile = new TFile("csc_4d_segperf.root", "RECREATE");
  TTree* ptree = new TTree("csc_4d_segperf", "CSC 4D segment performance");
  ptree->Branch("run",     &run,      "run/I");
  ptree->Branch("evt",     &evt,      "evt/I");
  ptree->Branch("nentry",  &ient,     "nentry/I");
  ptree->Branch("ntruth",  &ntruth,   "ntruth/I");
  ptree->Branch("nreco",   &nreco,    "nreco/I");
  ptree->Branch("nfound",  &nfound,   "nfound/I");
  ptree->Branch("nlost",   &nlost,    "nlost/I");
  ptree->Branch("nfake",   &nfake,    "nfake/I");
  ptree->Branch("found",    found,    "found[nentry]/I");
  ptree->Branch("yh",       yh,       "yh[nentry]");
  ptree->Branch("zh",       zh,       "zh[nentry]");
  ptree->Branch("ayh",      ayh,      "ayh[nentry]");
  ptree->Branch("azh",      azh,      "azh[nentry]");
  ptree->Branch("eta",      eta,      "eta[nentry]");
  ptree->Branch("pt",       pt,       "pt[nentry]");
  ptree->Branch("y",        y,        "y[nentry]");
  ptree->Branch("z",        z,        "z[nentry]");
  ptree->Branch("ay",       ay,       "ay[nentry]");
  ptree->Branch("az",       az,       "az[nentry]");
  ptree->Branch("dy",       dy,       "dy[nentry]");
  ptree->Branch("dz",       dz,       "dz[nentry]");
  ptree->Branch("day",      day,      "day[nentry]");
  ptree->Branch("daz",      daz,      "daz[nentry]");
  ptree->Branch("eyz",      eyz,      "eyz[nentry]");
  ptree->Branch("eyay",     eyay,     "eyay[nentry]");
  ptree->Branch("eyaz",     eyaz,     "eyaz[nentry]");
  ptree->Branch("ezay",     ezay,     "ezay[nentry]");
  ptree->Branch("ezaz",     ezaz,     "ezaz[nentry]");
  ptree->Branch("eayaz",    eayaz,    "eayaz[nentry]");
  ptree->Branch("chsq",     chsq,     "chsq[nentry]");
  ptree->Branch("drec",     drec,     "drec[nentry]");
  ptree->Branch("dsim",     dsim,     "dsim[nentry]");
  ptree->Branch("zsec",     zsec,     "zsec[nentry]/I");
  ptree->Branch("istation", istation, "istation[nentry]/I");
  ptree->Branch("phisec",   phisec,   "phisec[nentry]/I");
  ptree->Branch("sector",   sector,   "sector[nentry]/I");

  SimsegAccessor simseg(pstree);
  Segment4dAccessor segment(pctree);
  //  pctree->BuildIndex("run", "evt");
  
  // Build index for segment tree.
  typedef map<Runevt, int> EvtIndex;
  EvtIndex segment_index;
  for ( int isevt=0; isevt<pstree->GetEntries(); ++isevt ) {
    segment.GetEntry(isevt);
    Runevt re(segment.run, segment.evt);
    segment_index[re] = isevt;
    cout << "Index: " << re << " " << isevt << endl;
  }

  // Assign limits for matching distances.
  double max_simdist = 1000;
  double good_simdist =  10;
  double max_recdist = 10000;
  double good_recdist =  10;
  cout << "Maximum sim distance is " << max_simdist << endl;
  cout << "   Good sim distance is " << good_simdist << endl;
  cout << "Maximum rec distance is " << max_recdist << endl;
  cout << "   Good rec distance is " << good_recdist << endl;

  // Loop over events.
  int nevt = 0;
  int ntot = 0;
  int nfoundtot = 0;
  int nlosttot = 0;
  int nfaketot = 0;
  for ( int isevt=0; isevt<pstree->GetEntries(); ++isevt ) {
    ++nevt;
    simseg.GetEntry(isevt);
    run = simseg.run;
    evt = simseg.evt;
    ient = 0;
    Runevt re(run, evt);
    cout << "Processing run:event " << re << endl;
    ntruth = simseg.nentry;
    // Fetch the segments for this event.
    EvtIndex::const_iterator iisevt = segment_index.find(re);
    if ( iisevt == segment_index.end() ) {
      cout << "  Event not found in segment tree!" << endl;
      continue;
    }
    int jsevt = iisevt->second;
    segment.GetEntry(jsevt);
    nreco = segment.nentry;
    // Fetch sim segments.
    SimSegList simseg_map[maxchamber];
    for ( int issg=0; issg<ntruth; ++issg ) {
      if ( issg > MAXENT ) {
        cout << "Too many sim segments!!!" << endl;
        abort();
      }
      SimSeg* psseg = new SimSeg;
      SimSeg& sseg = *psseg;
      int ichm = Chamber(simseg.istation[issg], simseg.zsec[issg],
                         simseg.phisec[issg]).index();
      sseg.issg = issg;
      sseg.ichm = ichm;
      sseg.vec[iy] = simseg.y[issg];
      sseg.vec[iz] = simseg.z[issg];
      sseg.vec[iay] = simseg.axy[issg];
      sseg.vec[iaz] = simseg.axz[issg];
      sseg.eta = simseg.eta[issg];
      sseg.pt = simseg.pt[issg];
      simseg_map[ichm].push_back(psseg);
    }
    // Fetch reco segments.
    RecSegList recseg_map[maxchamber];
    // Loop over segments.
    for ( int irsg=0; irsg<segment.nentry; ++irsg ) {
      RecSeg* prseg = new RecSeg;
      RecSeg& rseg = *prseg;
      int ichm = Chamber(segment.istation[irsg], segment.zsec[irsg],
                         segment.phisec[irsg]).index();
      rseg.irsg = irsg;
      rseg.ichm = ichm;
      rseg.vec[iy] = segment.y[irsg];
      rseg.vec[iz] = segment.x[irsg];
      rseg.vec[iay] = segment.ay[irsg];
      rseg.vec[iaz] = segment.ax[irsg];
      rseg.err[ij(iy,iy)] = segment.dy[irsg]*segment.dy[irsg];
      rseg.err[ij(iz,iz)] = segment.dx[irsg]*segment.dx[irsg];
      rseg.err[ij(iay,iay)] = segment.day[irsg]*segment.day[irsg];
      rseg.err[ij(iaz,iaz)] = segment.dax[irsg]*segment.dax[irsg];
      rseg.err[ij(iy,iz)] = segment.eyx[irsg];
      rseg.err[ij(iy,iay)] = segment.eyay[irsg];
      rseg.err[ij(iy,iaz)] = segment.eyax[irsg];
      rseg.err[ij(iz,iay)] = segment.exay[irsg];
      rseg.err[ij(iz,iaz)] = segment.exax[irsg];
      rseg.err[ij(iay,iaz)] = segment.eayax[irsg];
      rseg.chsq = segment.chsq[irsg];
      recseg_map[ichm].push_back(prseg);
    }
    // Loop over chambers and look for matches.
    
    nfound = 0;
    nlost = 0;
    nfake = 0;
    for ( int ichm=0; ichm<maxchamber; ++ichm ) {
      Chamber chm(ichm);
      SimSegList& simsegs = simseg_map[ichm];
      ntot += simsegs.size();
      RecSegList& recsegs = recseg_map[ichm];
      if ( simsegs.size() ) {
        // Phi matching
        cout << chm << " has " << simsegs.size() << " simulated and "
             << recsegs.size() << " reconstructed 4D segments." << endl;
        // Perform matching.
        Match mat(simsegs, recsegs, simdist, recdist,
                  max_simdist, good_simdist, max_recdist, good_recdist);
        nfake += mat.fake().size();
        nfaketot += mat.fake().size();
        nfound += mat.found().size();
        nfoundtot += mat.found().size();
        nlost += mat.lost().size();
        nlosttot += mat.lost().size();
        cout << mat << endl;
        const Match::EvalMap& mfound = mat.found();
        const Match::RefDistanceMap& simdist = mat.found_reference_distance();
        const Match::EvalDistanceMap& recdist = mat.found_evaluation_distance();
        // Fill tree variables for this segment.
        for ( SimSegList::const_iterator issg=simsegs.begin();
              issg!=simsegs.end(); ++issg ) {
          const SimSeg* psseg = *issg;
          const SimSeg& sseg = *psseg;
          Match::EvalMap::const_iterator ifnd = mfound.find(psseg);
          const RecSeg* prec = ifnd==mfound.end() ? 0 : ifnd->second;
          istation[ient] = chm.istation();
          zsec[ient] = chm.zsec();
          phisec[ient] = chm.phisec();
          sector[ient] = zsec[ient]* (2*phisec[ient]-istation[ient]+1);
          yh[ient] = sseg.vec[iy];
          zh[ient] = sseg.vec[iz];
          ayh[ient] = sseg.vec[iay];
          azh[ient] = sseg.vec[iaz];
          eta[ient] = sseg.eta;
          pt[ient] = sseg.pt;
          // Rec phi
          if ( prec ) {
            const RecSeg& rseg = *prec;
            found[ient] = true;
            y[ient] = rseg.vec[iy];
            z[ient] = rseg.vec[iz];
            ay[ient] = rseg.vec[iay];
            az[ient] = rseg.vec[iaz];
            dy[ient] = sqrt(rseg.err[ij(iy,iy)]);
            dz[ient] = sqrt(rseg.err[ij(iz,iz)]);
            day[ient] = sqrt(rseg.err[ij(iay,iay)]);
            daz[ient] = sqrt(rseg.err[ij(iaz,iaz)]);
            eyz[ient] = rseg.err[ij(iy,iz)];
            eyay[ient] = rseg.err[ij(iy,iay)];
            eyaz[ient] = rseg.err[ij(iy,iaz)];
            ezay[ient] = rseg.err[ij(iz,iay)];
            ezaz[ient] = rseg.err[ij(iz,iaz)];
            eayaz[ient] = rseg.err[ij(iay,iaz)];
            chsq[ient] = rseg.chsq;
            dsim[ient] = simdist.find(psseg)->second;
            drec[ient] = recdist.find(psseg)->second;
          } else {
            found[ient] = false;
            y[ient] = 0.0;
            z[ient] = 0.0;
            ay[ient] = 0.0;
            az[ient] = 0.0;
            dy[ient] = 0.0;
            dz[ient] = 0.0;
            day[ient] = 0.0;
            daz[ient] = 0.0;
            eyz[ient] = 0.0;
            eyay[ient] = 0.0;
            eyaz[ient] = 0.0;
            ezay[ient] = 0.0;
            ezaz[ient] = 0.0;
            eayaz[ient] = 0.0;
            chsq[ient] = 0.0;
            dsim[ient] = 0.0;
            drec[ient] = 0.0;
          }
          ++ient;
        }  // end loop over sim segs
      } else {
        nfake += recsegs.size();
        nfaketot += recsegs.size();
      }  // end if simsegs.size()
      // Delete local segments.
      for ( RecSegList::const_iterator isg=recsegs.begin();
            isg!=recsegs.end(); ++isg ) delete *isg;
      recsegs.clear();
      for ( SimSegList::const_iterator issg=simsegs.begin();
            issg!=simsegs.end(); ++issg ) delete *issg;
      simsegs.clear();
    }  // End loop over chambers
    // Fill tree for this events.
    ptree->Fill();
  } // End loop over events

  // Display result summary.
  cout << "Efficiency = " << nfoundtot << "/" << ntot << " = ";
  if( ntot != 0 ) cout << double(nfoundtot)/double(ntot) << endl;
  else cout << "nan" << endl;
  cout << "Fakes/event = " << nfaketot << "/" << nevt << " = ";
  if( nevt != 0 ) cout << double(nfaketot)/double(nevt) << endl;
  else cout << "nan" << endl;

  // Write output file.
  pfile->Write();

#endif
  cout << "Done." << endl;
  return 0;
}

// Build the skeleton functions.
#ifndef GENERATE_ONLY
#define SimsegAccessor_cxx
#include "CscSegmentPerformance/SimsegAccessor.h"
void SimsegAccessor::Loop() { }
#define Segment4dAccessor_cxx
#include "CscSegmentPerformance/Segment4dAccessor.h"
void Segment4dAccessor::Loop() { }
#endif
