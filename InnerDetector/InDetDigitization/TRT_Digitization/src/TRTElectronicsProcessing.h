/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRTELECTRONICSPROCESSING_H
#define TRTELECTRONICSPROCESSING_H

#include <vector>

class TRTDigit;
class TRTElectronicsNoise;
class IAtRndmGenSvc;
#include "CLHEP/Random/RandomEngine.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaKernel/MsgStreamMember.h"

class TRTDigSettings;

/**
 * Electronics Processing
 */
class TRTElectronicsProcessing {
public:
  TRTElectronicsProcessing( const TRTDigSettings* digset,
			    ServiceHandle <IAtRndmGenSvc> atRndmGenSvc,
			    TRTElectronicsNoise * electronicsnoise );
  ~TRTElectronicsProcessing();

  MsgStream& msg (MSG::Level lvl) const { return m_msg << lvl; }
  bool msgLevel  (MSG::Level lvl) { return m_msg.get().level() <= lvl; }

  /**
   * TRT energy deposit
   */
  class Deposit {
  public:
    /**
     * Create deposit
     */
    Deposit(const double& e, const double& t): m_energy(e), m_time(t) {}

    double energy() const { return m_energy; } /**< Get energy of deposit */
    double time() const { return m_time; }     /**< Get time of deposit */


  private:
    double m_energy;  /**< Energy */
    double m_time;    /**< Time   */
  };

  /**
   * Process deposits in a straw
   * -# fill cluster energies into appropriate time bins
   * -# signal shaping
   * -# add analog electronics noise
   * -# discriminator response
   * -# encode into a single digit
   *
   * @param deposits:       all deposits, this straw
   * @param hitID:          ID of straw
   * @param outdigit:       output TRTDigit
   * @param lowthreshold:   discriminator value, low threshold
   * @param noiseamplitude: noise amplitude
   * @param highthreshold:  discriminator value, high threshold
   */
  void ProcessDeposits( const std::vector<Deposit>&,
			const int& hitID,
			TRTDigit& outdigit,
                        std::vector<int>& m_fastOrHtDiscriminator,
			double lowthreshold,
			const double& noiseamplitude,
			bool isArgonStraw,
			double highthreshold = -1.0
		      );


  double getHighThreshold ( int hitID, bool isArgonStraw);


private:
  //NB copy-constructor and assignment operator declared, but not defined.
  TRTElectronicsProcessing(const TRTElectronicsProcessing&);
  TRTElectronicsProcessing& operator= (const TRTElectronicsProcessing&);

  //--- Initialization methods

  /**
   * Initialize constants. Call TabulateSignalShape()
   */
  void Initialize();

  /**
   * Tabulate low and high level signal shapes from functions
   * LowThresholdSignalShape() and HighThresholdSignalShape(), respectively
   */
  void TabulateSignalShape();

  //---

  /**
   * Shape electron drift signal according to appropriate signal shapes.
   * - input: @c m_energyDistribution[]
   * - @param isArgonStraw: straw type (Argon or Xenon)
   * - @param isStreamer:   streamer/non-streamer mode
   * - output: @c m_lowThresholdSignal[], @c m_highThresholdSignal[]
   */
  void SignalShaping(bool isArgonStraw, bool isStreamer);

  /**
   * Simulate discriminator response
   * - input: @c m_lowThresholdSignal[], @c m_highThresholdSignal[]
   * - output: @c m_lowThresholdDiscriminator[],
   *           @c m_highThresholdDiscriminator[],
   * @param lowthreshold:   low threshold discriminator setting
   * @param highthreshold: high threshold discriminator setting
   */
  void DiscriminatorResponse(const double& lowthreshold,
			     const double& highthreshold) const;

  /**
   * Encode 27 bit digit from discriminator response
   * [8 low + 1 high + 8 low + 1 high + 8 low + 1 high]:
   * - input: @c m_lowThresholdDiscriminator[],
   *           @c m_highThresholdDiscriminator[]
   *
   * @return digit
   */
  unsigned EncodeDigit() const;


  const TRTDigSettings* m_settings;

  TRTElectronicsNoise * m_pElectronicsNoise;
  CLHEP::HepRandomEngine * m_pHRengine;
  CLHEP::HepRandomEngine * m_pHRengine2;

  // FastOR
  bool m_doFastOr;
  bool m_doStreamer;
  double m_streamerThreshold;
  int nBinsOverHT() const; // ToHT/0.78125 in streamer mode

  double m_timeInterval;       /**< Time interval covered by digit [75 ns] */

  int m_totalNumberOfBins;     /**< Total no. internal bins             */
  int m_numberOfPreZeroBins;   /**< No. internal bins before main event */
  int m_numberOfPostZeroBins;  /**< No. internal bins after main event  */

  double m_binWidth;   /**< Time interval corresponding to each internal bin*/

  int m_minDiscriminatorWidthInBinWidths; /**< Min. discriminator width [internal bins]*/
  int m_discriminatorSettlingTimeInBinWidths; /**< Discriminator settling time [bins]  */
  int m_discriminatorDeadTimeInBinWidths;     /**< Discriminator dead time [int. bins] */
  int m_minWidthMinusSettlingTimeInBinWidths; /**< Min. discriminator time minus settling * time [internal bins] */

  double m_lowThresholdBar[2]; /**<  Low threshold discriminator setting */
  double m_lowThresholdEC[2];  /**<  Low threshold discriminator setting */

  unsigned int m_maskA;   /**< mask - ever used? */
  unsigned int m_maskB;   /**< mask - ever used? */
  unsigned int m_maskC;   /**< mask - ever used? */
  unsigned int m_maskHT;  /**< mask - ever used? */

  // Signal shaping functions for low and high thresholds are stored as discrete amplitudes.
  // There are four LT\HT pairs of amplitudes with iMode = isArgonStraws + 2*isStreamer:
  //   iMode = 0 (isArgon = 0, isStreamer = 0) : Xenon straw in proportional-mode
  //   iMode = 1 (isArgon = 1, isStreamer = 0) : Argon straw in proportional-mode
  //   iMode = 2 (isArgon = 0, isStreamer = 1) : Xenon straw in streamer-mode
  //   iMode = 3 (isArgon = 1, isStreamer = 1) : Argon straw in streamer-mode
  std::vector<double> m_lowThresholdSignalShape[4];
  std::vector<double> m_highThresholdSignalShape[4];

  // Deposit energy in timed bins before shaping.
  double* m_energyDistribution;

  // LT and HT signals after shaping
  std::vector<double> m_lowThresholdSignal;
  std::vector<double> m_highThresholdSignal;

  // LT and HT discriminated signals (bits).
  int* m_lowThresholdDiscriminator;  /**< Signal after discrimination */
  int* m_highThresholdDiscriminator; /**< Signal after discrimination */

  mutable Athena::MsgStreamMember m_msg;

};

#endif
