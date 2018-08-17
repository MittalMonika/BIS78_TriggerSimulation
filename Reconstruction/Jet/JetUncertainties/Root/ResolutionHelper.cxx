/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/


#include "JetUncertainties/ResolutionHelper.h"
#include "JetUncertainties/Helpers.h"

#include "TEnv.h"
#include "TFile.h"

namespace jet
{

ResolutionHelper::ResolutionHelper(const std::string& name, const std::string& jetDef)
    : asg::AsgMessaging(name)
    , m_name(name)
    , m_jetDef(jetDef)
    , m_isInit(false)
    , m_smearOnlyMC(false)
    , m_ptNomHistData(NULL)
    , m_ptNomParamData(CompParametrization::UNKNOWN)
    , m_ptNomHistMC(NULL)
    , m_ptNomParamMC(CompParametrization::UNKNOWN)
    , m_fvNomHistData(NULL)
    , m_fvNomParamData(CompParametrization::UNKNOWN)
    , m_fvNomHistMC(NULL)
    , m_fvNomParamMC(CompParametrization::UNKNOWN)
    , m_mQCDNomHistData(NULL)
    , m_mQCDNomParamData(CompParametrization::UNKNOWN)
    , m_mQCDNomHistMC(NULL)
    , m_mQCDNomParamMC(CompParametrization::UNKNOWN)
    , m_mWZNomHistData(NULL)
    , m_mWZNomParamData(CompParametrization::UNKNOWN)
    , m_mWZNomHistMC(NULL)
    , m_mWZNomParamMC(CompParametrization::UNKNOWN)
    , m_mHbbNomHistData(NULL)
    , m_mHbbNomParamData(CompParametrization::UNKNOWN)
    , m_mHbbNomHistMC(NULL)
    , m_mHbbNomParamMC(CompParametrization::UNKNOWN)
    , m_mTopNomHistData(NULL)
    , m_mTopNomParamData(CompParametrization::UNKNOWN)
    , m_mTopNomHistMC(NULL)
    , m_mTopNomParamMC(CompParametrization::UNKNOWN)
{ }

ResolutionHelper::ResolutionHelper(const ResolutionHelper& toCopy)
    : asg::AsgMessaging(toCopy.m_name)
    , m_name(toCopy.m_name)
    , m_jetDef(toCopy.m_jetDef)
    , m_isInit(toCopy.m_isInit)
    , m_smearOnlyMC(toCopy.m_smearOnlyMC)
    , m_ptNomHistData(!toCopy.m_ptNomHistData ? NULL : new UncertaintyHistogram(*toCopy.m_ptNomHistData))
    , m_ptNomParamData(toCopy.m_ptNomParamData)
    , m_ptNomHistMC(!toCopy.m_ptNomHistMC ? NULL : new UncertaintyHistogram(*toCopy.m_ptNomHistMC))
    , m_ptNomParamMC(toCopy.m_ptNomParamMC)
    , m_fvNomHistData(!toCopy.m_fvNomHistData ? NULL : new UncertaintyHistogram(*toCopy.m_fvNomHistData))
    , m_fvNomParamData(toCopy.m_fvNomParamData)
    , m_fvNomHistMC(!toCopy.m_fvNomHistMC ? NULL : new UncertaintyHistogram(*toCopy.m_fvNomHistMC))
    , m_fvNomParamMC(toCopy.m_fvNomParamMC)
    , m_mQCDNomHistData(!toCopy.m_mQCDNomHistData ? NULL : new UncertaintyHistogram(*toCopy.m_mQCDNomHistData))
    , m_mQCDNomParamData(toCopy.m_mQCDNomParamData)
    , m_mQCDNomHistMC(!toCopy.m_mQCDNomHistMC ? NULL : new UncertaintyHistogram(*toCopy.m_mQCDNomHistMC))
    , m_mQCDNomParamMC(toCopy.m_mQCDNomParamMC)
    , m_mWZNomHistData(!toCopy.m_mWZNomHistData ? NULL : new UncertaintyHistogram(*toCopy.m_mWZNomHistData))
    , m_mWZNomParamData(toCopy.m_mWZNomParamData)
    , m_mWZNomHistMC(!toCopy.m_mWZNomHistMC ? NULL : new UncertaintyHistogram(*toCopy.m_mWZNomHistMC))
    , m_mWZNomParamMC(toCopy.m_mWZNomParamMC)
    , m_mHbbNomHistData(!toCopy.m_mHbbNomHistData ? NULL : new UncertaintyHistogram(*toCopy.m_mHbbNomHistData))
    , m_mHbbNomParamData(toCopy.m_mHbbNomParamData)
    , m_mHbbNomHistMC(!toCopy.m_mHbbNomHistMC ? NULL : new UncertaintyHistogram(*toCopy.m_mHbbNomHistMC))
    , m_mHbbNomParamMC(toCopy.m_mHbbNomParamMC)
    , m_mTopNomHistData(!toCopy.m_mTopNomHistData ? NULL : new UncertaintyHistogram(*toCopy.m_mTopNomHistData))
    , m_mTopNomParamData(toCopy.m_mTopNomParamData)
    , m_mTopNomHistMC(!toCopy.m_mTopNomHistMC ? NULL : new UncertaintyHistogram(*toCopy.m_mTopNomHistMC))
    , m_mTopNomParamMC(toCopy.m_mTopNomParamMC)
{ }

ResolutionHelper::~ResolutionHelper()
{
    JESUNC_SAFE_DELETE(m_ptNomHistData);
    JESUNC_SAFE_DELETE(m_ptNomHistMC);
    JESUNC_SAFE_DELETE(m_fvNomHistData);
    JESUNC_SAFE_DELETE(m_fvNomHistMC);
    JESUNC_SAFE_DELETE(m_mQCDNomHistData);
    JESUNC_SAFE_DELETE(m_mQCDNomHistMC);
    JESUNC_SAFE_DELETE(m_mWZNomHistData);
    JESUNC_SAFE_DELETE(m_mWZNomHistMC);
    JESUNC_SAFE_DELETE(m_mHbbNomHistData);
    JESUNC_SAFE_DELETE(m_mHbbNomHistMC);
    JESUNC_SAFE_DELETE(m_mTopNomHistData);
    JESUNC_SAFE_DELETE(m_mTopNomHistMC);
}

StatusCode ResolutionHelper::parseInput(TEnv& settings, TFile* histFile, const TString& key, const TString& defaultValue, UncertaintyHistogram* hist, CompParametrization::TypeEnum& param)
{
    // Get the string
    TString value = settings.GetValue(key,defaultValue);

    // Check the trivial case
    if (value == "")
        return StatusCode::SUCCESS;

    // Ensure it matches the expected format
    if (!value.Contains(","))
    {
        ATH_MSG_ERROR("Key of " << key.Data() << " has an unexpected value format (missing comma): " << value.Data());
        return StatusCode::FAILURE;
    }

    // Split the string
    std::vector<TString> splitValue = utils::vectorize<TString>(value,", ");
    if (splitValue.size() < 2)
    {
        ATH_MSG_ERROR("Key of " << key.Data() << " has unexpected value format (less than 2 strings):" << value.Data());
        return StatusCode::FAILURE;
    }
    else if (splitValue.size() > 2)
    {
        ATH_MSG_ERROR("Key of " << key.Data() << " has unexpected value format (more than 2 strings):" << value.Data());
        return StatusCode::FAILURE;
    }
    
    // Ensure that the parametrization is valid
    param = CompParametrization::stringToEnum(splitValue.at(1));
    if (param == CompParametrization::UNKNOWN)
    {
        ATH_MSG_ERROR("Key of " << key.Data() << " has unexpected parametrization value:" << splitValue.at(1));
        return StatusCode::FAILURE;
    }

    // Create the histogram
    hist = new UncertaintyHistogram(splitValue.at(0)+"_"+m_jetDef.c_str(),true);

    // Initialize the histogram
    if (hist->initialize(histFile).isFailure())
        return StatusCode::FAILURE;

    // Print out that we successfully read the input
    ATH_MSG_INFO(Form("  %s: \"%s\"",key.Data(),value.Data()));

    return StatusCode::SUCCESS;
}

StatusCode ResolutionHelper::initialize(TEnv& settings, TFile* histFile)
{
    if (m_isInit)
    {
        ATH_MSG_ERROR("Blocking double-initialization: " << m_name);
        return StatusCode::FAILURE;
    }
    
    // Read nominal resolution information from the configuration file
    // In many cases (especially configs before summer 2018), there are no inputs to find
    // As such, it is perfectly normal behaviour to read zero inputs from the file
    // However, there are some expectations
    //  There should be no cases of data resolution without MC resolution
    //  There can be cases of MC resolution without data resolution
    //  Histograms and their parametrizations must be paired

    // Expected format:
    //      KeyString: HistName,Parametrization
    //  Example:
    //      NominalPtResData: PtResData,PtAbsEta
    //          Histogram name is PtResData_JETDEF
    //          Parametrization is pT vs |eta|


    // Start with nominal pT resolution
    if (parseInput(settings,histFile,"NominalPtResData","",m_ptNomHistData,m_ptNomParamData).isFailure())
        return StatusCode::FAILURE;
    if (parseInput(settings,histFile,"NominalPtResMC","",m_ptNomHistMC,m_ptNomParamMC).isFailure())
        return StatusCode::FAILURE;
    if (m_ptNomHistData && !m_ptNomHistMC)
    {
        ATH_MSG_ERROR("There should never be a nominal data resolution without a nominal MC resolution (pT)");
        return StatusCode::FAILURE;
    }

    // Now the nominal four-vector resolution
    if (parseInput(settings,histFile,"NominalFourVecResData","",m_fvNomHistData,m_fvNomParamData).isFailure())
        return StatusCode::FAILURE;
    if (parseInput(settings,histFile,"NominalFourVecResMC","",m_fvNomHistMC,m_fvNomParamMC).isFailure())
        return StatusCode::FAILURE;
    if (m_fvNomHistData && !m_fvNomHistMC)
    {
        ATH_MSG_ERROR("There should never be a nominal data resolution without a nominal MC resolution (four-vector)");
        return StatusCode::FAILURE;
    }

    // Now the nominal mass resolution (topology-dependent)
    // For simplicity, this is done with separate histograms for each one
    // Lots of code duplication, but time is of the essence, and it works

    // QCD topology
    if (parseInput(settings,histFile,"NominalMassResDataQCD","",m_mQCDNomHistData,m_mQCDNomParamData).isFailure())
        return StatusCode::FAILURE;
    if (parseInput(settings,histFile,"NominalMassResMCQCD","",m_mQCDNomHistMC,m_mQCDNomParamMC).isFailure())
        return StatusCode::FAILURE;
    if (m_mQCDNomHistData && !m_mQCDNomHistMC)
    {
        ATH_MSG_ERROR("There should never be a nominal data resolution without a nominal MC resolution (four-vector)");
        return StatusCode::FAILURE;
    }

    // WZ topology
    if (parseInput(settings,histFile,"NominalMassResDataWZ","",m_mWZNomHistData,m_mWZNomParamData).isFailure())
        return StatusCode::FAILURE;
    if (parseInput(settings,histFile,"NominalMassResMCWZ","",m_mWZNomHistMC,m_mWZNomParamMC).isFailure())
        return StatusCode::FAILURE;
    if (m_mWZNomHistData && !m_mWZNomHistMC)
    {
        ATH_MSG_ERROR("There should never be a nominal data resolution without a nominal MC resolution (four-vector)");
        return StatusCode::FAILURE;
    }

    // Hbb topology
    if (parseInput(settings,histFile,"NominalMassResDataHbb","",m_mHbbNomHistData,m_mHbbNomParamData).isFailure())
        return StatusCode::FAILURE;
    if (parseInput(settings,histFile,"NominalMassResMCHbb","",m_mHbbNomHistMC,m_mHbbNomParamMC).isFailure())
        return StatusCode::FAILURE;
    if (m_mHbbNomHistData && !m_mHbbNomHistMC)
    {
        ATH_MSG_ERROR("There should never be a nominal data resolution without a nominal MC resolution (four-vector)");
        return StatusCode::FAILURE;
    }

    // Top topology
    if (parseInput(settings,histFile,"NominalMassResDataTop","",m_mTopNomHistData,m_mTopNomParamData).isFailure())
        return StatusCode::FAILURE;
    if (parseInput(settings,histFile,"NominalMassResMCTop","",m_mTopNomHistMC,m_mTopNomParamMC).isFailure())
        return StatusCode::FAILURE;
    if (m_mTopNomHistData && !m_mTopNomHistMC)
    {
        ATH_MSG_ERROR("There should never be a nominal data resolution without a nominal MC resolution (four-vector)");
        return StatusCode::FAILURE;
    }

    // Check if the user has specified if this is full correlations or MC-only
    TString smearType = settings.GetValue("ResolutionSmearOnlyMC","");
    if (smearType != "")
    {
        if (!smearType.CompareTo("true",TString::kIgnoreCase))
            m_smearOnlyMC = true;
        else if (!smearType.CompareTo("false",TString::kIgnoreCase))
            m_smearOnlyMC = false;
        else
        {
            ATH_MSG_ERROR("The value of ResolutionSmearOnlyMC doesn't look like the expected boolean: " << smearType.Data());
            return StatusCode::FAILURE;
        }
        ATH_MSG_INFO(Form("  ResolutionSmearOnlyMC: \"%s\"",m_smearOnlyMC ? "true" : "false"));
    }

    // If we are also smearing data, we need the data histograms for any MC histogram
    if (!m_smearOnlyMC)
    {
        if (m_ptNomHistMC && !m_ptNomHistData)
        {
            ATH_MSG_ERROR("Requested full smearing correlations (both data and MC), but only provided the MC nominal histogram for pT");
            return StatusCode::FAILURE;
        }
        if (m_fvNomHistMC && !m_fvNomHistMC)
        {
            ATH_MSG_ERROR("Requested full smearing correlations (both data and MC), but only provided the MC nominal histogram for fourvec");
            return StatusCode::FAILURE;
        }
        if (m_mQCDNomHistMC && !m_mQCDNomHistData)
        {
            ATH_MSG_ERROR("Requested full smearing correlations (both data and MC), but only provided the MC nominal histogram for mQCD");
            return StatusCode::FAILURE;
        }
        if (m_mWZNomHistMC && !m_mWZNomHistData)
        {
            ATH_MSG_ERROR("Requested full smearing correlations (both data and MC), but only provided the MC nominal histogram for mWZ");
            return StatusCode::FAILURE;
        }
        if (m_mHbbNomHistMC && !m_mHbbNomHistData)
        {
            ATH_MSG_ERROR("Requested full smearing correlations (both data and MC), but only provided the MC nominal histogram for mHbb");
            return StatusCode::FAILURE;
        }
        if (m_mTopNomHistMC && !m_mTopNomHistData)
        {
            ATH_MSG_ERROR("Requested full smearing correlations (both data and MC), but only provided the MC nominal histogram for mTop");
            return StatusCode::FAILURE;
        }
    }


    m_isInit = true;
    return StatusCode::SUCCESS;
}

std::pair<const UncertaintyHistogram*,CompParametrization::TypeEnum> ResolutionHelper::getNominalResolution(const CompScaleVar::TypeEnum smearType, const JetTopology::TypeEnum topology, const bool readMC) const
{
    // First get the resolution histogram and parametrization
    const jet::UncertaintyHistogram* resolution = NULL;
    CompParametrization::TypeEnum param         = CompParametrization::UNKNOWN;
    
    if (!m_isInit)
    {
        ATH_MSG_ERROR("Asking for the nominal resolution before initialization");
        return std::pair<UncertaintyHistogram*,CompParametrization::TypeEnum>(NULL,CompParametrization::UNKNOWN);
    }


    switch (smearType)
    {
        case CompScaleVar::MassRes:
        case CompScaleVar::MassResAbs:
            switch (topology)
            {
                case JetTopology::QCD:
                    if (readMC)
                    {
                        resolution = m_mQCDNomHistMC;
                        param      = m_mQCDNomParamMC;
                    }
                    else
                    {
                        resolution = m_mQCDNomHistData;
                        param      = m_mQCDNomParamData;
                    }
                    break;

                case JetTopology::WZ:
                    if (readMC)
                    {
                        resolution = m_mWZNomHistMC;
                        param      = m_mWZNomParamMC;
                    }
                    else
                    {
                        resolution = m_mWZNomHistData;
                        param      = m_mWZNomParamData;
                    }
                    break;

                case JetTopology::Hbb:
                    if (readMC)
                    {
                        resolution = m_mHbbNomHistMC;
                        param      = m_mHbbNomParamMC;
                    }
                    else
                    {
                        resolution = m_mHbbNomHistData;
                        param      = m_mHbbNomParamData;
                    }
                    break;

                case JetTopology::Top:
                    if (readMC)
                    {
                        resolution = m_mTopNomHistMC;
                        param      = m_mTopNomParamMC;
                    }
                    else
                    {
                        resolution = m_mTopNomHistData;
                        param      = m_mTopNomParamData;
                    }
                    break;

                case JetTopology::MIXED:
                    // We shouldn't read this, as it was checked at a higher level
                    // Just to be safe, check it and return error code
                    ATH_MSG_ERROR("Mass resolution depends on a single jet topology, not a mixed topology");
                    return std::pair<UncertaintyHistogram*,CompParametrization::TypeEnum>(NULL,CompParametrization::UNKNOWN);

                default:
                    // We shouldn't read this, as it was checked at a higher level
                    // Just to be safe, check it and return error code
                    ATH_MSG_ERROR("Mass resolution depends on the jet topology, which was not specified");
                    return std::pair<UncertaintyHistogram*,CompParametrization::TypeEnum>(NULL,CompParametrization::UNKNOWN);
            }
            break;

        case CompScaleVar::PtRes:
        case CompScaleVar::PtResAbs:
            if (readMC)
            {
                resolution = m_ptNomHistMC;
                param      = m_ptNomParamMC;
            }
            else
            {
                resolution = m_ptNomHistData;
                param      = m_ptNomParamData;
            }
            break;
        
        case CompScaleVar::FourVecRes:
        case CompScaleVar::FourVecResAbs:
            if (readMC)
            {
                resolution = m_fvNomHistMC;
                param      = m_fvNomParamMC;
            }
            else
            {
                resolution = m_fvNomHistData;
                param      = m_fvNomParamData;
            }
            break;

        default:
            // This is not a resolution uncertainty component
            // We should not get here
            // Print an erorr and return error code
            ATH_MSG_ERROR("Asked for the smearing factor for a non-resolution component");
            return std::pair<UncertaintyHistogram*,CompParametrization::TypeEnum>(NULL,CompParametrization::UNKNOWN);
    }
    return std::pair<const UncertaintyHistogram*,CompParametrization::TypeEnum>(resolution,param);
}

bool ResolutionHelper::hasRelevantInfo(const CompScaleVar::TypeEnum type, const JetTopology::TypeEnum topology) const
{
    if (!m_isInit)
    {
        ATH_MSG_ERROR("Asking for nominal resolution information before initialization");
        return false;
    }

    // Check that the nominal MC histograms for the specified type exist
    //      We have already checked that the parametrizations exist for each histogram
    //      We have already checked that the data histograms exist for each MC if relevant
    switch (type)
    {
        case CompScaleVar::PtRes:
        case CompScaleVar::PtResAbs:
            return m_ptNomHistMC != NULL;

        case CompScaleVar::FourVecRes:
        case CompScaleVar::FourVecResAbs:
            return m_fvNomHistMC != NULL;

        case CompScaleVar::MassRes:
        case CompScaleVar::MassResAbs:
            switch (topology)
            {
                case JetTopology::QCD:
                    return m_mQCDNomHistMC != NULL;

                case JetTopology::WZ:
                    return m_mWZNomHistMC  != NULL;

                case JetTopology::Hbb:
                    return m_mHbbNomHistMC != NULL;

                case JetTopology::Top:
                    return m_mTopNomHistMC != NULL;

                default:
                    ATH_MSG_ERROR("Unexpected topology type, cannot determine if relevant info exists: " << JetTopology::enumToString(topology).Data());
                    return false;
            }

        default:
            ATH_MSG_ERROR("Unexpected variable type, cannot determine if relevant info exists: " << CompScaleVar::enumToString(type).Data());
            return false;
    }
}

} // end jet namespace

