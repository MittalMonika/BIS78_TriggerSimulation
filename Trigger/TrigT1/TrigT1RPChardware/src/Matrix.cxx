/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include <fstream>
#include <iostream>
#include <cmath>
#include "TrigT1RPChardware/Matrix.h"
#include "TrigT1RPChardware/CMAVERSION.h"

using namespace std;

//----------------------------------------------------------------------------//
 // number of thresholds
const ubit16 Matrix::s_nthres = 3;        
 // number of channels in side=0 and side=1;
const ubit16 Matrix::s_nchan[2]={32,64};  
 // Bunch Crossing period, ns
const float  Matrix::s_BCtime=25.0;
 // Number of DLL cycles
const ubit16 Matrix::s_NDLLCYC=8;
 // DLL period, ns
const float  Matrix::s_DLLtime=s_BCtime/(float)s_NDLLCYC;
 // Number of Bunch-Crossings to be considered
const sbit16 Matrix::s_NBunch= NOBXS;
 // Lenght of the CMA buffers
const sbit16 Matrix::s_nclock=s_NDLLCYC*s_NBunch;
 // ReadOut offset in DLL steps
const sbit16 Matrix::s_ROOffset = 1;
 // number of channel in a group for the timing setting
const sbit16 Matrix::s_timeGroupA=16;
const sbit16 Matrix::s_timeGroupB=8;
 // number of bits for a CMAword word
