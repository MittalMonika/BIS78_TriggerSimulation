/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include<fstream>
#include<iomanip>
#include<cstring>
#include "TrigT1RPChardware/RXReadOut.h"

using namespace std;

//----------------------------------------------------------------------------//
RXReadOut::RXReadOut(ubit16 sectorID, PadReadOut **PROlist) 
                     : BaseObject(Hardware,"RXReadOut") {
m_sectorID=sectorID;
m_PROlist = PROlist;
DISP<<" WELCOME in RXReadOut "<<endl
    <<" Sector ID= "<<m_sectorID<<endl;
DISP_DEBUG;
reset();
makeFragment();
topRXBody();
DISP<<" Fragement done; now try to read it back "<<endl;
DISP_DEBUG;
for(ubit16 i=0; i<m_numberOfWordsInFragment; i++) {
 DISP<<" data word is "<<hex<<readRXWord()<<dec<<endl;
 DISP_DEBUG;
}
}//end-of-RXReadOut::RXReadOut
//----------------------------------------------------------------------------//
RXReadOut::~RXReadOut() {
}//end-of-RXReadOut::RXReadOut
//----------------------------------------------------------------------------//
void RXReadOut::reset() {
m_Header=0x0000;
m_Footer=0x0000;
for(ubit16 i=0; i<s_numberOfPads; i++) { m_PADFragment[i]=0;}
m_numberOfPDFragments=0;
m_numberOfWordsInFragment=0;
topRXBody();
}//end-of-RXReadOut::reset
//----------------------------------------------------------------------------//
void RXReadOut::makeFragment() {
 makeHeader();
 makeBody();
 makeFooter();
}//end-of-RXReadOut::makeFragment
//----------------------------------------------------------------------------//
void RXReadOut::makeHeader() {
ubit16 headerval[3];
headerval[0] = 0;
headerval[1] = m_sectorID;
headerval[2] = 0; // Status bits (to be defined yet)
m_Header = m_RROS.makeHeader(headerval);
m_numberOfWordsInFragment+=1;
DISP<<" RX Header: "<<hex<<m_Header<<dec<<endl;
DISP_DEBUG;
}//end-of-makeHeader()
//----------------------------------------------------------------------------//
void RXReadOut::makeFooter() {
m_Footer = m_RROS.makeFooter((ubit16)0);
m_numberOfWordsInFragment+=1; 
}//end-of-PadReadOut::makeFooter()
//----------------------------------------------------------------------------//
void RXReadOut::makeBody() {
PadReadOut *PADFragment[s_numberOfPads]={0};
PadReadOutStructure PROS;
ubit16 numberOfPadWords=0;
ubit16 i;
DISP<<" RXReadOut makeBody "<<endl;
DISP_DEBUG;
for(i=0; i<s_numberOfPads; i++) {
 DISP<<" RXReadOut:: "<<i<<" pointer to PadReadOut is "<<*(m_PROlist+i)<<endl;
 DISP_DEBUG;
 if(*(m_PROlist+i)) {
  DISP<<" getting Header from PAD "<<endl;
  DISP_DEBUG;
  PROS = (*(m_PROlist+i))->getHeader();
  ubit16 padid = PROS.padid();
  numberOfPadWords=(*(m_PROlist+i))->numberOfFragmentWords();
  DISP<<" getHeader from RXReadOut done "<<endl 
      <<" il numero di Pad words = "<<numberOfPadWords<<endl
      <<" Identificatore di questa Pad e': "<<padid<<endl;
  DISP_DEBUG;
  if(padid<s_numberOfPads) {
   if(!PADFragment[padid]) {
    PADFragment[padid] = *(m_PROlist+i);
   } else {
    DISP<<" RXReadOut::makeBody: more than one PAD with address "
        <<padid<<endl;
    DISP_DEBUG;
   }//end-of-if(!PADFragment
   DISP<<" PadReadOut: "<<hex<<(*(m_PROlist+i))->readHeader()<<dec<<endl;
   DISP_DEBUG;
  } else {
   DISP<<" RXReadOut::makeBody: padid address "<<padid<<" is bad "<<endl;
   DISP_ERROR;
  }//end-of-if(padid<s_numberOfPads
 }//end-of-if(*(m_PROlist
}//end-of-for
//
// copy PADFragment to the array data member m_PADFragment
//
ubit16 j=0;
for(i=0; i<s_numberOfPads; i++) {
 if(PADFragment[i]) {
  m_PADFragment[j]=PADFragment[i];
  DISP<<" makeBody; number of PAD WOrds="
      <<m_PADFragment[j]->numberOfFragmentWords()<<endl;
  DISP_DEBUG;
  m_numberOfWordsInFragment+=m_PADFragment[j]->numberOfFragmentWords();
  j++;
 }//end-of-if(
}//end-of-for(i
m_numberOfPDFragments=j;
DISP<<" Number of PAD Fragments= "<<m_numberOfPDFragments<<endl;
DISP_DEBUG;
}//end-of-RXReadOut::makeBody
//----------------------------------------------------------------------------//
void RXReadOut::topRXBody() {
m_addressOfWordScanned=0;
m_newPDIndex         =0;
m_newPDRO            =1;
m_numberOfWordsInPDRO=0;
m_numberOfWordsRead  =0;
m_endOfPDFragments   =0;
m_currentPDRO        =0;
}//end-of-topRXBody
//----------------------------------------------------------------------------//
unsigned short int RXReadOut::readHeader() {
return m_Header;
}//end-of-readHeader
//----------------------------------------------------------------------------//
unsigned short int RXReadOut::readFooter() {
return m_Footer;
}//end-of-readFooter
//----------------------------------------------------------------------------//
unsigned short int RXReadOut::readBody() {
ubit16 output=0xffff;

 if(m_newPDRO) {
 
  if(m_newPDIndex<s_numberOfPads) {
   if(m_PADFragment[m_newPDIndex]) {
    m_currentPDRO=m_PADFragment[m_newPDIndex];
    m_currentPDRO->topPADBody();
    m_numberOfWordsInPDRO=m_currentPDRO->numberOfFragmentWords();
    m_numberOfWordsRead=0;
    m_newPDRO=0;
   }//end
   m_newPDIndex++;
  } else {
   m_endOfPDFragments=1;
  }//end
 
 }//end-of-if(m_newPDRO
//
 if(!m_endOfPDFragments) {
  output = m_currentPDRO->readPADWord();
  m_numberOfWordsRead++;
  if(m_numberOfWordsRead>=m_numberOfWordsInPDRO) m_newPDRO=1;
 }//end-of-if(!m_endOfPDFragments
 return output;
}//end-of-readBody()
//----------------------------------------------------------------------------//
unsigned short int RXReadOut::readRXWord() {
ubit16 output=0xffff;
     if(m_addressOfWordScanned==0)                           output=readHeader();
else if(m_addressOfWordScanned==(m_numberOfWordsInFragment-1)) output=readFooter();
else output = readBody();
m_addressOfWordScanned++;
return output;
}//end-of-readRXWord
//----------------------------------------------------------------------------//
void RXReadOut::bytestream(ostream &stream) {
stream<<hex<<m_Header<<dec<<endl;               // header
for(ubit16 i=0; i<m_numberOfPDFragments; i++) {
 m_PADFragment[i]->bytestream(stream);          // body
}//end-of-for(ubit16 i
stream<<hex<<m_Footer<<dec<<endl;               // footer
}//end-of-MatrixReadOut::bytestream
//----------------------------------------------------------------------------//
RXReadOutStructure RXReadOut::getHeader() {
RXReadOutStructure theStruct(m_Header);
return theStruct;
}//end-of-RXReadOut::getHeader
//----------------------------------------------------------------------------//
RXReadOutStructure RXReadOut::getFooter() {
RXReadOutStructure theStruct(m_Footer);
return theStruct;
}//end-of-RXReadOut::getFooter
