// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGT1RESULT_MUCTPI_RDO_H
#define TRIGT1RESULT_MUCTPI_RDO_H

// STL include(s):
#include <vector>
#include <string>
#include <cstdint>

// Gaudi/Athena include(s):
#include "AthenaKernel/CLASS_DEF.h"

/**
 *   $Date: 2007-07-05 13:26:22 $
 *
 *   @short Class representing the readout data of the MuCTPI hardware and simulation
 *
 *          The MuCTPI hardware and the simulation can send all the muon candidates
 *          recevied in a maximal window spanning +-2 BCs around the triggered BC 
 *          (giving a window of maximally 5 BCs) together with the muon multiplicities
 *          in the same window to the readout system. This class represents the ROD
 *          fragment generated by the MuCTPI to Athena.
 *
 *          For a detailed description of the data format of the MuCTPI, see
 *          the MIROD documentation (https://edms.cern.ch/file/248757/1/mirod.pdf).
 *
 *     @see LVL1MUCTPI::L1Muctpi
 *     @see MuCTPIByteStreamCnv
 *
 *  @author Tadashi Maeno <tmaeno@bnl.gov>
 *  @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
 *  @author David Berge <David.Berge@cern.ch>
 *
 * @version $Revision: 1.9 $
 */
class MuCTPI_RDO {

public:
  /// Enumeration defining the different sources for the muon candidates
  enum SectorLocation { BARREL = 0, ENDCAP = 1, FORWARD = 2 };

  /// "Intermediate" constructor
  /**
   * This constructor is tipically used for filling the object with information
   * about muon candidates from one bunch crossing.
   *
   * @param candidateMultiplicity   The multiplicity word with the 6 separate 3-bit muon multiplicities
   * @param dataWord                The muon candidate data words
   */
  MuCTPI_RDO( const uint32_t candidateMultiplicity,
              const std::vector< uint32_t >& dataWord )
    : m_dataWord( dataWord ) {
    setCandidateMultiplicity( candidateMultiplicity );
  }

  /// Full constructor
  /**
   * This constructor can be used to fill the object with the multiplicity and data words
   * from an up to +-5 buch crossing wide window.
   *
   * @param candidateMultiplicity   The multiplicity words with the 6 separate 3-bit muon multiplicities
   * @param dataWord                The muon candidate data words
   */
  MuCTPI_RDO( const std::vector< uint32_t >& candidateMultiplicity,
              const std::vector< uint32_t >& dataWord )
    : m_candidateMultiplicity( candidateMultiplicity ),
      m_dataWord( dataWord ) {
  }

  /// Default constructor
  /**
   * A parameter-less constructor to be able to use the class with POOL.
   */
  MuCTPI_RDO ()
    : m_candidateMultiplicity( 0 ),
      m_dataWord( 0 ) {}

  /// Destructor
  /**
   * The destructor doesn't really have to do anything.
   */
  ~MuCTPI_RDO () {}

  /// Function <strong>adding</strong> a multiplicity word to the object
  /**
   * Be careful, that contrary to its name, this function doesn't reset the multiplicity
   * word list, it only adds the specified word to the end of the list.
   *
   * @param candidateMultiplicity  The multiplicity word to be added to the object
   */
  void setCandidateMultiplicity( const uint32_t candidateMultiplicity ) {
    m_candidateMultiplicity.push_back(candidateMultiplicity);
  }
  /// Function returning the candidate multiplicity for the triggered bunch crossing
  /**
   * In case of reading out multiple BCs, the one that is considered to have triggered
   * the event is always the "center" one. (In principle you can only read out 1, 3 or
   * 5 BCs, not 2 or 4!) This function takes care of returning this one word.
   *
   * @return The "central" multiplicity word
   */
  uint32_t candidateMultiplicity() const {
    if( m_candidateMultiplicity.size() == 0 ) return 0;
    else return m_candidateMultiplicity[ ( m_candidateMultiplicity.size() - 1 ) / 2 ];
  }
  /// Function returning all candidate multiplicities
  /**
   * This function can be used to retrieve the candidate multiplicities from all the saved
   * bunch crossings.
   *
   * @return A vector of all the candidate multiplicity words
   */
  const std::vector< uint32_t >& getAllCandidateMultiplicities() const {
    return m_candidateMultiplicity;
  }

  /// Function <strong>adding</strong> a data word to the object
  /**
   * Just like with MuCTPI_RDO::setCandidateMultiplicity, be careful that this function also
   * only adds the specified data word to the list of stored data words, it doesn't reset
   * the list.
   *
   * @param dataWord  The 32-bit data word to be added to the object
   */
  void setDataWord( const uint32_t dataWord ) {
    m_dataWord.push_back( dataWord );
  }
  /// Function returning the muon candidate data words
  /**
   * The muon candidate data words returned by this function are very similar to the muon RoI words,
   * but they contain a bit more information. This is the last 3 bits of the BCID in which they
   * were created, which is important to be able to separate the muon candidates from the different
   * bunch crossings.
   *
   * @return A vector of all the muon candidate data words
   */
  const std::vector< uint32_t >& dataWord() const {
    return m_dataWord;
  }

