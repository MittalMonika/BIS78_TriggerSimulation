/*
   Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include "LArAutoCorrTotalCondAlg.h"

#include "LArElecCalib/LArConditionsException.h"

#include "LArIdentifier/LArOnlineID.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"

#include "GaudiKernel/EventIDRange.h"

//#include <cmath>

LArAutoCorrTotalCondAlg::LArAutoCorrTotalCondAlg(const std::string &name,
                                                 ISvcLocator *pSvcLocator)
    : ::AthAlgorithm(name, pSvcLocator),
      m_LArADC2MeVObjKey("LArADC2MeV"),
      m_LArOnOffIdMappingObjKey("LArOnOffIdMap"),
      m_LArShapeObjKey("LArShapeSym"),
      m_LArAutoCorrObjKey("LArAutoCorrSym"),
      m_LArNoiseObjKey("LArNoiseSym"),
      m_LArPedestalObjKey("LArPedestal"),
      m_LArfSamplObjKey("LArfSamplSym"),
      m_LArMinBiasObjKey("LArMinBiasSym"),
      m_LArAutoCorrTotalObjKey("LArAutoCorrTotal"),
      m_condSvc("CondSvc", name), m_Nminbias(0), m_NoPile(false), m_isMC(true),
      m_isSuperCell(false), m_useMixedOFCOpt(false), m_Nsamples(5),
      m_firstSample(0), m_deltaBunch(1) {
  declareProperty("LArADC2MeVObjKey", m_LArADC2MeVObjKey,
                  "Key to read LArADC2MeV object");
  declareProperty("LArOnOffIdMappingObjKey", m_LArOnOffIdMappingObjKey,
                  "Key to read LArOnOffIdMapping object");
  declareProperty("LArShapeObjKey", m_LArShapeObjKey,
                  "Key to read LArShape object");
  declareProperty("LArAutoCorrObjKey", m_LArAutoCorrObjKey,
                  "Key to read LArAutoCorr object");
  declareProperty("LArNoiseObjKey", m_LArNoiseObjKey,
                  "Key to read LArNoise object");
  declareProperty("LArPedestalObjKey", m_LArPedestalObjKey,
                  "Key to read LArPedestal object");
  declareProperty("LArfSamplObjKey", m_LArfSamplObjKey,
                  "Key to read LArfSampl object");
  declareProperty("LArMinBiasObjKey", m_LArMinBiasObjKey,
                  "Key to read LArMinBias object");
  declareProperty("LArAutoCorrTotalObjKey", m_LArAutoCorrTotalObjKey,
                  "Key to write LArAutoCorrTotal object");
  declareProperty("Nminbias", m_Nminbias);
  declareProperty("NoPile", m_NoPile);
  declareProperty("isMC", m_isMC);
  declareProperty("isSuperCell", m_isSuperCell);
  declareProperty("UseMixedOFCOpt", m_useMixedOFCOpt);
  declareProperty("Nsamples", m_Nsamples, "Max number of samples to use");
  declareProperty(
      "firstSample", m_firstSample,
      "First sample to use for in-time event on the full pulse shape");
  declareProperty("deltaBunch", m_deltaBunch,
                  "Delta between filled bunches in 25 ns units");
}

LArAutoCorrTotalCondAlg::~LArAutoCorrTotalCondAlg() {}

StatusCode LArAutoCorrTotalCondAlg::initialize() {
  ATH_MSG_DEBUG("initialize " << name());

  // CondSvc
  ATH_CHECK(m_condSvc.retrieve());

  // ReadCondHandle initialization
  ATH_CHECK(m_LArShapeObjKey.initialize());
  ATH_CHECK(m_LArAutoCorrObjKey.initialize());
  ATH_CHECK(m_LArOnOffIdMappingObjKey.initialize());
  ATH_CHECK( m_LArADC2MeVObjKey.initialize());

  ATH_CHECK(m_LArAutoCorrTotalObjKey.initialize());

  m_NoPile = false;
  if (m_Nminbias <= 0)
    m_NoPile = true;

  if (!m_NoPile) {
    if (m_isMC) {
      ATH_CHECK(m_LArNoiseObjKey.initialize());
    } else {
      ATH_CHECK(m_LArPedestalObjKey.initialize());
    }
    ATH_CHECK(m_LArfSamplObjKey.initialize());
    ATH_CHECK(m_LArMinBiasObjKey.initialize());
  }

  // WriteCondHandle initialization
  if (m_condSvc->regHandle(this, m_LArAutoCorrTotalObjKey).isFailure()) {
    ATH_MSG_ERROR("Unable to register WriteCondHandle "
                  << m_LArAutoCorrTotalObjKey.fullKey() << " with CondSvc");
    return StatusCode::FAILURE;
  }

  // Number of gains (does this have to be in initialize now b/c of AthenaMT?)
  // Copied from LArADC2MeVCondAlg.cxx
  if (m_isSuperCell) {
    m_nGains = 1;
    // ATH_CHECK(m_larSCOnlineIDKey.initialize());
  } else {
    m_nGains = 3;
    // ATH_CHECK(m_larOnlineIDKey.initialize());
  }

  return StatusCode::SUCCESS;
}

StatusCode LArAutoCorrTotalCondAlg::execute() {

  // WriteHandle setup
  SG::WriteCondHandle<LArAutoCorrTotal> writeHandle(m_LArAutoCorrTotalObjKey);
  // So the following should not be called usually?!
  if (writeHandle.isValid()) {
    ATH_MSG_DEBUG(
        "CondHandle "
        << writeHandle.fullKey() << " is already valid.");
    return StatusCode::SUCCESS;
  }

  // Identifier helper
  // Copied from LArADC2MeVCondAlg.cxx
  // need to understand casting dynamics here
  const LArOnlineID_Base *larOnlineID = nullptr;
  if (m_isSuperCell) {
    // SG::ReadCondHandle<LArOnline_SuperCellID>
    // larOnlineHdl{m_larSCOnlineIDKey}
    const LArOnline_SuperCellID *scidhelper;
    ATH_CHECK(detStore()->retrieve(scidhelper, "LArOnline_SuperCellID"));
    larOnlineID = scidhelper; // cast to base-class
  } else {                    // regular cells
    // SG::ReadCondHandle<LArOnlineID> larOnlineHdl{m_larOnlineIDKey};
    const LArOnlineID *idhelper;
    ATH_CHECK(detStore()->retrieve(idhelper, "LArOnlineID"));
    larOnlineID = idhelper; // cast to base-class
  }
  // Mapping helper
  const LArOnOffIdMapping *larOnOffIdMapping = nullptr;
  SG::ReadCondHandle<LArOnOffIdMapping> larOnOffIdMappingHdl{
    m_LArOnOffIdMappingObjKey
  };
  larOnOffIdMapping = *larOnOffIdMappingHdl;
  if (larOnOffIdMapping == nullptr) {
    ATH_MSG_ERROR("Failed to retrieve LArOnOffIdMapping object");
  }

  // Get pointers to inputs
  // Retrieve validity ranges and determine their intersection
  // //FIXME use new style of intersection checking, see twiki
  EventIDRange rangeShape, rangeAutoCorr;

  SG::ReadCondHandle<ILArShape> ShapeHdl{ m_LArShapeObjKey };
  // FIXME: should check if handle is properly created and/or check if handle is
  // properly retrieved
  // operator star of a ReadCondHandle returns a const pointer to type T
  const ILArShape *larShape{ *ShapeHdl };
  if (!ShapeHdl.range(rangeShape)) {
    ATH_MSG_ERROR("Failed to retrieve validity range for " << ShapeHdl.key());
  }

  SG::ReadCondHandle<ILArAutoCorr> AutoCorrHdl{ m_LArAutoCorrObjKey };
  const ILArAutoCorr *larAutoCorr{ *AutoCorrHdl };
  if (!AutoCorrHdl.range(rangeAutoCorr)) {
    ATH_MSG_ERROR("Failed to retrieve validity range for "
                  << AutoCorrHdl.key());
  }

  SG::ReadCondHandle<LArADC2MeV> ADC2MeVHdl{ m_LArADC2MeVObjKey };
  const LArADC2MeV *larADC2MeV = nullptr;
  larADC2MeV = *ADC2MeVHdl;
  if (larADC2MeV == nullptr) {
    ATH_MSG_ERROR("Failed to retrieve LArADC2MeV object");
  }

  // Determine intersection of the two required objects
  EventIDRange rangeIntersection =
      EventIDRange::intersect(rangeShape, rangeAutoCorr);
  // if ( rangeIntersection.start() > rangeIntersection.stop() ) {
  // ATH_MSG_ERROR( "Invalid intersection range: " << rangeIntersection);
  // return StatusCode::FAILURE;
  //}

  // Consider the determinstic objects
  // checking isMC and NoPile again seems very clumsy. How to check if key has
  // already been initialized?
  const ILArNoise *larNoise = nullptr;
  const ILArPedestal *larPedestal = nullptr;
  const ILArfSampl *larfSampl = nullptr;
  const ILArMinBias *larMinBias = nullptr;

  if (!m_NoPile) {
    if (m_isMC) {
      EventIDRange rangeNoise;
      SG::ReadCondHandle<ILArNoise> NoiseHdl{ m_LArNoiseObjKey };
      larNoise = *NoiseHdl;
      if (!NoiseHdl.range(rangeNoise)) {
        ATH_MSG_ERROR("Failed to retrieve validity range for "
                      << NoiseHdl.key());
      }
      rangeIntersection.intersect(rangeIntersection, rangeNoise);
    } else {
      EventIDRange rangePedestal;
      SG::ReadCondHandle<ILArPedestal> PedestalHdl{ m_LArPedestalObjKey };
      larPedestal = *PedestalHdl;
      if (!PedestalHdl.range(rangePedestal)) {
        ATH_MSG_ERROR("Failed to retrieve validity range for "
                      << PedestalHdl.key());
      }
      rangeIntersection.intersect(rangeIntersection, rangePedestal);
    }

    EventIDRange rangefSampl;
    SG::ReadCondHandle<ILArfSampl> fSamplHdl{ m_LArfSamplObjKey };
    larfSampl = *fSamplHdl;
    if (!fSamplHdl.range(rangefSampl)) {
      ATH_MSG_ERROR("Failed to retrieve validity range for "
                    << fSamplHdl.key());
    }
    rangeIntersection.intersect(rangeIntersection, rangefSampl);

    EventIDRange rangeMinBias;
    SG::ReadCondHandle<ILArMinBias> MinBiasHdl{ m_LArMinBiasObjKey };
    larMinBias = *MinBiasHdl;
    if (!MinBiasHdl.range(rangeMinBias)) {
      ATH_MSG_ERROR("Failed to retrieve validity range for "
                    << MinBiasHdl.key());
    }
    rangeIntersection.intersect(rangeIntersection, rangeMinBias);
  }

  // Check sanity of final range
  if (rangeIntersection.start() > rangeIntersection.stop()) {
    ATH_MSG_ERROR("Invalid intersection range: " << rangeIntersection);
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("IOV found from intersection for AutoCorrTotal object: "
               << rangeIntersection);

  // make output object
  // dimensions: number of gains x number of channel IDs x elements of
  // AutoCorrTotal
  std::unique_ptr<LArAutoCorrTotal> larAutoCorrTotal =
      std::make_unique<LArAutoCorrTotal>(larOnlineID, larOnOffIdMapping, m_nGains);

  std::vector<HWIdentifier>::const_iterator it = larOnlineID->channel_begin();
  std::vector<HWIdentifier>::const_iterator it_e = larOnlineID->channel_end();
  int count = 0;
  int count2 = 0;

  for (; it != it_e; ++it) {
    count++;
    const HWIdentifier chid = *it;
    const IdentifierHash hid = larOnlineID->channel_Hash(chid);
    // const unsigned int id32 = chid.get_identifier32().get_compact();

    if (larOnOffIdMapping->isOnlineConnected(chid)) {
        count2++;

      if (m_useMixedOFCOpt) {
        const bool isEMB = larOnlineID->isEMBchannel(chid);
        const bool isEMECOW = larOnlineID->isEMECOW(chid);
        if (isEMB || isEMECOW) {
          ATH_MSG_DEBUG("No Pileup AutoCorr for ChID 0x" << MSG::hex << chid
                                                         << MSG::dec);
          m_NoPile = true;
        } else {
          ATH_MSG_DEBUG("Using Pileup AutoCorr for ChID 0x" << MSG::hex << chid
                                                            << MSG::dec);
          m_NoPile = false;
        }
      }

      for (size_t igain = 0; igain < m_nGains; igain++) {
        const ILArShape::ShapeRef_t Shape = larShape->Shape(chid, igain);
        const int nsamples_shape = static_cast<int>(Shape.size());

        const ILArAutoCorr::AutoCorrRef_t AC =
            larAutoCorr->autoCorr(chid, igain);

        int nsamples_AC_OFC = AC.size() + 1;

        if (nsamples_AC_OFC > m_Nsamples) {
          nsamples_AC_OFC = m_Nsamples;
        }

        // fix HEC first sample +1 if the firstSample is 0 and nsamples 4
        unsigned int ihecshift = 0;
        if (larOnlineID->isHECchannel(chid) && nsamples_AC_OFC == 4 &&
            m_firstSample == 0) {
          ihecshift = 1;
          // ATH_MSG_DEBUG( "Using firstSample +1 for HEC ChID 0x" << MSG::hex
          // << id << MSG::dec  );
        }

        //:::::::::::::::::::::::::::::::
        // NB:
        // nsamples_shape    = number of samples of the Shape function (e.g 32)
        // nsamples_AC_OFC = size of AC matrix & OFC vector (e.g 5 in Atlas)
        //:::::::::::::::::::::::::::::::
        float fSigma2 = 0.;
        if (!m_NoPile) {
          float SigmaNoise;
          if (m_isMC)
            SigmaNoise = larNoise->noise(chid, igain);
          else {
            float RMSpedestal = larPedestal->pedestalRMS(chid, igain);
            if (RMSpedestal > (1.0 + LArElecCalib::ERRORCODE))
              SigmaNoise = RMSpedestal;
            else
              SigmaNoise = 0.; //(we will have the ERROR message below)
          }
          float fSampl = larfSampl->FSAMPL(chid);
          float MinBiasRMS = larMinBias->minBiasRMS(chid);
          if (fSampl != 0)
            MinBiasRMS /= fSampl;
          const std::vector<float> polynom_adc2mev =
              larADC2MeV->ADC2MEV(hid, igain);
          float Adc2MeV = 0.;
          if (polynom_adc2mev.size() > 0) {
            Adc2MeV = (polynom_adc2mev)[1];
          }
          if (SigmaNoise != 0 && Adc2MeV != 0)
            fSigma2 = pow(MinBiasRMS / (SigmaNoise * Adc2MeV), 2);

          if (fSampl == 0 || SigmaNoise == 0 || Adc2MeV == 0) {
            if (m_isMC) {
              ATH_MSG_ERROR(larOnlineID->show_to_string(
                                larOnOffIdMapping->cnvToIdentifier(chid))
                            << "fSampl (" << fSampl << "), SigmaNoise ("
                            << SigmaNoise << ") or Adc2MeV (" << Adc2MeV
                            << ") null "
                            << "=> AutoCorrTotal = only AutoCorr elect. part ");
            }
            fSigma2 = 0.;
          }
          // warning: MinBiasRMS is in MeV (at the scale of the hits)
          //         SigmaNoise is in ADC counts
          //  so MinBiasRMS/fScale and SigmaNoise*Adc2MeV are the same scale
          //  (MeV at the scale of the cells)
        } // end if m_NoPile

        // get in vTerms all the possible non trivial N(N-1)/2 terms of the
        // autocorrelation matrix
        int nsize_tot = (nsamples_AC_OFC - 1) * (nsamples_AC_OFC) / 2;

        std::vector<float> vTerms;

        vTerms.resize(2 * nsize_tot + nsamples_AC_OFC, 0.);
        //:::::::::::::::::::::::::::::::

        for (int j1 = 0; j1 < nsamples_AC_OFC - 1; j1++) {
          for (int j2 = j1 + 1; j2 < nsamples_AC_OFC; j2++) {
            int l = abs(j2 - j1) - 1;
            int index =
                j1 * nsamples_AC_OFC - j1 * (j1 + 1) / 2 + j2 - (j1 + 1);
            vTerms[index] = AC[l];
          }
        }

        // 2nd terms :
        for (int j1 = 0; j1 < nsamples_AC_OFC - 1; ++j1) {
          for (int j2 = j1 + 1; j2 < nsamples_AC_OFC; j2++) {
            int index =
                j1 * nsamples_AC_OFC - j1 * (j1 + 1) / 2 + j2 - (j1 + 1);
            float Rij = 0;
            for (int k = 0; k < nsamples_shape; ++k) {
              if ((j2 - j1 + k) >= 0 && (j2 - j1 + k) < nsamples_shape) {
                int ibunch = 0;
                if ((j1 + m_firstSample + ihecshift - k) % m_deltaBunch == 0)
                  ibunch = 1;
                Rij += Shape[k] * Shape[j2 - j1 + k] * ibunch;
              }
            }
            vTerms[nsize_tot + index] = fSigma2 * Rij;
          }
        }

        // 3rd term : RMS of pileup per samples (multiplied by fSigma2)
        for (int j1 = 0; j1 < nsamples_AC_OFC; j1++) {
          float Rms2i = 0;
          for (int k = 0; k < nsamples_shape; ++k) {
            int ibunch = 0;
            if ((j1 + m_firstSample + ihecshift - k) % m_deltaBunch == 0)
              ibunch = 1;
            Rms2i += pow(Shape[k], 2) * ibunch;
          }
          vTerms[2 * nsize_tot + j1] = fSigma2 * Rms2i;
        }

        // storage
        larAutoCorrTotal->set(hid, igain, vTerms);

      } //(loop on gains)

    } else // unconnected
      for (unsigned int igain = 0; igain < 3; igain++) {
        unsigned nsize_tot = (m_Nsamples - 1) * (m_Nsamples) + m_Nsamples;
        std::vector<float> empty(nsize_tot, 0.);
        larAutoCorrTotal->set(hid, igain, empty);
      }
  }


  ATH_CHECK(writeHandle.record(rangeIntersection,larAutoCorrTotal.release()));

  ATH_MSG_INFO("LArAutoCorrTotal Ncell " << count);
  // ATH_MSG_INFO( "LArAutoCorrTotal Nsymcell " << count2  );
  ATH_MSG_DEBUG("end of loop over cells ");

  return StatusCode::SUCCESS;
}
