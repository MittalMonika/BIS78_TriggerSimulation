/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//$Id: hepevtcom.cxx,v 1.1 2005-10-28 12:07:39 dellacqu Exp $
// ======================================================================
//      PARAMETER (NMXHEP=2000) 
//      COMMON/HEPEVT/NEVHEP,NHEP,ISTHEP(NMXHEP),IDHEP(NMXHEP), 
//     &        JMOHEP(2,NMXHEP),JDAHEP(2,NMXHEP),PHEP(5,NMXHEP),VHEP(4,NMXHEP)
// ======================================================================
///**********************************************************/
///*           D E S C R I P T I O N :                      */
///*--------------------------------------------------------*/
///* NEVHEP          - event number (or some special meaning*/
///*                    (see documentation for details)     */
///* NHEP            - actual number of entries in current  */
///*                    event.                              */
///* ISTHEP[IHEP]    - status code for IHEP'th entry - see  */
///*                    documentation for details           */
///* IDHEP [IHEP]    - IHEP'th particle identifier according*/
///*                    to PDG.                             */
///* JMOHEP[IHEP][0] - pointer to position of 1st mother    */
///* JMOHEP[IHEP][1] - pointer to position of 2nd mother    */
///* JDAHEP[IHEP][0] - pointer to position of 1st daughter  */
///* JDAHEP[IHEP][1] - pointer to position of 2nd daughter  */
///* PHEP  [IHEP][0] - X momentum [Gev/c]                   */
///* PHEP  [IHEP][1] - Y momentum [Gev/c]                   */
///* PHEP  [IHEP][2] - Z momentum [Gev/c]                   */
///* PHEP  [IHEP][3] - Energy [Gev]                         */
///* PHEP  [IHEP][4] - Mass[Gev/c^2]                        */
///* VHEP  [IHEP][0] - X vertex [mm]                        */
///* VHEP  [IHEP][1] - Y vertex [mm]                        */
///* VHEP  [IHEP][2] - Z vertex [mm]                        */
///* VHEP  [IHEP][3] - production time [mm/c]               */
///*========================================================*/
//
// This interface to HEPEVT common block treats the block as
// an array of bytes --- the precision and number of entries 
// is determined "on the fly" by the wrapper and used to decode
// each entry.
//
// HEPEVT_EntriesAllocation is the maximum size of the HEPEVT common block 
// that can be interfaced. It is NOT the actual size of the HEPEVT common 
// used in each individual application. The actual size can be changed on
// the fly using HepMC::HEPEVT_Wrapper::set_max_number_entries().
// Thus HEPEVT_EntriesAllocation should typically be set
// to the maximum possible number of entries --- 10000 is a good choice
// (and is the number used by ATLAS versions of Pythia).

#include <ctype.h>

enum {HEPEVT_EntriesAllocation=10000};

const unsigned int hepevt_bytes_allocation = 
sizeof(long int) * ( 2 + 4 * HEPEVT_EntriesAllocation )
  + sizeof(double) * ( 9 * HEPEVT_EntriesAllocation );

extern "C" {
  struct hepevt{
    char data[hepevt_bytes_allocation];
  };
}

hepevt hepevt_;