  /// Binary 111 representing the maximal multiplicity value for a given threshold
  static const uint32_t MULT_VAL = 7;
  /// Number of multiplicity bits reserved per threshold
  static const uint32_t MULT_BITS = 3;
  /// Defining the number of p<sub>T</sub> thresholds in the system
  static const uint32_t MULT_THRESH_NUM = 6;
  /// Telling that the 3-bit BCID comes at "position 7" in the multiplicity word
  static const uint32_t MULT_BCID_POS = 7;

  /// Weird mask for the bit showing if more than two muon candidates were in the trigger sector
  static const uint32_t CAND_OVERFLOW_MASK = 0x1;
  /// Position of the candidate overflow mask
  static const uint32_t CAND_OVERFLOW_SHIFT = 0;
  /// Weird mask for the bit showing if more than one muon candidates were in the sector RoI
  static const uint32_t ROI_OVERFLOW_MASK = 0x1;
  /// Position of the RoI overflow mask
  static const uint32_t ROI_OVERFLOW_SHIFT = 1;

  /// Bit in the candidate's address turned on for endcap candidates
  static const uint32_t ENDCAP_ADDRESS_MASK = 0x80;
  /// Bit in the candidate's address turned on for forward candidates
  static const uint32_t FORWARD_ADDRESS_MASK = 0x40;

  /// Mask for extracting the sector ID for endcap candidates from the data word
  static const uint32_t ENDCAP_SECTORID_MASK = 0x3f;
  /// Mask for extracting the sector ID for forward candidates from the data word
  static const uint32_t FORWARD_SECTORID_MASK = 0x1f;
  /// Mask for extracting the sector ID for barrel candidates from the data word
  static const uint32_t BARREL_SECTORID_MASK = 0x1f;
  /// Mask for the bit showing which hemisphere the candidate came from.(1: positive; 0: negative)
  static const uint32_t SECTOR_HEMISPHERE_MASK = 0x1;

  /// Mask for extracting the RoI for barrel candidates from the data words
  static const uint32_t BARREL_ROI_MASK = 0x1f;
  /// Mask for extracting the RoI for endcap candidates from the data words
  static const uint32_t ENDCAP_ROI_MASK = 0xff;
  /// Mask for extracting the RoI for forward candidates from the data words
  static const uint32_t FORWARD_ROI_MASK = 0x3f;
  /// Position of the RoI bits in the data word
  static const uint32_t ROI_SHIFT = 2;

  /// Mask for extracting the overlap bits for barrel candidates from the data words
  static const uint32_t BARREL_OL_MASK = 0x3;
  /// Position of the overlap bits in barrel data words
  static const uint32_t BARREL_OL_SHIFT = 9;
  /// Mask for extracting the overlap bits for endcap candidates from the data words
  static const uint32_t ENDCAP_OL_MASK = 0x1;
  /// Position of the overlap bits in endcap data words
  static const uint32_t ENDCAP_OL_SHIFT = 10;

  /// Mask for extracting the p<sub>T</sub> threshold passed by the candidate from the data word
  static const uint32_t CAND_PT_MASK = 0x7;
  /// Position of the p<sub>T</sub> threshold bits in the data words
  static const uint32_t CAND_PT_SHIFT = 11;

  /// Mask for extracting the last 3 bits of the BCID of the muon candidate from the data word
  static const uint32_t CAND_BCID_MASK = 0x7;
  /// Position of the BCID bits in the data words
  static const uint32_t CAND_BCID_SHIFT = 14;

  /// Mask for extracting the address of the muon candidate from the data word
  static const uint32_t CAND_SECTOR_ADDRESS_MASK = 0xff;
  /// Position of the muon candidate's address in the data word
  static const uint32_t CAND_SECTOR_ADDRESS_SHIFT = 17;

  /// Mask for extracting the bit from the data word showing whether the candidate had the highest p<sub>T</sub> in the sector
  static const uint32_t CAND_HIGHEST_PT_MASK = 0x1;
  /// Position of the "highest p<sub>T</sub>" bit
  static const uint32_t CAND_HIGHEST_PT_SHIFT = 25;

  /// Mask for extracting the bit from the data word showing if the muon candidate was sent to the RoIB
  static const uint32_t CAND_SENT_ROI_MASK = 0x1;
  /// Position of the "candidate sent to RoIB" bit.
  static const uint32_t CAND_SENT_ROI_SHIFT = 26;

  /// Position of the bit turned on for the multiplicity words that distinguishes them from the data words
  static const uint32_t MULT_WORD_FLAG_SHIFT = 29;

  /// Position of the bit specifying the candidate's sign
  static const uint32_t CAND_TGC_CHARGE_SIGN_SHIFT = 27;

  /// Position of the bit specifying if a candidate was vetoed in the multiplicity sum
  static const uint32_t CAND_VETO_SHIFT = 28;

private:
  /// Variable storing the multiplicity word(s) sent to the CTP
  std::vector< uint32_t > m_candidateMultiplicity;
  /// Variable storing the muon data words read out
  std::vector< uint32_t > m_dataWord;

}; // class MuCTPI_RDO

CLASS_DEF( MuCTPI_RDO , 6270 , 0 )

#endif // TRIGT1RESULT_MUCTPI_RDO_H