const sbit16 Matrix::s_wordlen=32;
//----------------------------------------------------------------------------//
Matrix::Matrix(int run, int event, CMAword debug,
               int subsys, int proj, int sect, 
	       int padadd, int lowhig, int add[2],int locadd) 
	       : BaseObject(Hardware,"Matrix")
{
ubit16 df=0; // debug flag address
m_run = run;
m_event=event;
//
// debug flags first
//
m_matrixDebug = debug;

if(m_matrixDebug&1<<df) {
 DISP <<"============================================="<<endl
      <<"Constructor of Matrix called with parameters:"<<endl
      <<subsys<<" "<<proj<<" "<<sect<<" "<<lowhig
      <<" "<<add[0]<<add[1]<<endl
      <<"============================================="<<endl;
 DISP_DEBUG;
}
m_thisBC=0;                       // temporary initialization

m_BCzero=BCZERO;
//m_BCzero=(s_nclock/s_NDLLCYC)/2;      // default initialization of m_BCzero
                                // user setting by setBCzero

// 
//
//                 BCzero
//                   |
//                   |
//                   |
//                   V
//  BCID=-2  BCID=-1 .BCID= 0  BCID=+1  BCID=+2        
//                   .
//                   .
// ********+********+********+********+********
// 01234567 01234567 01234567 01234567 01234567 
//
//
// Matrix attributes
//
m_subsystem=subsys;
m_projection=proj;
m_sector=sect;
m_pad=padadd;
m_lowhigh=lowhig;
//
m_address[0]=add[0];
m_address[1]=add[1];
m_localadd=locadd;
//
// initialize some variables 
//
initDat();
//
// initialize the RPC data structure 
//
initRPCpointers();
//
// initialize the CMAword registers
// 
initRegisters();
//
// initialize the Configuration pointers
//
initPointers();
//
// set default CM parameters configuration
//
setDefaultConfiguration();
}
//-----------------------------------------------------------------------//
Matrix::~Matrix() {
ubit16 df=1;
deleteRPCdata();
if(m_matrixDebug&1<<df) {
 DISP<<"Distructor of Matrix executed "<<endl;
 DISP_DEBUG;
}
}//end-of-Matrix::~Matrix()
//-----------------------------------------------------------------------//
void Matrix::deleteRPCdata() {
ubit16 i;
//ubit16 df=1;
rpcdata *rpcpnt, *rpcpntnext;
//
// delete the dynamic memory allocated by the Matrix object
//
for(i=0;i<2;i++) {
 rpcpnt=m_datarpc[i];
 while (rpcpnt) {
  rpcpntnext=rpcpnt->next;
  delete rpcpnt;
  rpcpnt=rpcpntnext;
 }//end-of-while(rpcpnt)
}//end-of-for(i
//if(rpcpnt) delete rpcpnt;
//
// set DEFAULT parameters for the Coincidence Matrix
//
initRPCpointers();
//
}//end-of-Matrix::deleteRPCdata
//-----------------------------------------------------------------------//
void Matrix::reset() {
//
// initialize some variables 
//
initDat();
//
// delete dynamic memory used for RPC data
//
deleteRPCdata();
//
// initialize the CMAword registers
// 
initRegisters();
//
}//end-of-Matrix::reset
//-----------------------------------------------------------------------//
void Matrix::initRegisters() {
ubit16 i, j, k, l, m;
for(i=0; i<2; i++)      {     // side address
 for(j=0; j<2; j++)      {    // layer address
  for(k=0; k<s_nclock; k++) {   // clock bins
   for(l=0; l<2; l++)      {  // the two words to make 64 bits
    rodat[i][j][k][l]=0;
    m_input[i][j][k][l]=0;
     for(m=0;m<s_nthres;m++)   {
     m_prepr[m][i][j][k][l]=0;
     m_mjori[m][i][j][k][l]=0;
    }//end-of-for(m
   }//end-of-for(l
  }//end-of-for(k
 }//end-of-for(j
}//end-of-for(i
for(i=0; i<s_nthres; i++) {    // thresholds
 for(j=0;j<s_NBunch;j++){ 
  m_trigger[i][j]=0;
 }//end-of-for(j
 for(j=0; j<s_nclock; j++) {   // clock bins
  m_trigg[i][j]=0;
 }//end-of-for(j
}//end-of-for(i
for(i=0;i<s_NBunch;i++) {    // loop on left-right overlapping flags
 for(j=0; j<2; j++){ 
  m_triggerOverlap[i][j]=0;
 }//end-of-for(j
}//end-of-for(i
//
// initialize k-pattern out
//

for(j=0;j<s_nclock;j++) {
 overlapRO[j]           =0;
 highestthRO[j]         =0;
 m_k_pattern[j]           =0;
 k_readout[j]           =0;
 for(i=0;i<2;i++){
  m_triggerOverlapRO[j][i]=0;
 }
}
for(i=0;i<s_NBunch;i++) {
 m_highestth[i]=0;
 m_overlap[i]  =0;
}//end-of-for(i
}//end-of-Matrix::initRegisters
//-----------------------------------------------------------------------//
void Matrix::initRPCpointers() {
//                        
// initialize the rpcdata structure
//
for(ubit16 i=0;i<2;i++) {
 m_datarpc[i]=0;
}//end-of-for(i
}//end-of-Matrix::initRPCpointers
//-----------------------------------------------------------------------//
void Matrix::initPointers() {
//
// initialize the Configuration pointers
//
m_chdly = 0;  // pointer to channel delays
m_width = 0;  // pointer to pulse widths
m_locDi = 0;  // pointer to local Coincidence Direction
m_kRead = 0;  // pointer to threshold pattern for readout
m_roads = 0;  // pointer to the road Matrix settings
m_major = 0;  // pointer to the majority
m_overl = 0;  // pointer to the overlapping channel list
m_geome = 0;  // pointer to "geometry"
}//end-of-Matrix::initPointers
//-----------------------------------------------------------------------//
void  Matrix::initDat() {
// put the current BC at the center of the buffer
m_BCID = -1;
}//end-of-Matrix::initDat
//-----------------------------------------------------------------------//
void  Matrix::setDefaultConfiguration() {
ubit16 df=0; // debug flag address as for constructor
ubit16 i, j, k, l;
//
// set DEFAULT parameters for the Coincidence Matrix
//
//

//
// Coincidence Windows for the the three thresholds
//
for(i=0; i<s_nthres; i++) {
 for(j=0; j<s_nchan[0]; j++) {
  m_trigRoad[i][j][0]=0x00000000;
  m_trigRoad[i][j][1]=0x00000000;
 }//end-of-for(j
}//end-of-for(i
//
// Majority setting
//
m_majorities[0]=2;      // majority values for threshold address 0
m_majorities[1]=2;      // majority values for threshold address 1
m_majorities[2]=2;      // majority values for threshold address 2
//
// threshold to be used for coincidence of the lowpt trigger with the
// external RPC doublet
//
m_lowtohigh  = s_nthres-1;
//
// threshold to be used to provide the trigger data in the readout;
// important for the correct functioning of the LVL2 (muFast)
//
m_toreadout  = 0;       //threshold of the pattern to be sent to the readout
//
// address of the threshold to be considered in overlap (dimuon counting)
//
m_overlapthres = 0;
//
// address of the configuration for the local coincidence
//
m_localDirec[0] = 7;    //majority Direction
m_localDirec[1] = 7;    //majority Direction
//
// default for m_matOverlap
//
for(i=0; i<2; i++)       {m_matOverlap[i]=0;}
//
// default for the signal delay, deadtime and pulse width arrays
//
for(i=0; i<2; i++) {    // side
 for(j=0; j<2; j++) {   // layer
  for(k=0; k<(s_nchan[1]/s_timeGroupB); k++) {  // group
   m_pulseWidth[i][j][k]=8;
  }//end-of-for(k
  for(k=0; k<(s_nchan[1]/s_timeGroupA); k++) {  // group
   m_channDelay[i][j][k]=0;
  }//end-of-for(k
  for(k=0; k<(s_nchan[1]/s_timeGroupB); k++) {  // group
   m_channDeadT[i][j][k]=0;
  }//end-of-for(k  
 }//end-of-for(j
}//end-of-for(i
//
// Masking to 0 and masking to 1
//
for(i=0; i<2; i++) {    // side
 for(j=0; j<2; j++) {   // layer (or "1/2"=0, "2/2"=1 in case of m_channMask1)
  for(k=0; k<s_nchan[1]; k++) {
   m_channMask0[i][j][k]=0;
  }//end-of-for(k
  for(l=0; l<s_nthres; l++) {
   m_channMask1[l][i][j][0]=0;
   m_channMask1[l][i][j][1]=0;
  }//end-of-for(l=0
  m_channReadOutMask[i][j][0]=0;
  m_channReadOutMask[i][j][1]=0;
 }//end-of-for(j
}//end-of-for(i
//
// default for trigger dead time
//
for(k=0; k<(s_nchan[0]/s_timeGroupB); k++) {  // group
  m_trigDeadTime[k]=8;
}//end-of-for(
//
// initialize m_BunchPhase and m_BunchOffset
//
m_BunchPhase= 0;  // use this setting for standard ATLAS simulation;
//m_BunchPhase=-1;  // use this setting with comparison with the HARDWARE; 
                  // value fixed with VHDL comparison 1-7 august 2004.
                  // use with setBCzero(0);
m_BunchOffset=0; // test with hardware; use with setBCzero(0);
//
// default for m_diagonal
//
for(i=0; i<s_nchan[0]; i++) { m_diagonal[i]=0;}
 
if(m_matrixDebug&1<<df) {
    DISP <<"===================================================================="<<endl
         <<"Matrix::setDefaultConfiguration: "
         <<"Default settings have been loaded."<<std::endl
         <<"==================================================================="<<endl;
    DISP_DEBUG;
    dispDefaultConfiguration();
} 
}//end-of-Matrix::setDefaultConfiguration
//-----------------------------------------------------------------------//
void  Matrix::dispDefaultConfiguration() {
ubit16 i,j,k;
//
// Coincidence Windows for the the three thresholds
//
DISP <<"================================="<<std::endl
     <<"Matrix::dispDefaultConfiguration:"<<std::endl
     <<"=================================";
DISP_DEBUG;
DISP <<"--------------------------------"<<std::endl
     <<"+ Coincidence Windows:          "<<std::endl;
DISP_DEBUG;
for(i=0; i<s_nthres; i++) {
 DISP  <<" + Threshold address:"<<i;
 DISP_DEBUG;
 for(j=0; j<s_nchan[0]; j++) {
  DISP  <<"    -Channel address:"<<j
        <<" Window 63-32 "
	<<std::hex<<m_trigRoad[i][j][1]
        <<" Window 31-00 "
	<<          m_trigRoad[i][j][0]<<std::dec;
  DISP_DEBUG;
 }//end-of-for(j
}//end-of-for(i
//
// Majority setting
//
DISP <<"--------------------------------"<<std::endl
     <<"+ Majority addresses:           "<<std::endl
     <<"--------------------------------";
DISP_DEBUG;
for(i=0; i<s_nthres; i++) {
 DISP <<" - threshold address "<<i<<" value "<<m_majorities[i];
 DISP_DEBUG;
}    
//
// threshold to be used for coincidence of the lowpt trigger with the
// external RPC doublet
//
DISP <<"--------------------------------"<<std::endl
     <<"+ Threshold address low-to-high: "<<m_lowtohigh<<std::endl
     <<"+ Threshold address low-to-readout: "<<m_toreadout<<std::endl
     <<"+ Threshold address for overlap: "<<m_overlapthres;
DISP_DEBUG;
//
// address of the configuration for the local coincidence
//
DISP <<"--------------------------------"<<std::endl
     <<"+ Local coincidence setting:    "<<std::endl
     <<" -Pivot Plane:       "<<m_localDirec[0]<<std::endl
     <<" -Coincidence Plane: "<<m_localDirec[1];
DISP_DEBUG;
//
// Overlap default masking
//
DISP <<"--------------------------------"<<std::endl
     <<"+ Overlap mask setting:    "<<std::endl
     <<" -`right' address          "<<m_matOverlap[0]<<std::endl
     <<" -`left ' address          "<<m_matOverlap[1];
DISP_DEBUG;
//
// default for the signal delay, deadtime and pulse width arrays
//
DISP <<"-----------------------------------------------"<<std::endl
     <<"+ Channel pulse-width, delay and deadtime :    ";
DISP_DEBUG;
for(i=0; i<2; i++) {    // side
 if(!i) {
  DISP<<" +Pivot Plane ";
  DISP_DEBUG;
 } else {
  DISP<<" +Coincidence Plane ";
  DISP_DEBUG;
 }
 for(j=0; j<2; j++) {   // layer
  DISP<<"  +Layer "<<j;
  DISP_DEBUG;
  for(k=0; k<(s_nchan[i]/s_timeGroupB); k++) {  // group
   DISP <<"   -group "<<k
        <<" pulsewidth "<<m_pulseWidth[i][j][k];
   DISP_DEBUG;
  }//end-of-for(k
  for(k=0; k<(s_nchan[i]/s_timeGroupA); k++) {  // group
   DISP <<"   -group "<<k
	<<" delay "<<m_channDelay[i][j][k];
   DISP_DEBUG;
  }//end-of-for(k
  for(k=0; k<(s_nchan[i]/s_timeGroupB); k++) {  // group
   DISP <<"   -group "<<k
	<<" deadtime "<<m_channDeadT[i][j][k];
   DISP_DEBUG;
  }//end-of-for(k
 }//end-of-for(j
}//end-of-for(i
//
// Masking to 0
//
DISP <<"-----------------------------------------------"<<std::endl
     <<"+ Map of the masked-to-0 channels :    ";
DISP_DEBUG;
for(i=0; i<2; i++) {    // side
 if(!i) {
  DISP<<" +Pivot Plane ";
  DISP_DEBUG;
 } else {
  DISP<<" +Coincidence Plane ";
  DISP_DEBUG;
 }
 for(j=0; j<2; j++) {   // layer
  DISP<<"  +Layer "<<j;
  DISP_DEBUG;
  for(k=0; k<s_nchan[1]; k++) {
   if(m_channMask0[i][j][k]) {
    DISP<<"   -channel "<<k;
    DISP_DEBUG;
   }//end-of-if
  }//end-of-for(k
 }//end-of-for(j
}//end-of-for(i
//
// default for trigger dead time
//
DISP <<"-----------------------------------------------"<<std::endl
     <<"+ Trigger Dead Time   ";
DISP_DEBUG;
for(k=0; k<(s_nchan[0]/s_timeGroupB); k++) {  // group
 DISP <<" -group "<<k
      <<" Trigger DeadTime "<<m_trigDeadTime[k];
 DISP_DEBUG;
}//end-of-for(
//
// Simulation relevant parameters (used to align with the hardware)
//
DISP <<"-----------------------------------------------"<<std::endl
     <<"+ Simulation parameters to align with the hardware (not used in CM)";
DISP_DEBUG;
//
// m_BunchPhase and m_BunchOffset
//
DISP <<"+BunchPhase  "<<m_BunchPhase<<std::endl
     <<"  BunchPhase =  0 : to be used for standard ATLAS LVL1 simulation"<<std::endl
     <<"  BunchPhase = -1 : to be used to compare with the hardware; "<<std::endl
     <<"                    this value fixed with VHDL comparison 1-7 august 2004.";  
DISP_DEBUG;
DISP <<"+BunchOffset  "<<m_BunchOffset<<std::endl
     <<"  BunchOffset = 0 : to test with hardware; use this setting with setBCzero(0).";
DISP_DEBUG;
//
// the end
//
DISP <<"======================================"<<std::endl
     <<"Matrix::dispDefaultConfiguration: Done"<<std::endl
     <<"======================================";
DISP_DEBUG;
}//end-of-Matrix::dispDefaultConfiguration
//-----------------------------------------------------------------------//
void  Matrix::putData (int sidemat, int layer, int stripaddress, float time){
//
sbit16 BCID;             // BC  time bin (from 0)
ubit16 DLLID;            // DLL time bin (from 0)
ubit16 df=2;             // debug flag address
rpcdata *rpcpntnew;	
if(m_matrixDebug&1<<df) {
 DISP<<"Matrix:putData: putting data on Matrix"<<endl;
 DISP_DEBUG;
}   
//
   BCID = (int)(time/s_BCtime);
   if(time<0.0) BCID--; // to cope with negative times
//
// the next line determines the DLL clock bin associated
// to the given time. For historical reasons, a +1 offset has
// been used so far:
// DLLID= (int)((time-(float)BCID*s_BCtime)/s_DLLtime)+1;
// It should be now removed. Aleandro, 11 November 2008.
//
   DLLID= (int)((time-(float)BCID*s_BCtime)/s_DLLtime);
   if(DLLID==s_NDLLCYC) {
    BCID++;
    DLLID=0;
   }//end-of-if(DLLID
//
// put this digi in the dynamic store
//
// coverity change
//   rpcpntnew = new rpcdata;
//
// check the stripaddress is consistent with the Matrix dimension
//
if (BCID>=NOBXS) return;

   if( stripaddress>=0 && stripaddress < s_nchan[sidemat] )  {  
  // coverity change
   rpcpntnew = new rpcdata;

   if(!rpcpntnew) {  
    DISP<<"=========================="<<endl
        <<"=   Matrix::putData;     ="<<endl
        <<"=   new pointer for      ="<<endl
        <<"=   rpcpntnew;           ="<<endl
        <<"=   No Memory Available  ="<<endl
        <<"=========================="<<endl;
    DISP_ERROR;
    exit(1);
  }//end-of-if(!rpcpntnew)
   rpcpntnew->layer    = layer;
   rpcpntnew->stripadd = stripaddress;
   rpcpntnew->time     = time;
   rpcpntnew->BC       = BCID;
   rpcpntnew->DLL      = DLLID;
   rpcpntnew->masked   = 0;
   rpcpntnew->maskto1  = 0;
   rpcpntnew->deadtime = 0;
   rpcpntnew->delay    = 0;
   rpcpntnew->mark     = 0;
   rpcpntnew->next     = m_datarpc[sidemat];
// 
// put this element as first in the queue
//
   m_datarpc[sidemat] = rpcpntnew;  

   } else {
    DISP<<" Matrix::putData failure: channel addressed is "
        <<stripaddress<<" for matrix side "<<sidemat<<endl;
    DISP_ERROR;
   }//end-of-if
}//end-of-putData
//----------------------------------------------------------------------//
void Matrix::putPatt (const Matrix *p) {
//
// input:  p  pointer of the low-pt Matrix belonging to the
//            high-pt one
//
CMAword k;
ubit16 i, j;
ubit16 df=3; // debug flag address
if(m_matrixDebug&1<<df) {
 DISP<<" method putPatt called; p is "<<p<<endl;
 DISP_DEBUG;
}
//
//
// for(i=0; i<2; i++)           {     // loop on the two majorities
//  for(j=0; j<s_nclock; j++)     {     // loop on the clock bins
//   for(k=0; k<2; k++)           {   // loop on buffer words
//    for(n=0; n<s_nthres; n++) {       // loop on the "s_nthres" registers
//     m_mjori[n][0][i][j][k] = p->m_k_pattern[j];
//    }//end-of-for(n
//   }//end-of-for(k
//  }//end-of-for(j
// }//end-of-for(i
//
//
float time=0.0;
k=1;

//   cout<<" RITARDI-1 "<<endl;
//   for(int iside=0;iside<=1;iside++){
//     for(int ilayer=0;ilayer<=1;ilayer++){
//       for(ubit16 k=0; k<(s_nchan[iside]/s_timeGroupA); k++) {
//         cout<<" side="<<iside<<" layer="<<ilayer<<" group="<<k
//  	   <<" delay="<<m_channDelay[iside][ilayer][k]<<endl;
//       }
//     }
//   }//end-of-for(ubit16 k



for(i=0; i<s_nchan[0]; i++) {
 for(j=0; j<s_nclock; j++) {
  if( p->m_k_pattern[j] & k ) {
   time = ((float)j+0.5)*s_DLLtime
        + (float)m_thisBC*s_BCtime
        - (float)(m_BCzero)*s_BCtime;
   putData(0,0,i,time);
  }//end-of-if(p->m_k_pattern
 }//end-of-for(j
 k=k<<1;
}//end-of-for(i
//
if(m_matrixDebug&1<<df) {
 DISP<<" copy_pivot; input matrix address "<<p<<endl;
 DISP_DEBUG;
}//end-of-if(m_matrixDebug&1<<df)
}//end-of-method putPatt
//----------------------------------------------------------------------//
void Matrix::setRunEvent(int runNum, int eventNum) {
 m_run=runNum;
 m_event=eventNum;
}//end-of-Matrix::setRunEvent
//----------------------------------------------------------------------//
void Matrix::setBCzero (ubit16 offset) {
//
// set the BunchCrossing=0 to the "offset" array address in memory
//
if(offset<=s_NBunch-1) m_BCzero=offset;
 else m_BCzero=s_NBunch-1;
}//end-of-setBCzero
//----------------------------------------------------------------------//
void Matrix::setDeadTime (ubit16 deadt) {
for(ubit16 iside=0; iside<2; iside++) {
 for(ubit16 ilayer=0; ilayer<2; ilayer++) {
  for(ubit16 k=0; k<(s_nchan[iside]/s_timeGroupB); k++) {
   setDeadTime(iside,ilayer,k,deadt);
  }//end-of-for(ubit16 k
 }//end-of-for(ubit16 ilayer
}//end-of-for(ubit16 iside
}//end-of-setDeadTime
//----------------------------------------------------------------------//
void Matrix::setDeadTime (ubit16 iside, ubit16 ilayer, 
                          ubit16 deadt) {
for(ubit16 k=0; k<(s_nchan[iside]/s_timeGroupB); k++) {
 setDeadTime(iside,ilayer,k,deadt);
}//end-of-for(ubit16 k
}//end-of-setDeadTime
//----------------------------------------------------------------------//
void Matrix::setDeadTime (ubit16 iside, ubit16 ilayer, ubit16 igroup, 
                          ubit16 deadt) {
if(iside<2 && ilayer<2 && igroup<(s_nchan[1]/s_timeGroupB)) {
 m_channDeadT[iside][ilayer][igroup]=deadt;
 if(iside==0 && igroup>3) {
  DISP<<" Matrix::setDeadTime: problems with side and group addresses"<<endl
      <<" Matrix::setDeadTime: side="<<iside<<" layer="<<ilayer
      <<" group="<<igroup<<endl;
  DISP_ERROR;
 }//end-of-if
} else {
 DISP<<" Matrix::setDeadTime: problems in adressing pulseWidth"<<endl
     <<" Matrix::setDeadTime: side="<<iside<<" layer="<<ilayer
     <<" group="<<igroup<<endl;
 DISP_ERROR;
}//end-of-if
}//end-of-setDeadTime
//----------------------------------------------------------------------//
void Matrix::setDelay (ubit16 iside, ubit16 ilayer, 
                       ubit16 delay) {
if(iside > 1 || ilayer > 1 ) {
   DISP<<" Matrix::setDelay: problems with side and layer addresses"<<endl
       <<" Matrix::setDelay: side="<<iside<<" layer="<<ilayer<<endl;
   DISP_ERROR;
} else {
  for(ubit16 k=0; k<(s_nchan[iside]/s_timeGroupA); k++) {
   setDelay(iside,ilayer,k,delay);
  }//end-of-for(ubit16 k
}
}//end-of-setDelay
//----------------------------------------------------------------------//
void Matrix::setDelay (ubit16 iside, ubit16 ilayer, ubit16 igroup, 
                       ubit16 delay) {
if(iside<2 && ilayer<2 && igroup<(s_nchan[1]/s_timeGroupA)) {
 m_channDelay[iside][ilayer][igroup]=delay;
 if(iside==0 && igroup>3) {
  DISP<<" Matrix::setDelay: problems with side and group addresses"<<endl
      <<" Matrix::setDelay: side="<<iside<<" layer="<<ilayer
      <<" group="<<igroup<<endl;
  DISP_ERROR;
 }//end-of-if
} else {
 DISP<<" Matrix::setDelay: problems in adressing pulseWidth"<<endl
     <<" Matrix::setDelay: side="<<iside<<" layer="<<ilayer
     <<" group="<<igroup<<endl;
 DISP_ERROR;
}//end-of-if
}//end-of-setDelay
//----------------------------------------------------------------------//
void Matrix::setPulseWidth (ubit16 length) {
for(ubit16 iside=0; iside<2; iside++) {
 for(ubit16 ilayer=0; ilayer<2; ilayer++) {
  for(ubit16 k=0; k<(s_nchan[iside]/s_timeGroupB); k++) {
   setPulseWidth(iside,ilayer,k,length);
  }//end-of-for(ubit16 k
 }//end-of-for(ubit16 ilayer
}//end-of-for(ubit16 iside
}//end-of-setPulseWidth
//----------------------------------------------------------------------//
void Matrix::setPulseWidth (ubit16 iside, ubit16 ilayer, 
                            ubit16 length) {
for(ubit16 k=0; k<(s_nchan[iside]/s_timeGroupB); k++) {
 setPulseWidth(iside,ilayer,k,length);
}//end-of-for(ubit16 k
}//end-of-setPulseWidth
//----------------------------------------------------------------------//
void Matrix::setPulseWidth (ubit16 iside, ubit16 ilayer, ubit16 igroup, 
                            ubit16 length) {
if(iside<2 && ilayer<2 && igroup<s_nchan[1]/s_timeGroupB) {
 m_pulseWidth[iside][ilayer][igroup]=length;
 if(iside==0 && igroup>3) {
  DISP<<" Matrix::setPulseWidth: problems with side and group addresses"<<endl
      <<" Matrix::setPulseWidth: side="<<iside<<" layer="<<ilayer
      <<" group="<<igroup<<endl;
  DISP_ERROR;
 }//end-of-if
} else {
 DISP<<" Matrix::setPulseWidth: problems in adressing pulseWidth"<<endl
     <<" Matrix::setPulseWidth: side="<<iside<<" layer="<<ilayer
     <<" group="<<igroup<<endl;
 DISP_ERROR;
}//end-of-if
}//end-of-setPulseWidth
//----------------------------------------------------------------------//
void Matrix::setMask0 (ubit16 iside, ubit16 ilayer, ubit16 ichannel) {
if(iside>1 || ilayer>1 || ichannel>(s_nchan[iside]-1)) {
 DISP<<" Matrix::setMask0: problems with side/layer/channel addresses"<<endl
     <<" Matrix::setMask0: side="<<iside<<" layer="<<ilayer
     <<" channel="<<ichannel<<endl;
 DISP_ERROR;
} else {
 m_channMask0[iside][ilayer][ichannel]=1;
}
}//end-of-setMask0
//----------------------------------------------------------------------//
void Matrix::setMask1 (ubit16 ithreshold, ubit16 iside, ubit16 imajority, ubit16 ichannel) {
if(ithreshold>2 || iside>1 || imajority>1 || ichannel>(s_nchan[iside]-1)) {
 DISP<<" Matrix::setMask1: problems with side/layer/channel addresses"<<endl
     <<" Matrix::setMask1: threshold= "<<ithreshold<<" side="<<iside<<" majority="<<imajority
     <<" channel="<<ichannel<<endl;
 DISP_ERROR;
} else {
 set_to_1 (&m_channMask1[ithreshold][iside][imajority][0],ichannel);
}
}//end-of-setMask1
//----------------------------------------------------------------------//
void Matrix::setMask1 (ubit16 ithreshold, ubit16 iside, ubit16 imajority) {
for(int ichannel=0; ichannel<s_nchan[iside]; ichannel++) {
 setMask1(ithreshold,iside,imajority,ichannel);
}
}//end-of-setMask1
//----------------------------------------------------------------------//
void Matrix::setMask1 (ubit16 ithreshold, ubit16 iside) {
for(int imajority=0; imajority<2; imajority++) {
 setMask1(ithreshold,iside,imajority);
}
}//end-of-setMask1
//----------------------------------------------------------------------//
void Matrix::setMaskReadOut (ubit16 iside, ubit16 ilayer, ubit16 ichannel) {
if(iside>1 || ilayer>1 || ichannel>(s_nchan[iside]-1)) {
 DISP<<" Matrix::setMaskReadOut: problems with side/layer/channel addresses"<<endl
     <<" Matrix::setMaskReadOut: side= "<<iside<<" layer="<<ilayer<<" channel="<<ichannel<<endl;
 DISP_ERROR;
} else {
 set_to_1 (&m_channReadOutMask[iside][ilayer][0],ichannel);
}
}//end-of-setMaskReadOut
//----------------------------------------------------------------------//
void Matrix::setMaskReadOut (ubit16 iside, ubit16 ilayer, ubit16 ichannel,
                             ubit16 nchannels) {
for(ubit16 i=ichannel; i<(ichannel+nchannels-1); i++) {
 setMaskReadOut (iside, ilayer, ichannel, i);
}
}//end-of-setMaskReadOut
//----------------------------------------------------------------------//
void Matrix::setConfig (int *l, ubit16 *p, int *k, CMAword *r, int *m, 
                      CMAword *o, sbit32 *g) {
//
// input:  l   pointer to local coincidence direction
//         p   pointer to pulse width
//         k   pointer to threshold for k-readout pattern
//         r   pointer to the three Matrix roads;
//         m   pointer to the three majorities;
//         o   pointer to the overlapping channel list;
//         g   pointer to "geometry"
//
ubit16 i, j, jg;
ubit16 addGroupMax=0;
// ubit16 df=4;  // debug flag address 
////////////////////////////////////////////////////////////
m_locDi=l;      // local coincidence direction              //
m_width=p;      // pulse width                              //
m_chdly=0;   // delay                                    //
m_kRead=k;      // k-readout address                        //
m_roads=r;      // programmed coincidence window address    //
m_major=m;      // programmed majority address              //
m_overl=o;      // programmed overlap flag address          //
m_geome=g;      // pre-calculated geometry                  //
////////////////////////////////////////////////////////////
for(i=0; i<2; i++) { setLocalDirection(i,*(m_locDi+i));}
// set Pulse Widths
for(i=0; i<2; i++) {                // side
       addGroupMax=s_nchan[0]/s_timeGroupB;
 if(i) addGroupMax=s_nchan[1]/s_timeGroupB;
 for(j=0; j<2; j++) {               // layer
  for(jg=0; jg<addGroupMax; jg++) { // group
// setPulseWidth(i,j,jg,*(m_width+jg+8*j+16*i));
// setDelay     (i,j,jg,*(m_chdly+jg+8*j+16*i));
  }//end-of-for(jg
 }//end-of-for(j
}//end-of-for(i
// set address of threshold to be readout
setKReadOut(*m_kRead);
// set majority levels
for(i=0;i<s_nthres;i++) { setMajority(i,*(m_major+i));}
// set coincidence roads for the s_nthres thresholds
for(i=0; i<s_nthres; i++) {
 for(j=0; j<s_nchan[0]; j++) {
  setRoad(i,j,0,*(m_roads+s_nchan[0]*2*i+2*j+0));
  setRoad(i,j,1,*(m_roads+s_nchan[0]*2*i+2*j+1));
 }//end-of-for(j
}//end-of-for(i
// set the overlap registers
for(i=0; i<2; i++) { setMatOverlap(i,*(m_overl+i));}
for(i=0; i<s_nchan[0]; i++) { setDiagonal(i,*(m_geome+i));}
}//end-of-Matrix::setConfig
//----------------------------------------------------------------------//
void Matrix::setLocalDirection(ubit16 add, int content) {
if(add>1) {
 DISP<<" Matrix::setLocalDirection :  add= "<<add<<" not valid"<<endl;
 DISP_ERROR;
} else {
 m_localDirec[add]=content;
}//end-of-if;
}//end-of-Matrix::setLocalDirection
//----------------------------------------------------------------------//
void Matrix::setKReadOut(int content) {
if(content<0||content>>2) {
 DISP<<" Matrix::setKReadout :  threshold address = "
     <<content<<" not valid"<<endl;
 DISP_ERROR;
} else {
 m_toreadout=content;
}//end-of-if
}//end-of-Matrix::setKReadOut
//----------------------------------------------------------------------//
void Matrix::setOverlaThres(int content) {
if(content>=0 && content<s_nthres) {
  m_overlapthres = content;
} else {
  m_overlapthres = 0;
}
}//end-of-setOverlapThres
//----------------------------------------------------------------------//
void Matrix::setTrigDeadTime (ubit16 igroup, ubit16 deadt) {
if(igroup<4) {
 m_trigDeadTime[igroup] = deadt;
} else {
 DISP<<" Matrix::setTrigDeadTime :  igroup= "<<igroup<<" not valid"<<endl;
 DISP_ERROR;
}
}//end-of-setTrigDeadTime
//----------------------------------------------------------------------//
void Matrix::setTrigDeadTime (ubit16 deadt) {
for(ubit16 k=0; k<(s_nchan[0]/s_timeGroupB); k++) {
 setTrigDeadTime(k,deadt);
}//end-of-for(ubit16 k
}//end-of-setTrigDeadTime
//----------------------------------------------------------------------//
void Matrix::setMajority(ubit16 add, int content) {
if(add>=s_nthres) {
 DISP<<" Matrix::setMajority :  add= "<<add<<" not valid"<<endl;
 DISP_ERROR;
} else {
 m_majorities[add]=content;
}//end-of-if
}//end-of-Matrix::setMajority
//----------------------------------------------------------------------//
void Matrix::setRoad(ubit16 addThres, ubit16 addChn, ubit16 add64, 
                      CMAword content){
if(addThres>=s_nthres||addChn>s_nchan[0]||add64>1) {
 DISP<<" Matrix::setRoad :  addThres= "<<addThres
     <<" addChn= "<<addChn
     <<" add64= "<<add64<<" not valid"<<endl;
 DISP_ERROR;
} else {
 m_trigRoad[addThres][addChn][add64] = content;
}//end-of-if
}//end-of-Matrix::setRoad
//----------------------------------------------------------------------//
void Matrix::setRoad(ubit16 addThres, ubit16 addChn, char road[17]) {
if(addThres>=s_nthres||addChn>s_nchan[0]) {
 DISP<<" Matrix::setRoad :  addThres= "<<addThres
     <<" addChn= "<<addChn<<endl;
 DISP_ERROR;
} else {
 CMAword the32[2]={0,0};
 ubit16 outflag = char2int(road,the32);
 if(outflag) {
  DISP<<" Matrix::setRoad; outflag from char2int is positive: "
      <<outflag<<endl;
  DISP_ERROR;
  m_trigRoad[addThres][addChn][0] = 0;
  m_trigRoad[addThres][addChn][1] = 0;
 } else {
  m_trigRoad[addThres][addChn][0] = the32[0];
  m_trigRoad[addThres][addChn][1] = the32[1];
 }
}//end-of-if
}//end-of-Matrix::setRoad
//----------------------------------------------------------------------//
void Matrix::setMatOverlap(ubit16 add, CMAword content) {
if(add>1) {
 DISP<<" Matrix::setMatOverlap :  add= "<<add<<" not valid"<<endl;
 DISP_ERROR;
} else {
 m_matOverlap[add]=content;
}
}//end-of-Matrix::setMatOverlap
//----------------------------------------------------------------------//
void Matrix::setDiagonal(ubit16 add, sbit32 content) {
if(add>s_nchan[0]) {
 DISP<<" Matrix::setDiagonal :  add= "<<add<<" not valid"<<endl;
 DISP_ERROR;
} else {
 m_diagonal[add]=content;
}//end-of-if
}//end-of-Matrix::setDiagonal
//----------------------------------------------------------------------//
CMAword Matrix::getMatOverlap(ubit16 add) {
CMAword output=0;
if(add>1) {
 DISP<<" Matrix::getMatOverlap :  add= "<<add<<" not valid"<<endl;
 DISP_ERROR;
} else {
 output=m_matOverlap[add];
}
return output;
}//end-of-Matrix::getMatOverlap
//----------------------------------------------------------------------//
CMAword Matrix::getRoad(ubit16 addThres, 
                                   ubit16 addChn, ubit16 add64){
CMAword output=0;
if(addThres>=s_nthres||addChn>s_nchan[0]||add64>1) {
 DISP<<" Matrix::getRoad :  addThres= "<<addThres
     <<" addChn= "<<addChn
     <<" add64= "<<add64<<" not valid"<<endl;
 DISP_ERROR;
} else {
output=m_trigRoad[addThres][addChn][add64];
}//end-of-if
return output;
}//end-of-Matrix::getRoad
//----------------------------------------------------------------------//
int Matrix::getMajority(ubit16 add) {
int output=0;
if(add>=s_nthres) {
 DISP<<" Matrix::getMajority :  add= "<<add<<" not valid"<<endl;
 DISP_ERROR;
} else {
 output=m_majorities[add];
}//end-of-if
return output;
}//end-of-Matrix::getMajority
//----------------------------------------------------------------------//
void Matrix::execute() {
// returns coincidence flag:  0 if there is no trigger for all thresholds
//                            1 if there is at least one threshold with trigger
//                            2 if the lowtohigh threshold (or higher) is m_trigg
//
//execute this active CM
//
ubit16 df=4;  // debug flag address 
if(m_matrixDebug&1<<df) {
 DISP<<"===================="<<endl
     <<"|                  |"<<endl
     <<"|  Matrix::execute |"<<endl
     <<"|  --------------- |"<<endl
     <<"|                  |"<<endl
     <<"===================="<<endl;
 DISP_DEBUG;
 show_attributes();
}//end-of-if(m_matrixDebug&1<<df)
//
// 0) programmed windows (s_nthres) and time calibration constants for this
//    matrix
if(m_matrixDebug&1<<df) 
 wind();
// 1) store deadtime in dyn. structure; deadtime is applied by "applyDeadTime"
storeDeadtime();
// 2) masking;
masking();
// 3) delay
delay();
// 5) fill the CMA
load();
// 6) apply deadtime response to Input signals
deadTime();
copyDataToReadOut();
// 7) preprocessing
prepro();
// 8) coincidence;
coincide();
//
makeOut();
 
// Code to be activated for testing the VHDL simulation.
//makeOutPattern ();
//
}//end-of-method execute;
//----------------------------------------------------------------------//
void Matrix::storeDeadtime() {
ubit16 df=5;
rpcdata *rpchit;
if(m_matrixDebug&1<<df) {
 DISP<<"--------------------------"<<endl
     <<"|  Matrix::storeDeadtime |"<<endl
     <<"--------------------------"<<endl;
 DISP_DEBUG;
}//end-of-if(m_matrixDebug&1<<df)
for(ubit16 i=0; i<2; i++) {
 rpchit=m_datarpc[i];
 while(rpchit) {
  rpchit->deadtime = 
  m_channDeadT[i][rpchit->layer][rpchit->stripadd/s_timeGroupB];
  rpchit=rpchit->next;
 }//end-of-while(rpchit
}//end-of-for(ubit16 i
}//end-of-Matrix::storeDeadtime
//----------------------------------------------------------------------//
void Matrix::masking() {
ubit16 df=6;
rpcdata *rpchit;
if(m_matrixDebug&1<<df) {
 DISP<<"--------------------"<<endl 
     <<"|  Matrix::masking |"<<endl
     <<"--------------------"<<endl;
 DISP_DEBUG;
}//end-of-if(m_matrixDebug&1<<df)
for(ubit16 i=0; i<2; i++) {
 rpchit=m_datarpc[i];
 while(rpchit) {
  rpchit->masked =
  m_channMask0[i][rpchit->layer][rpchit->stripadd];
  rpchit=rpchit->next;
 }//end-of-while(rpchit
}//end-of-for(ubit16 i
}//end-of-Matrix::masking
//----------------------------------------------------------------------//
void Matrix::delay() {
ubit16 df=7;
rpcdata *rpchit;
if(m_matrixDebug&1<<df) {
 DISP<<"--------------------"<<endl
     <<"|  Matrix::delay   |"<<endl
     <<"--------------------"<<endl;
 DISP_DEBUG;
}//end-of-if(m_matrixDebug&1<<df)
for(ubit16 i=0; i<2; i++) {
 rpchit=m_datarpc[i];
 while(rpchit) {
  rpchit->delay    = 
  m_channDelay[i][rpchit->layer][rpchit->stripadd/s_timeGroupA];
  rpchit=rpchit->next;
 }//end-of-while(rpchit
}//end-of-for(ubit16 i
}//end-of-Matrix::delay
//----------------------------------------------------------------------//
void Matrix::load() {
ubit16 df=8;
sbit32 abs_time, timeadd;
rpcdata *rpcpnt;
//
if(m_matrixDebug&1<<df) {
 DISP<<"--------------------"<<endl
     <<"|  Matrix::load    |"<<endl
     <<"--------------------"<<endl;
 DISP_DEBUG;
}//end-of-if(m_matrixDebug&1<<df)
//
for(ubit16 i=0; i<2; i++) {  // "i" is the CMA side address
 rpcpnt=m_datarpc[i];
 while (rpcpnt) {
  if(m_matrixDebug&1<<df) {
   DISP<<"  Layer= " <<rpcpnt->layer
       <<" stripadd= "<<rpcpnt->stripadd
       <<" time= "<<rpcpnt->time
       <<" mask= "<<rpcpnt->masked
       <<" BC= "<<rpcpnt->BC
       <<" DLL= "<<rpcpnt->DLL
       <<" delay= "<<rpcpnt->delay
       <<endl;
   DISP_DEBUG;
  }//end-of-if(m_matrixDebug&1<<df)
  abs_time = s_NDLLCYC*rpcpnt->BC
           + rpcpnt->DLL
	   + rpcpnt->delay;
  
  timeadd = abs_time - s_NDLLCYC*m_thisBC // synchronize the data to m_thisBC
          + s_NDLLCYC*m_BCzero;         // put m_thisBC at the center of the buffer
                                      
  if(m_matrixDebug&1<<df) {
   DISP<<" abs_time= "<<abs_time<<" timeadd= "<<timeadd<<endl;
   DISP_DEBUG;
  }
  
//
// store this digit if it is within time window and not masked
//   
  if(timeadd>=0 && timeadd<s_nclock && !rpcpnt->masked) {
   if(m_matrixDebug&1<<df) {
    DISP<<" setting input with side "<<i<<" "<<rpcpnt->layer
        <<" "<<timeadd<<" 0"<<" for channel "<<rpcpnt->stripadd
	<<" timeadd "<<timeadd<<endl;
    DISP_DEBUG;
   }//end-of-if(m_matrixDebug&1<<df)
   set_to_1(&m_input[i][rpcpnt->layer][timeadd][0],rpcpnt->stripadd);
  }//end-of-if(timeadd
//
  rpcpnt=rpcpnt->next;
//
 }//end-of-while(rpcpnt)
}//end-of-for(i
}//end-of-Matrix::load()
//------------------------------------------------------------------------//
void Matrix::copyDataToReadOut() {
//
// copy input to rodat
//
for(ubit16 i=0; i<2; i++)      {     // side address
 for(ubit16 j=0; j<2; j++)      {    // layer address
  for(ubit16 k=0; k<s_nclock; k++) {   // clock bins
   for(ubit16 l=0; l<2; l++)      {  // the two words to make 64 bits  
    rodat[i][j][k][l]=m_input[i][j][k][l] & ~m_channReadOutMask[i][j][l];
   }//end-of-for(l
  }//end-of-for(k
 }//end-of-for(j
}//end-of-for(i
}//end-of-copyDataToReadOut
//------------------------------------------------------------------------//
void Matrix::prepro () {
ubit16 df=9;
if(m_matrixDebug&1<<df) {
 DISP<<"-------------------------"<<endl
     <<"| matrix: preprocessing |"<<endl
     <<"-------------------------"<<endl;
 DISP_DEBUG;
}//end-of-if(m_matrixDebug&1<<df)
//
// 1) pulse width
pulse_width();
// 2) declustering
declus();
// 3) majority
majori();
// 4) mask to 1 
maskTo1();
}//end-of-method prepro
//------------------------------------------------------------------------//
void Matrix::coincide () {
//
// input  none
//
// returns trigger flag
//
//
sbit16 BCaddress=0;
//
ubit16 df=10;
ubit16 i,j,l,m;
ubit16 thres, chan, nconf, conf[4][2];
for(i=0;i<4;i++) {
 for(j=0; j<2; j++) {
  conf[i][j]=0;
 }
}
//
if(m_matrixDebug&1<<df) {
 DISP<<"--------------------"<<endl
     <<"| matrix: coincide |"<<endl
     <<"--------------------"<<endl;
 DISP_DEBUG;
}//end-of-if(m_matrixDebug&1<<df)
//
// nconf gives the number of possible configurations compatible with
// the required majority; 
// conf[i][j] gives the two indices "j" for the side-x (pivot) and side-y
// array respectively correspondent to the configurtion number "i"
//
if(m_matrixDebug&1<<df) {
 DISP<<" Matrix::coincidence; lowhigh= "<<m_lowhigh<<endl
     <<" Matrix::coincidence; majority array ="
     <<" "<<m_majorities[0]<<" "<<m_majorities[1]<<" "<<m_majorities[2]<<endl;
 DISP_DEBUG;
}//end-of-if(m_matrixDebug&1<<df)
//
for(i=0; i<s_nclock; i++)             { // loop on clock cycles
 for(thres=0; thres<s_nthres; thres++){ // loop on the three possible thresholds
                                      // thresholds address increases with
				      // increasing pT 
  nconf = config(thres,&conf[0][0]);  // number of config. for this threshold
//
//if(m_matrixDebug&1<<df) {
// DISP<<"nconf="<<nconf<<" conf=";
// DISP_DEBUG;
// for(int ii=0;ii<4;ii++){ DISP<<" "<<conf[ii][0]; DISP_DEBUG;}
// DISP<<"  "; DISP_DEBUG;
// for(int ii=0;ii<4;ii++){ DISP<<" "<<conf[ii][1]; DISP_DEBUG;}
// DISP<<endl; DISP_DEBUG;
//}//end-of-if(m_matrixDebug&1<<df)

  for(l=0; l<nconf; l++)            { // loop on all config. for this threshold
   for(j=0;j<s_nchan[0];j++) {          // loop on all side-x channels
//
// coincidence evaluation:
// for the given configuration "l" and the given channel "j" in the 
// pivot plane, the coincidence requires 
// a) the required majority "m_mjori[thres][0][conf[l][0]][i][0]" 
//    for channel "j"
// b) the required majority "m_mjori[thres][1][conf[l][0]][i][0,1]"
//    in coincidence with the programmed road in "*(m_roads+2*j)" and
//    "*(m_roads+2*j+1)
//
//    if(   bitstatus(&m_mjori[thres][0][conf[l][0]][i][0],j)
//          *( m_mjori[thres][1][conf[l][1]][i][0]&(*(m_roads+32*2*thres+2*j+0))
//	  |  m_mjori[thres][1][conf[l][1]][i][1]&(*(m_roads+32*2*thres+2*j+1)))){



    if(   bitstatus(&m_mjori[thres][0][conf[l][0]][i][0],j)
          && ( (m_mjori[thres][1][conf[l][1]][i][0]&m_trigRoad[thres][j][0])
             |  (m_mjori[thres][1][conf[l][1]][i][1]&m_trigRoad[thres][j][1]))){

    if(m_matrixDebug&1<<df) {
     DISP<<"coincidence!!!"<<" clock="<<i<<" xchan="<<j<<endl;
     DISP_DEBUG;
    }//end-of-if(m_matrixDebug

     set_to_1(&m_trigg[thres][i],j);
     BCaddress = ((i+m_BunchPhase)/s_NDLLCYC)+m_BunchOffset;
     if(BCaddress>=0 && BCaddress<s_NBunch)
        m_trigger[thres][BCaddress]=1;  // set "m_trigger" to 1 for this threshold...
                                      // ...and this Bunch Crossing
				   
    }//end-of-if(bitstatus...   
   }//end-of-for(j
  }//end-of-for(l
//
//  for(m=0;m<2;m++) {   // loop on left-right sides to evaluate overlap
//   if(!m_triggerOverlapRO[i][m])
//       m_triggerOverlapRO[i][m]       = (m_trigg[thres][i] & m_matOverlap[m]);
//   if(!m_triggerOverlap[i/s_NDLLCYC][m])
//       m_triggerOverlap[i/s_NDLLCYC][m] = (m_trigg[thres][i] & m_matOverlap[m]);
//  }//end-of-for(m
 }//end-of-for(thres
}//end-of-for(i

if(CMAVERSION==2004) {
//
// build the rise pulse for "m_trigg" (320 MHz trigger output) signals
//
 CMAword previousTime=0;
 for(chan=0; chan<s_nchan[0]; chan++) {
   for(thres=0; thres<s_nthres; thres++){ // loop on the three possible thresholds
     for(i=s_nclock-1; i>0; i--)             { // loop on clock cycles
      previousTime = bitstatus(&m_trigg[thres][i-1],chan);
      if(bitstatus(&m_trigg[thres][i],chan) && previousTime) {
       set_to_0(&m_trigg[thres][i],chan);
      }//end-of-if(bitstatus
     }//end-of-for(i
   }//end-of-for(thres
 }//end-of-for(chan
}//end-of-if(CMAVERSION
// - - - - - - - - - - -

 for(chan=0; chan<s_nchan[0]; chan++) {
   for(thres=0; thres<s_nthres; thres++){ // loop on the three possible thresholds
     for(i=s_nclock-1; i>0; i--)             { // loop on clock cycles     
       if(bitstatus(&m_trigg[thres][i],chan)) {
         for(ubit16 idead=1; idead<m_trigDeadTime[chan/s_timeGroupB]; idead++) {
	  set_to_0(&m_trigg[thres][i+idead],chan);
	 }//end-of-for(   
       }//end-of-bitstatus     
     }//end-of-for(i
   }//end-of-for(thres
 }//end-of-for(chan

if(CMAVERSION==2004) {
//
// ...now determine the correspondent "m_trigger" (40 MHz trigger output)
//
 for(thres=0; thres<s_nthres; thres++){ // loop on the three possible thresholds
   // reset trigger
   for(j=0;j<s_NBunch;j++) {
     m_trigger[thres][j]=0;
   }//end-of-for(j
   // compute new "m_trigger"
   for(i=0; i<s_nclock; i++)             { // loop on clock cycles
     BCaddress = ((i+m_BunchPhase)/s_NDLLCYC)+m_BunchOffset;
     if(BCaddress>=0 && BCaddress<s_NBunch) {
      if(m_trigg[thres][i]) m_trigger[thres][BCaddress]=1;
     }//emd-of-if(BCadd
   }//end-of-for(i
 }//end-of-for(thres
}//end-of-if(CMAVERSION
// - - - - - - - - - - -
//
// find triggers in overlap regions
//
//for(thres=0; thres<s_nthres; thres++){ // loop on the three possible thresholds
  thres = m_overlapthres;
  for(i=0; i<s_nclock; i++)             { // loop on clock cycles
   if(m_trigg[thres][i]) {
    
     for(m=0;m<2;m++) {   // loop on left-right sides to evaluate overlap
       if(!m_triggerOverlapRO[i][m]) {
         m_triggerOverlapRO[i][m]       = (m_trigg[thres][i] & m_matOverlap[m]);
       }
       BCaddress = ((i+m_BunchPhase)/s_NDLLCYC)+m_BunchOffset;
       if(BCaddress>=0 && BCaddress<s_NBunch) {
         if(!m_triggerOverlap[BCaddress][m])
           m_triggerOverlap[BCaddress][m] = (m_trigg[thres][i] & m_matOverlap[m]);
       }//end-of-if(BCaddress
      }//end-of-for(m
    
   }//end-of-for(m_trigg
  }//end-of-for(i
//}//end-of-for(thres
//
// normalize m_triggerOverlapRO
//
for(i=0; i<s_nclock; i++)             { // loop on clock cycles
 for(m=0;m<2;m++) {   // normalize to 1 overlap flags
  m_triggerOverlapRO[i][m] = m_triggerOverlapRO[i][m] ? 1 : 0; 
 }//end-of-for(m
}//end-of-for(i
//
// normalize m_triggerOverlap
//
for(i=0;i<s_NBunch; i++) { //loop on bunches
 for(m=0;m<2;m++) {   // normalize to 1 overlap flags
  m_triggerOverlap[i][m] = m_triggerOverlap[i][m] ? 1 : 0;
 }//end-of-for(m
}//end-of-for(i
}//end-of-Matrix::coincide
//------------------------------------------------------------------------//
void Matrix::maskTo1 () {
ubit16 df=11;
if(m_matrixDebug&1<<df) {
 DISP<<"---------------------"<<endl
     <<"| Matrix::mask_to_1 |"<<endl
     <<"---------------------"<<endl;
 DISP_DEBUG;
}//end-of-if(m_matrixDebug&1<<df)

ubit16 i, j, k, l, m;
for(m=0;m<s_nthres;m++)   {       // thresholds
 for(i=0; i<2; i++)      {      // side address
  for(j=0; j<2; j++)      {     // majority address
   for(l=0; l<s_nchan[i]; l++) {  // channel
    if(bitstatus(&m_channMask1[m][i][j][0],l)) {
      for(k=0; k<s_nclock; k++) {   // clock bins
       set_to_1(&m_mjori[m][i][j][k][0],l);
      }//end-of-for(k
     }//end-of-if(m_channMask1
    }//end-of-for(l
  }//end-of-for(j
 }//end-of-for(i
}//end-of-for(m

}//end-of-Matrix::mask_to_1
//------------------------------------------------------------------------//
void Matrix::deadTime() {
ubit16 i, j, k, l; 
ubit16 temp[s_nclock];
//
for(i=0;i<2;i++) {             // loop on both matrix sides
 for(j=0;j<2; j++) {           // loop on both trigger layers
  for(k=0;k<s_nchan[i]; k++)   { // loop on all channels
//
// set to 1 the bins in "temp" where signals are "on" for the first time
//
   for(l=0;l<(s_nclock-1);l++) { // loop on clock bins
    ((!bitstatus(&m_input[i][j][l  ][0],k)) &&
       bitstatus(&m_input[i][j][l+1][0],k) ) ?
     temp[l+1]=1 : temp[l+1]=0 ;
   }//end-of-for(l
   temp[0]=bitstatus(&m_input[i][j][0][0],k);
//
// transfer to "input" the signals in "temp" far enough each other in time
//
   sbit16 lastUp=-1;
   for(l=0;l<s_nclock;l++)     { // loop on clock bins
//   
    if(!temp[l]) {
     set_to_0(&m_input[i][j][l][0],k);
    } else {
//    
     if( (lastUp<0) || (l-lastUp)>=m_channDeadT[i][j][k/s_timeGroupB]) {
      lastUp=l;
      set_to_1(&m_input[i][j][l][0],k);
     } else {
      set_to_0(&m_input[i][j][l][0],k);
     }//end-of-if    
    }//end-of-if
//   
   }//end-of-for(l
  }//end-of-for(k
 }//end-of-for(j
}//end-of-for(i
}//end-of-Matrix::deadTime
//------------------------------------------------------------------------//
void Matrix::pulse_width () {
ubit16 df=12;
ubit16 i,j,l,m;
sbit16 k;
if(m_matrixDebug&1<<df) {
 DISP<<"-----------------------"<<endl
     <<"| Matrix::pulse_width |"<<endl
     <<"-----------------------"<<endl;
 DISP_DEBUG;
}//end-of-if(m_matrixDebug&1<<df)
//
for(i=0; i<2; i++) {                // loop on the two Matrix sides 
 for(j=0; j<2; j++)  {              // loop on both layers
  for(l=0; l<s_nchan[i]; l++)     {   // loop on all channels
   for(k=s_nclock-1; k>=0; k--) {     // loop on the s_nclock cycles backwards
    if(bitstatus(&m_input[i][j][k][0],l)) {
// loop on all time bins to be set to 1
     for(m=k+1; m<k+m_pulseWidth[i][j][l/s_timeGroupB]; m++) {   
      if(m<s_nclock) {
       set_to_1(&m_input[i][j][m][0],l);
      } else {
       break;
      }//end-of-if(m
     }//end-of-for(m
    }//end-of-if(bitstatus
   }//end-of-for(k
  }//end-of-for(l
 }//end-of-for(j
}//end-of-for(i
//
}//end-of-Matrix::pulse_width
//------------------------------------------------------------------------//
void Matrix::majori () {
//
// input   none
// returns  nothing
//
// Fills the majority registers the 1/2 and 2/2 majority logic
//
// Dynamic scanning of "on" channels has not to be done
// in this first part of the method
//
ubit16 i, j, k, l, n;
ubit16 df=13;  // debug flag address
CMAword buffi[2], buffo[2];
if(m_matrixDebug&1<<df) {
 DISP<<"-------------------"<<endl
     <<"| Matrix::majori  |"<<endl
     <<"-------------------"<<endl;
 DISP_DEBUG;
}//end-of-if(m_matrixDebug&1<<df)
//
// the loop on the CMA sides has to be made as follows:
// 1) side=0 and side=1 as long as the lowpt Matrix is concerned;
// 2) side=1 only as long as the highpt Matrix is concerned.
// To do this, loop from lowhigh address.
//
for(n=0; n<s_nthres; n++) {    // the s_nthres thresholds
 for(i=0; i<2; i++)       {  // the two Matrix sides
  for(j=0; j<s_nclock; j++)  { // the clock cycles
   for(k=0;k<2;k++) {        // the two words to make 64 bits
// copy layer with address 0,1 to buffi; initialize buffoutput
    buffi[k] = m_prepr[n][i][1][j][k];
    buffo[k] = 0;
   }
   shift(&buffi[0],&buffo[0],i);
   for(k=0; k<2; k++)  {     // the two words
// 1/2 majority
    m_mjori[n][i][0][j][k]=m_prepr[n][i][0][j][k] | m_prepr[n][i][1][j][k];
// 2/2 majority 
    m_mjori[n][i][1][j][k]=m_prepr[n][i][0][j][k] & buffo[k];
   }//end-of-for(k
//
// complete preprocessing...
//
   for(l=0; l<s_nchan[i]; l++) { // loop in the number of channel for this side
    if( bitstatus(&m_mjori[n][i][1][j][0],l)) {
     if((l>0)         &&(!bitstatus(&m_mjori[n][i][1][j][0],l-1))) set_to_0(&m_mjori[n][i][0][j][0],l-1);
     if((l<s_nchan[i]-1)&&(!bitstatus(&m_mjori[n][i][1][j][0],l+1))) set_to_0(&m_mjori[n][i][0][j][0],l+1);
    }//end-of-if(
   }//end-of-for(l  
// preprocessing completed 
  }//end-of-for(j
 }//end-of-for(i
}//end-of-for(n
}//end-of-method majori
//------------------------------------------------------------------------//
void Matrix::shift (CMAword *buffi, CMAword *buffo, ubit16 i) {
//
ubit16 k;
switch (m_localDirec[i]) {
 case 1:   // n(layer0)-->n(layer1)
  for(k=0;k<2;k++) { *(buffo+k) = *(buffi+k); }
  break;
 case 2:   // n(0)-->n-1(1)
  for(k=0;k<2;k++) { *(buffo+k) = *(buffi+k) << 1; }
  *(buffo+1)=(*(buffi+0) & 0x80000000) ? (*(buffo+1) | 0x1) : (*(buffo+1) | 0);
  break;
 case 3:   // case 2 plus case 1
  for(k=0;k<2;k++) { *(buffo+k) = *(buffi+k) | (*(buffi+k) << 1); }
  *(buffo+1)=(*(buffi+0) & 0x80000000) ? (*(buffo+1) | 0x1) : (*(buffo+1) | 0);
  break;
 case 4:   // n(0)-->n+1(1)
  for(k=0;k<2;k++) { *(buffo+k) = *(buffi+k) >> 1; }
  *(buffo+0)=(*(buffi+1) & 0x1) ? (*(buffo+0) | 0x80000000) : (*(buffo+0) | 0);
  break;
 case 5:   // case 4 plus case 1
  for(k=0;k<2;k++) { *(buffo+k) = *(buffi+k) | (*(buffi+k) >> 1); }
  *(buffo+0)=(*(buffi+1) & 0x1) ? (*(buffo+0) | 0x80000000) : (*(buffo+0) | 0);
  break;
 case 6:   // case 4 plus case 2
  for(k=0;k<2;k++) { *(buffo+k) = (*(buffi+k) >> 1) | (*(buffi+k) << 1); }
  *(buffo+0)=(*(buffi+1) & 0x1) ? (*(buffo+0) | 0x80000000) : (*(buffo+0) | 0);
  *(buffo+1)=(*(buffi+0) & 0x80000000) ? (*(buffo+1) | 0x1) : (*(buffo+1) | 0);
  break;
 case 7:   // case 4 plus case 2 pluse case 1
  for(k=0;k<2;k++) { *(buffo+k) =  *(buffi+k)       | 
                                  (*(buffi+k) >> 1) | 
                                  (*(buffi+k) << 1); 
  }
  *(buffo+0)=(*(buffi+1) & 0x1) ? (*(buffo+0) | 0x80000000) : (*(buffo+0) | 0);
  *(buffo+1)=(*(buffi+0) & 0x80000000) ? (*(buffo+1) | 0x1) : (*(buffo+1) | 0);
  break;
 default:
  cout<<" Matrix::shift -- m_localDirec["<<i<<"]="<<m_localDirec[i]
      <<" not expected; m_localDirec forced to be 1 "<<endl;
  for(k=0;k<2;k++) { *(buffo+k) = *(buffi+k); }
}
//
// correct patter in *(buffo+1) in case side of Matrix (="i" in this method)
// is = 1
//
 if(!i) *(buffo+1)=0;
//
}//end-of-Matrix::shift(buff0, buffi, buffo)
//------------------------------------------------------------------------//
void Matrix::declus () {
//
// Dynamic scanning of "on" channels has not to be done
// in this first part of the method
//
ubit16 df=14;
ubit16 nup,first, i,j,k,l;
first=0;
if(m_matrixDebug&1<<df) {
 DISP<<"------------------------"<<endl
     <<"| matrix: declustering |"<<endl
     <<"------------------------"<<endl;
 DISP_DEBUG;
}//end-of-if(m_matrixDebug&1<<df)
//
// loop on m_input data
//
for(i=0; i<2; i++) {        // loop on the two sides
 for(j=0; j<2; j++) {       // loop on the two layers
  for(k=0; k<s_nclock; k++) { // loop on the time bins
   nup=0;                   // counter of consecutive "on" channels
   for(l=0; l<s_nchan[i]; l++) {   // loop on the Matrix channels
    if(bitstatus(&m_input[i][j][k][0],l)) {
     nup++;
     if(nup==1) first=l; 
    }//end-of-if(bitstatus 
    if (!bitstatus(&m_input[i][j][k][0],l)||(l==(s_nchan[i]-1))){
     if(nup) {
      reduce(i,j,k,0,nup,first);
      nup=0;
     }//end-of-if(nup
    }//end-of-if(bitstatus
   }//end-of-for(l
  }//end-of-for(k
 }//end-of-for(j
}//end-of-for(i
}//end-of-method declustering
//------------------------------------------------------------------------//
void Matrix::reduce ( ubit16 ia, ubit16 ja, ubit16 ka, ubit16 la,
                      ubit16 nup, ubit16 first) {
//
// input  ia, ja, ka, la  indices of the Matrix data
//        nup    cluster size = number of consecutive channels on
//        first  address of the first channel on in the cluster (from 0)
// returns  nothing
//
// It copies into m_prepr the channels from input selected by the 
// declustering logic.
// 
//
ubit16 ncop, i, j, na;
ubit16 df=15;
ncop=0;
j=0;
if(m_matrixDebug&1<<df) {
 DISP<<" --------------------"<<endl
     <<" |  Matrix::reduce  |"<<endl
     <<" --------------------"<<endl
     <<" nup= "<<nup<<" first "<<first<<endl;
 DISP_DEBUG;
}//end-of-if(m_matrixDebug&1<<df)
//
// analyse nup value and apply the cluster reduction according to it.
// j     is the first channel of the cluster retained after declustering;
// ncop  is the cluster size after declustering
//
if        (nup<=2) {
           j=first;
           ncop=nup;
} else  if(nup==3) {
           j=first+1;
           ncop=1;
} else if (nup==4) {
           j=first+1;
           ncop=2;
} else if (nup>4) {
           j= first+2;
           ncop=nup-4;
}//end-of-if
if(m_matrixDebug&1<<df) {
 DISP<<" j= "<<j<<" ncop= "<<ncop<<endl;
 DISP_DEBUG;
}//end-of-if(m_matrixDebug&1<<df)
//
// copy the reduced cluster into the "s_nthres" m_prepr registers
//
for(na=0; na<s_nthres; na++) {
 for(i=j;i<j+ncop;i++) {     // loop on each element of the reduced cluster
  set_to_1(&m_prepr[na][ia][ja][ka][la],i);
 }//end-of-for(i
}//end-of-for(na
}//end-of-reduce
//------------------------------------------------------------------------//
void Matrix::makeOut() {
ubit16 i, j, k;
//
// fill k-pattern
//
for(j=0; j<s_nclock; j++)     {    // loop on the clock bins
 m_k_pattern[j] = m_trigg[m_lowtohigh][j];
 k_readout[j] = m_trigg[m_toreadout][j];
}//end-of-for(j
//
// find the highest satisfied threshold;
// identify Bunch Crossing and put it in BCID.
//
for(j=0;j<s_NBunch;j++) {
 for(i=0;i<s_nthres;i++) {
  if(m_trigger[i][j]) {
   m_highestth[j]=i+1;  // put threshold address+1 (correct threshold value)
   if(m_BCID < 0) m_BCID = j          // local Bunch Crossing IDentifier
                     + m_thisBC;    // re-syncronize to absolute BCID
//		     - m_BCzero; // remove offset used
  }//end-of-if(m_trigger
 }//end-of-for(i
}//end-of-for(j
//
// Trigger in Overlapping channels is reported in m_triggerOverlap
//
for(j=0;j<s_NBunch;j++) {
 m_overlap[j]= m_triggerOverlap[j][0]+2*m_triggerOverlap[j][1];
}//end-of-for(j
//
// find the highest satisfied threshold for ReadOut pourposes;
//
for(j=0; j<s_nclock; j++)     {     // loop on the clock bins
 for(i=0; i<s_nthres; i++) {        // loop on thresholds
  for(k=0; k<s_nchan[0]; k++) {     // loop on channels (pivot side)
   if(m_trigg[i][j]&(1<<k)) highestthRO[j]=i+1;
  }//end-of-for(k
 }//end-of-for(i
}//end-of-for(j
//
//
// Trigger in Overlapping channels is reported in m_triggerOverlapRO
//
for(j=0;j<s_nclock;j++) {
 overlapRO[j]= m_triggerOverlapRO[j][0]+2*m_triggerOverlapRO[j][1];
}//end-of-for(j
//
}//end-of-Matrix::makeOut
//------------------------------------------------------------------------//
void Matrix::makeTestPattern (ubit16 mode, ubit16 ktimes) {
//
int df=16;
const ubit16 maxchan=100;
const ubit16 maxtimes=1000;
ubit16 i,j,l;
ubit16 IJ[maxtimes][4], channels[maxtimes][4][maxchan]; 
float times[maxtimes]; char plane[4][3];
const float timeOffsetHit=114.675;
//
rpcdata *rpcpnt;
int ntimes;
float timemin;
//
strcpy(plane[0],"I0");
strcpy(plane[1],"I1");
strcpy(plane[2],"J0");
strcpy(plane[3],"J1");
if(m_matrixDebug&1<<df) {
 DISP<<"-------------------------------"<<endl
     <<"|  Matrix::makeTestPattern    |"<<endl
     <<"-------------------------------"<<endl;
 DISP_DEBUG;
}//end-of-if(m_matrixDebug&1<<df)
//
ntimes = 0;
ubit16 completed = 0;
for(l=0; l<maxtimes; l++) {for(i=0; i<4; i++) {IJ[l][i]=0;}}
//
// first reset the marker flags
//
for(i=0;i<2;i++) {
 rpcpnt = m_datarpc[i];
 while (rpcpnt) {
  rpcpnt->mark = 0;
  rpcpnt=rpcpnt->next;
 }//end-of-while(rpcpnt
}//end-of-for(i
//
while(!completed) {
 completed=1;
 timemin = 999999999.;
 for(i=0; i<2; i++) {  // "i" is the CMA side address
  rpcpnt=m_datarpc[i];
  while (rpcpnt) {
   if(rpcpnt->time < timemin && !rpcpnt->mark) {
    timemin=rpcpnt->time;
    completed=0;
   }//end-of-if(rpcnt
   rpcpnt=rpcpnt->next;
  }//end-of-while(rpcpnt)
 }//end-of-for(i
 if(!completed) {
  if(ntimes<maxtimes) ntimes += 1;
  times[ntimes-1]=timemin;
  for(i=0; i<2; i++) {  // "i" is the CMA side address
  rpcpnt=m_datarpc[i];
   while (rpcpnt) {
    if(rpcpnt->time == timemin) {
     rpcpnt->mark=1;
     if(IJ[ntimes-1][rpcpnt->layer+2*i] < maxchan) {
      IJ[ntimes-1][rpcpnt->layer+2*i] +=1;
      }
     channels[ntimes-1][rpcpnt->layer+2*i][IJ[ntimes-1][rpcpnt->layer+2*i]-1]=
     rpcpnt->stripadd;
    }//end-of-if(rpcnt
    rpcpnt=rpcpnt->next;
   }//end-of-while(rpcpnt)
  }//end-of-for(i
 }//end-of-if(!completed
}//end-of-while(!completed)
//
//
// open output file
//
ofstream vhdlinput;
vhdlinput.open("k-trigger.output",ios::app);
if(!vhdlinput){
 DISP<<" File for vhdl analysis not opened. "<<endl
     <<" =================================="<<endl<<endl;
 DISP_ERROR;
 } else {
  if(m_matrixDebug&1<<df) {
   DISP<<" File for vhdl analysis correctly opened"<<endl<<endl;
   DISP_ERROR;
 }//end-of-if(m_matrixDebug&1<<df)
}//end-of-if(!vhdlinput
if(mode) {
 vhdlinput<<" RUN "<<m_run<<" EVENT "<<m_event<<" WINDOW "<<s_NBunch;
 vhdlinput<<" LINES "<<(ntimes+ktimes)<<std::endl;
}//end-of-if(mode
 for(l=0;l<ntimes; l++) {
  vhdlinput<<" TIME "<<times[l]+timeOffsetHit<<" ";
  for(i=0; i<4; i++) {
   vhdlinput<<plane[i][0]<<plane[i][1]<<" "<<IJ[l][i]<<" ";
   for(j=0; j<IJ[l][i]; j++) {
    vhdlinput<<channels[l][i][IJ[l][i]-1-j]<<" ";
   }//end-of-for(j
  }//end-of-for(i
  vhdlinput<<std::endl;
 }//end-of-for(l
//
vhdlinput.close();
}//end-of-makeTestPattern
//------------------------------------------------------------------------//
void Matrix::makeOutPattern () {
//int df=17;
const float timeOffsetKPa=168.125;
const float timeOffsetThr=210.500;
ubit16 i, l;
CMAword bit;
ubit16 chanHistory[32];
for(i=0;i<32;i++) {chanHistory[i]=0;}
const ubit16 maxchan = 100;
const ubit16 maxtimes = s_nclock;
ubit16 ntimes, newtime;
ubit16 nchannels[maxtimes][2][2], channels[maxtimes][2][2][maxchan];
float time, times[maxtimes];
//
// trigger registers: k-trigger (historically was k-pattern)
//
ntimes=0;
for(i=0; i<s_nclock; i++) {
 nchannels[i][0][0]=0;
}
time = (float)m_thisBC*s_BCtime - ((float) (m_BCzero*s_NDLLCYC))*s_DLLtime;
for(i=0; i<s_nclock; time += s_DLLtime, i++) {
  bit=1;
  newtime=1;
  for(l=0; l<s_nchan[0];l++) {
   if(m_k_pattern[i] & bit ) {
    if(!chanHistory[l]) {
     if(newtime) {
      if(ntimes<maxtimes) ntimes+=1;
      times[ntimes-1] = time;
     }//end-of-if(newtime
     if(nchannels[ntimes-1][0][0]<maxchan) nchannels[ntimes-1][0][0] += 1;
      channels[ntimes-1][0][0][nchannels[ntimes-1][0][0]-1]=l;
      newtime=0;
     }//end-of-if(!chanHistory
    chanHistory[l]=1;  
   } else { // if(m_k_pattern
    chanHistory[l]=0;
   }//end-of-if(m_k_pattern
   bit=bit<<1;
  }//end-of-for(l
}//end-of-for(i

ubit16 nthresPass, thresPass[NOBXS], overlPass[NOBXS], BCidentifier[NOBXS];
nthresPass=0;
for(i=0;i<s_NBunch; i++) {
 if(m_highestth[i]) {
  thresPass[nthresPass] = m_highestth[i];
  overlPass[nthresPass] = m_overlap[i];
  BCidentifier[nthresPass] = i;
  nthresPass += 1;
 }
}
makeTestPattern(1,ntimes+2*nthresPass);
//
// open output file
//
ofstream out_k_trigger;
out_k_trigger.open("k-trigger.output",ios::app);
//
// code to print out m_k_pattern
//
for(i=0; i<ntimes; i++) {
 out_k_trigger<<" TIME "<<times[i]+timeOffsetKPa<<" K ";
 out_k_trigger<<nchannels[i][0][0]<<" ";
 for(l=0;l<nchannels[i][0][0]; l++) {
  out_k_trigger<<channels[i][0][0][l]<<" ";
 }//end-of-for(l
 out_k_trigger<<endl;
}//end-of-for(i
//
if(nthresPass) {
 for(i=0;i<nthresPass;i++) {
  out_k_trigger<<" TIME "<<BCidentifier[i]*25.+timeOffsetThr;
  out_k_trigger<<" THR "<<thresPass[i]<<endl;
  if(overlPass[i]) {
    out_k_trigger<<" TIME "<<BCidentifier[i]*25.+timeOffsetThr;
    out_k_trigger<<" OVL "<<overlPass[i]<<std::endl;
  }
 }
}
//
out_k_trigger.close();
//
}//end-of-makeOutPattern
//------------------------------------------------------------------------//
ubit16 Matrix::config (ubit16 i, ubit16 *arr) {
//
// input:  i     threshold address
//         *arr  pointer to bidimen. array
//
// returns the number of possible configurations compatible with the 
// required majority pointed by majorities[i]
//
// Data *arr(k) and *arr(k+1) give the majority addresses for the side-x
// and side-y respectevely.
//
ubit16 nconf=0;
//
// DISP<<"lowhig="<<m_lowhigh<< "majorities= "<<m_majorities[0]
//     <<" "<< m_majorities[1]<<" "<<m_majorities[2]<<endl;
// DISP_DEBUG;
//
switch (m_lowhigh) {
 case 0:                    // low-pt trigger matrix
 switch (m_majorities[i]) {       // select the required majority for each thresh.;
  case 0:                     // 1-out-of-4 majority: no implementation yet
                              // gives 1/2 in I OR 1/2 in J
   nconf=0;
   break;
  case 1:                     // 2-out-of-4 majority
   nconf=1;
   *(arr+0)=0; *(arr+1)=0;
   break;
  case 2:                     // 3-out-of-4 majority
   nconf=2;
   *(arr+0)=0; *(arr+1)=1;
   *(arr+2)=1; *(arr+3)=0;
   break;
  case 3:                     // 4-out-of-4 majority
   nconf=1;
   *(arr+0)=1; *(arr+1)=1;
   break;
  default:
   DISP<<" Matrix::config: the majority "<<m_majorities[i]
       <<" is unforeseen "<<endl;
   DISP_ERROR;
 }
  break;
 case 1:                    // high-pt trigger matrix
// high-pt trigger
  switch(m_majorities[i]) {
   case 1:                   // 1-out-of-2 majority
    nconf=1;
    *(arr+0)=0;
    *(arr+1)=0;
    break;
   case 2:                   // 2-out-of-2 majority
    nconf=1;
    *(arr+0)=0;
    *(arr+1)=1;
    break;
   default:
    DISP<<" Matrix::config: the majority "<<m_majorities[i]
        <<" is unforeseen "<<endl;
    DISP_ERROR;
  }
  break;
 default:
  DISP<<" Matrix::config: lowhighpt "<<m_lowhigh<<" is unforeseen "<<endl;
  DISP_ERROR;
}
 return nconf;
}//end-of-method-config
//------------------------------------------------------------------------//
int Matrix::getSubsystem() {return m_subsystem;}
//------------------------------------------------------------------------//
int Matrix::getProjection() {return m_projection;}
//------------------------------------------------------------------------//
int Matrix::getSector() {return m_sector;}
//------------------------------------------------------------------------//
int Matrix::getPad() {return m_pad;}
//------------------------------------------------------------------------//
int Matrix::getLowHigh() {return m_lowhigh;}
//------------------------------------------------------------------------//
int Matrix::getAddress0() {return m_address[0];}
//------------------------------------------------------------------------//
int Matrix::getAddress1() {return m_address[1];}
//------------------------------------------------------------------------//
int Matrix::getLocalAdd() {return m_localadd;}
//------------------------------------------------------------------------//
ubit16 Matrix::getOutputThres(ubit16 bunch) {
return m_highestth[bunch];
}//end-of-ubit16-getOutput 
//------------------------------------------------------------------------//
ubit16 Matrix::getOutputOverl(ubit16 bunch) {
return m_overlap[bunch];
}//end-of-ubit16-getOutput
//------------------------------------------------------------------------//
sbit16 Matrix::getBunchPhase() {
return m_BunchPhase;
}//end-of-ubit16-getBunchPhase
//------------------------------------------------------------------------//
sbit16 Matrix::getBunchOffset() {
return m_BunchOffset;
}//end-of-ubit16-getBunchOffset
//------------------------------------------------------------------------//
void Matrix::set_to_1(CMAword *p, sbit16 channel) {
ubit16 i[2];
CMAword j;
i[0]=0; i[1]=0;
j=1;
inds(&i[0],channel);
if(!(channel<0)) {
 *(p+i[0]) = *(p+i[0]) | j<<i[1];
} else {
 DISP<<" Matrix::set_to_1: channel is negative; channel= "
     <<channel<<endl;
 DISP_ERROR;
}//end-of-if(!(channel<0 
}//end-of-Matrix::set_to_1
//----------------------------------------------------------------------//
void Matrix::set_to_0(CMAword *p, sbit16 channel) {
ubit16 i[2];
CMAword j;
j=1;
inds(&i[0],channel);
if(!(channel<0)) {
 *(p+i[0]) = *(p+i[0]) & ~(j<<i[1]);
} else {
 DISP<<" Matrix::set_to_1: channel is negative; channel= "
     <<channel<<endl;
 DISP_ERROR;
}//end-of-if(!(channel<0 
}//end-of-Matrix::set_to_1
//----------------------------------------------------------------------//
ubit16 Matrix::bitstatus(const CMAword *p,ubit16 channel) {
ubit16 id[2];
CMAword j;
j=1;
inds(&id[0],channel);
return *(p+id[0])& j<<id[1] ? 1 : 0 ;
}//end-of-Matrix::bitstatus
//----------------------------------------------------------------------//
void Matrix::wind () {
sbit16 i, j;
 DISP<<"-----------------------"<<endl
     <<"|     Matrix::wind    |"<<endl
     <<"-----------------------"<<endl
     <<" Matrix Roads "<<endl;
 DISP_DEBUG;
 for(i=0; i<s_nthres; i++) {
  for(j=0; j<s_nchan[0]; j++) {
   DISP<<" thres. "<<i<<" channel "<<j
//       <<" Road0 "<<hex<<(*(m_roads+32*2*i+2*j+0))<<dec
//       <<" Road1 "<<hex<<(*(m_roads+32*2*i+2*j+1))<<dec<<endl;
       <<" Road0 "<<hex<<(m_trigRoad[i][j][0])<<dec
       <<" Road1 "<<hex<<(m_trigRoad[i][j][1])<<dec<<endl;
   DISP_DEBUG;
  }
 }
 DISP<<" majorities: ";
 DISP_DEBUG;
 for(i=0;i<3;i++) {DISP<<m_majorities[i]<<" ";DISP_DEBUG;} 
 DISP<<" "<<endl; DISP_DEBUG;
 DISP<<" number of overlapping ' low' channels: "<<m_matOverlap[0]<<endl
     <<" number of overlapping 'high' channels: "<<m_matOverlap[1]<<endl;
 DISP_DEBUG;
 for(i=0;i<s_nchan[0];i++) {
 DISP<<" channel "<<i<<" in coincidence with "<<m_diagonal[i]<<endl;
 DISP_DEBUG;
 }//end-of-for(i
}//end-of-method-wind
//----------------------------------------------------------------------//
void Matrix::display() {
ubit16 i;
ubit16 df=19;
rpcdata *rpcpnt;
//
//if(this) {
 DISP<<"======================="<<endl
     <<"||   Matrix Display  ||"<<endl
     <<"======================="<<endl
     <<endl;
 DISP_DEBUG;
 show_attributes();
 DISP<<" "<<endl;
 DISP_DEBUG;
//
 DISP<<" All raw data "<<endl;
 DISP_DEBUG;
 for(i=0; i<2; i++) {
  DISP<<" Matrix Side is "<<i<<endl;
  DISP_DEBUG;
  rpcpnt=m_datarpc[i];
  while (rpcpnt) {
   DISP<<"  Layer= " <<rpcpnt->layer
       <<" stripadd= "<<rpcpnt->stripadd
       <<" time= "<<rpcpnt->time
       <<" mask= "<<rpcpnt->masked
       <<" BC= "<<rpcpnt->BC
       <<" DLL= "<<rpcpnt->DLL
       <<" delay= "<<rpcpnt->delay
       <<endl;
   DISP_DEBUG;
   rpcpnt=rpcpnt->next;
  }//end-of-while(rpcpnt)
 }//end-of-for(i
//
if(m_matrixDebug&1<<(df+0)) {
 DISP<<" Display Matrix Input "<<endl;
 DISP_DEBUG;
 disp_CMAreg(0); //display the input registers
}
//
//
if(m_matrixDebug&1<<(df+1)) {
 DISP<<" Display Matrix Preprocessing "<<endl;
 DISP_DEBUG;
 disp_CMAreg(1); //display the prepro registers
}
//
if(m_matrixDebug&1<<(df+2)) {
 DISP<<" Display Matrix Majority "<<endl;
 DISP_DEBUG;
 disp_CMAreg(2); //display the majority registers
}
//
if(m_matrixDebug&1<<(df+3)) {
 DISP<<" Display Trigger  "<<endl;
 DISP_DEBUG;
 disp_CMAreg(3); //display the trigger registers
}
//
//} else {
// DISP<<"======================="<<endl
//     <<"||    Matrix EMPTY   ||"<<endl
//     <<"======================="<<endl;
// DISP_DEBUG;
//}//end-of-Matrix::display
}//end-of-method display
//------------------------------------------------------------------------//
void Matrix::show_attributes () {
DISP<<" Matrix Attributes: "<<endl
    <<" Subsystem "<<m_subsystem<<"; Projection "<<m_projection
    <<"; Sector "<<m_sector<<"; Pad "<<m_pad<<"; LowHig "<<m_lowhigh
    <<"; addresses: "<<m_address[0]<<"  "<<m_address[1]<<endl; 
DISP_DEBUG;
}//end-of-Matrix::attributes
//------------------------------------------------------------------------//
void Matrix::disp_CMAreg(ubit16 id) {
//
// display the CMA registers
//
ubit16 i, j, k;
if(id<2) {
 for(i=0; i<2; i++) {     // loop on the two Matrix sides
  DISP<<" CMA Side (0=side-x; 1=side-y) "<<i;
  DISP_DEBUG;
  for(j=0; j<2; j++) {    // loop on the two Matrix layers
   switch (id) {
    case 0:
     DISP<<" Layer "<<j<<endl;
     DISP_DEBUG;
     dispRegister(&m_input[i][j][0][0],i);
     break;
    case 1:
     DISP<<" Layer "<<j<<endl;
     DISP_DEBUG;
     dispRegister(&m_prepr[0][i][j][0][0],i);
     break;
    default:
     DISP<<" Matrix::disp_CMAreg id value "<<id<<" not foreseen "<<endl;
     DISP_DEBUG;
   }//end-of-switch
  }//end-of-for(j
 }//end-of-for(i

} else {

 switch (id) {
  case 2:  
   for(i=0; i<s_nthres; i++) { // loop on threshold
    DISP<<" Threshold address "<<i<<endl;
    DISP_DEBUG;
    for(j=0; j<2; j++) {     // loop on matrix sides
     DISP<<" CMA Side (0=side-x; 1=side-y) "<<j;
     DISP_DEBUG;
     for(k=0; k<2; k++) {    // loop on majority types
      DISP<<" Majority type (0=1/2; 1=2/2) "<<k<<endl;
      DISP_DEBUG;
      dispRegister(&m_mjori[i][j][k][0][0],j);
     }//end-of-for(k
    }//end-of-for(j
   }//end-of-for(i
   break;
  case 3:
   for(i=0; i<s_nthres; i++) { // loop on the three thresholds
    DISP<<" Trigger Threshold address "<<i<<endl;
    DISP_DEBUG;
    dispTrigger(&m_trigg[i][0]);
   }//end-of-for(i
   DISP<<" ReadOut Buffer "<<endl;
   DISP_DEBUG;
   dispRegister(&rodat[0][0][0][0],0);
   break;
   default:
    DISP<<" Matrix::disp_CMAreg id value "<<id<<" not foreseen "<<endl;
    DISP_DEBUG;
  }//end-of-switch (id) 
 
}//end-of-if(id
DISP<<" "<<endl;
DISP_DEBUG;
}//end-of-Matrix::disp_CMAreg
//------------------------------------------------------------------------//
void Matrix::dispRegister(const CMAword *p, ubit16 side) {
ubit16 n, j, k;
//
// allocation for oststream strdisp
//
#if (__GNUC__) && (__GNUC__ > 2) 
    // put your gcc 3.2 specific code here
    __osstream* strdisp = new std::ostringstream;
#else
    // put your gcc 2.95 specific code here
    char buffer[5000];
    for (int i=0;i<5000;++i) buffer[i] = 0;
    __osstream* strdisp = new __osstream(buffer,5000);
#endif
//
n = (s_nchan[side]-1)/s_wordlen+1;
*strdisp<<"  ";

for(j=0;j<s_nchan[side];j+=2) {
 *strdisp<<" "<<j%10;
}//end-of-for
*strdisp<<" "<<endl;
for(j=0; j<s_nclock; j++) {       // loop on the s_nclock cycles
 *strdisp<<" "<<j%10<<" ";
 for(k=0; k<n; k++)      {      // loop on the buffer words
   dispBinary(p+k+2*j,strdisp);
 }//end-of-for(k
 *strdisp<<" "<<endl;

}//end-of-for(j

 DISP << strdisp->str();
 DISP_DEBUG;
 
 delete strdisp;
}//end-of-Matrix::dispRegister
//------------------------------------------------------------------------//
void Matrix::dispTrigger(const CMAword *p) {
ubit16 j;
//
// allocation for oststream strdisp
//

#if (__GNUC__) && (__GNUC__ > 2) 
    // put your gcc 3.2 specific code here
    __osstream* strdisp = new std::ostringstream;
#else
    // put your gcc 2.95 specific code here
    char buffer[5000];
    for (int i=0;i<5000;++i) buffer[i] = 0;
    __osstream* strdisp = new __osstream(buffer,5000);
#endif
//
*strdisp<<"  ";

for(j=0;j<s_nchan[0];j+=2) {
 *strdisp<<" "<<j%10;
}//end-of-for
*strdisp<<" "<<endl;
for(j=0; j<s_nclock; j++) {       // loop on the s_nclock cycles
 *strdisp<<" "<<j%10<<" ";
  dispBinary(p+j,strdisp);
 *strdisp<<" "<<endl;
}//end-of-for(j

 DISP << strdisp->str();
 DISP_DEBUG;
 
 delete strdisp;
}//end-of-Matrix::dispTrigger
//------------------------------------------------------------------------//
void Matrix::dispBinary (const CMAword *p, __osstream *strdisp) {
ubit16 i;
CMAword j;
j=1;
for(i=0; i<s_wordlen; i++) {
 if((*p) & j ){
  *strdisp<<"|";
 } else {
  *strdisp<<".";
 }//end-of-if
 j=j<<1;
 }//end-of-for(
}//end-of-Matrix::dispBinary
//------------------------------------------------------------------------//
void Matrix::dispWind () {
for(ubit16 i=0; i<s_nthres; i++) {dispWind(i);}
}//end-of-dispWind
//------------------------------------------------------------------------//
void Matrix::dispWind (ubit16 thres) {
#if (__GNUC__) && (__GNUC__ > 2) 
    // put your gcc 3.2 specific code here
    __osstream* strdisp = new std::ostringstream;
#else
    // put your gcc 2.95 specific code here
    char buffer[5000];
    for (int i=0;i<5000;++i) buffer[i] = 0;
    __osstream* strdisp(buffer,5000);
#endif

*strdisp<<endl
        <<" ========================="<<endl
        <<" =                       ="<<endl
        <<" =  Matrix::dispWind     ="<<endl
        <<" =  Threshold address "<<thres<<"  ="<<endl
        <<" =                       ="<<endl
        <<" ========================="<<endl
        <<endl;
//
for(sbit16 j=s_nchan[1]-1; j>=0; j--) {
 ubit16 ad1 = j/32;
 ubit16 ad2 = j%32;
 if(j<10) {
  *strdisp<<" "<<j<<" "; 
 } else {
  *strdisp<<j<<" ";
 }
 for(ubit16 k=0; k<s_nchan[0]; k++) {
  if((m_trigRoad[thres][k][ad1])&(1<<ad2)) {
   *strdisp<<"*";
  } else {
   *strdisp<<".";
  }//end-of-if
 }//end-of-for(k
 *strdisp<<" "<<endl;
}//end-of-for(j
*strdisp<<" "<<endl;
*strdisp<<"   00000000001111111111222222222233"<<endl
        <<"   01234567890123456789012345678901"<<endl;
//
 DISP << strdisp->str();
 DISP_DEBUG; 
 delete strdisp;
}//end-of-dispWind
//------------------------------------------------------------------------//
void Matrix::inds(ubit16 *i, ubit16 channel){
//
// input channel: CMA channel address
//
// output i[0] is the address of the 32-bit word to be considered
//        i[1] is the bit number of word with address i[0] corresponding 
//             to channel of CMA
//
*(i+0) = channel/s_wordlen; 
*(i+1) = channel%s_wordlen;
}//end-of-Matrix::inds
//----------------------------------------------------------------------------//
 ubit16 Matrix::char2int(const char *str, CMAword the32[2]) {
 ubit16 outflag=0;
 the32[0]=0;the32[1]=0;
 ubit16 stringLength=strlen(str);
 if(stringLength>16) {
  outflag=1;
 } else {
  for(ubit16 i=0; i<stringLength; i++) {
//        cout<<" Reading character "<<*(str+stringLength-1-i)<<endl;
        if(*(str+stringLength-1-i)=='0')
           the32[i/8]=the32[i/8]+0; 
   else if(*(str+stringLength-1-i)=='1')
           the32[i/8]=the32[i/8] + (intPow(16,i%8))*1;
   else if(*(str+stringLength-1-i)=='2')
           the32[i/8]=the32[i/8] + (intPow(16,i%8))*2;
   else if(*(str+stringLength-1-i)=='3')
           the32[i/8]=the32[i/8] + (intPow(16,i%8))*3;
   else if(*(str+stringLength-1-i)=='4')
           the32[i/8]=the32[i/8] + (intPow(16,i%8))*4;
   else if(*(str+stringLength-1-i)=='5')
           the32[i/8]=the32[i/8] + (intPow(16,i%8))*5;
   else if(*(str+stringLength-1-i)=='6')
           the32[i/8]=the32[i/8] + (intPow(16,i%8))*6;
   else if(*(str+stringLength-1-i)=='7')
           the32[i/8]=the32[i/8] + (intPow(16,i%8))*7;
   else if(*(str+stringLength-1-i)=='8')
           the32[i/8]=the32[i/8] + (intPow(16,i%8))*8;
   else if(*(str+stringLength-1-i)=='9')
           the32[i/8]=the32[i/8] + (intPow(16,i%8))*9;
   else if(*(str+stringLength-1-i)=='a' || *(str+stringLength-1-i)=='A')
           the32[i/8]=the32[i/8] + (intPow(16,i%8))*10;
   else if(*(str+stringLength-1-i)=='b' || *(str+stringLength-1-i)=='B')
           the32[i/8]=the32[i/8] + (intPow(16,i%8))*11;
   else if(*(str+stringLength-1-i)=='c' || *(str+stringLength-1-i)=='C')
           the32[i/8]=the32[i/8] + (intPow(16,i%8))*12;
   else if(*(str+stringLength-1-i)=='d' || *(str+stringLength-1-i)=='D')
           the32[i/8]=the32[i/8] + (intPow(16,i%8))*13;
   else if(*(str+stringLength-1-i)=='e' || *(str+stringLength-1-i)=='E')
           the32[i/8]=the32[i/8] + (intPow(16,i%8))*14;
   else if(*(str+stringLength-1-i)=='f' || *(str+stringLength-1-i)=='F')
           the32[i/8]=the32[i/8] + (intPow(16,i%8))*15;
   else outflag=2; 
  }//end-of-for 
 }

// if(outflag) {
//  the32[0]=0;
//  the32[1]=1;
// } else {
//  CMAword temp = the32[1];
//  the32[1]=the32[0];
//  the32[0]=temp;
// }
 return outflag;
}//end-of-char2int
//----------------------------------------------------------------------------//
CMAword Matrix::intPow (const ubit16 base, const ubit16 expo) {
CMAword output=1;
if(expo) {
 for(ubit16 i=1; i<=expo; i++) {
  output = output*base;
 }//end-of-for
}
return output;  
}//end-of-CMAword Matrix::intPow
//----------------------------------------------------------------------------//
void Matrix::histo(rpcbook *p) {  // rpcbook
p->fill(100,24.);
p->fill(200,35.);
}
