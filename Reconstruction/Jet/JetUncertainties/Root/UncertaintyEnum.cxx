/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


#include "JetUncertainties/UncertaintyEnum.h"

#include <math.h>

namespace jet
{

//////////////////////////////////////////////////
//                                              //
//  Component category enumeration methods      //
//                                              //
//////////////////////////////////////////////////

namespace CompCategory
{
    TString enumToString(const TypeEnum type)
    {
        switch (type)
        {
            case Statistical:   return "Statistical";
            case Detector:      return "Detector";
            case Modelling:     return "Modelling";
            case Mixed:         return "Mixed";
            case Effective:     return "Effective";
            case Other:         return "Other";
            default:            return "UNKNOWN";
        }
    }
    
    TypeEnum stringToEnum(const TString type)
    {
        if (!type.CompareTo("Statistical"))
            return Statistical;
        if (!type.CompareTo("Detector"))
            return Detector;
        if (!type.CompareTo("Modelling"))
            return Modelling;
        if (!type.CompareTo("Mixed"))
            return Mixed;
        if (!type.CompareTo("Effective"))
            return Effective;
        if (!type.CompareTo("Other"))
            return Other;
        return UNKNOWN;
    }
} // end CompCategory namespace


//////////////////////////////////////////////////
//                                              //
//  Component correlation enumeration methods   //
//                                              //
//////////////////////////////////////////////////

namespace CompCorrelation
{
    TString enumToString(const TypeEnum type)
    {
        switch (type)
        {
            case Uncorrelated:  return "Uncorrelated";
            case Correlated:    return "Correlated";
            default:            return "UNKNOWN";
        }
    }

    TypeEnum stringToEnum(const TString type)
    {
        if (!type.CompareTo("Uncorrelated"))
            return Uncorrelated;
        if (!type.CompareTo("Correlated"))
            return Correlated;
        return UNKNOWN;
    }
} // end CompCorrelation namespace


//////////////////////////////////////////////////
//                                              //
//  Component parametrization type enum methods //
//                                              //
//////////////////////////////////////////////////

namespace CompParametrization
{
    TString enumToString(const TypeEnum type)
    {
        switch(type)
        {
            case Pt:            return "Pt";
            case PtEta:         return "PtEta";
            case PtAbsEta:      return "PtAbsEta";
            case PtMass:        return "PtMass";
            case PtMassEta:     return "PtMassEta";
            case PtMassAbsEta:  return "PtMassAbsEta";
            case eLOGmOe:       return "eLOGmOe";
            case eLOGmOeEta:    return "eLOGmOeEta";
            case eLOGmOeAbsEta: return "eLOGmOeAbsEta";
            default:            return "UNKNOWN";
        }
    }

    TypeEnum stringToEnum(const TString type)
    {
        if (!type.CompareTo("Pt",TString::kIgnoreCase))
            return Pt;
        if (!type.CompareTo("PtEta",TString::kIgnoreCase))
            return PtEta;
        if (!type.CompareTo("PtAbsEta",TString::kIgnoreCase))
            return PtAbsEta;
        if (!type.CompareTo("PtMass",TString::kIgnoreCase))
            return PtMass;
        if (!type.CompareTo("PtMassEta",TString::kIgnoreCase))
            return PtMassEta;
        if (!type.CompareTo("PtMassAbsEta",TString::kIgnoreCase))
            return PtMassAbsEta;
        if (!type.CompareTo("eLOGmOe",TString::kIgnoreCase))
            return eLOGmOe;
        if (!type.CompareTo("eLOGmOeEta",TString::kIgnoreCase))
            return eLOGmOeEta;
        if (!type.CompareTo("eLOGmOeAbsEta",TString::kIgnoreCase))
            return eLOGmOeAbsEta;
        return UNKNOWN;
    }

    bool isAbsEta(const TypeEnum type)
    {
        switch(type)
        {
            case PtAbsEta:
            case PtMassAbsEta:  
            case eLOGmOeAbsEta: return true;
            
            default:            return false;
        }
    }
} // end CompParametrization namespace


//////////////////////////////////////////////////
//                                              //
//  Component parametrization type enum methods //
//                                              //
//////////////////////////////////////////////////

namespace CompMassDef
{
    TString enumToString(const TypeEnum type)
    {
        switch(type)
        {
            case CaloMass:      return "Calo";
            case TAMass:        return "TA";
            case CombMassQCD:   return "CombQCD";
            case CombMassWZ:    return "CombWZ";
            case CombMassHbb:   return "CombHbb";
            case CombMassTop:   return "CombTop";
            default:            return "UNKNOWN";
        }
    }

    TypeEnum stringToEnum(const TString type)
    {
        if (!type.CompareTo("Calo",TString::kIgnoreCase) || !type.CompareTo("Calorimeter",TString::kIgnoreCase))
            return CaloMass;
        if (!type.CompareTo("TA",TString::kIgnoreCase) || !type.CompareTo("TrackAssisted",TString::kIgnoreCase))
            return TAMass;
        if (type.BeginsWith("Comb",TString::kIgnoreCase) || type.BeginsWith("Combined",TString::kIgnoreCase))
        {
            if (type.EndsWith("QCD",TString::kIgnoreCase))
                return CombMassQCD;
            if (type.EndsWith("WZ",TString::kIgnoreCase))
                return CombMassWZ;
            if (type.EndsWith("Hbb",TString::kIgnoreCase))
                return CombMassHbb;
            if (type.EndsWith("Top",TString::kIgnoreCase))
                return CombMassTop;
        }
        return UNKNOWN;
    }

    TString getJetScaleString(const TypeEnum type)
    {
        switch(type)
        {
            case CaloMass:      return "JetJMSScaleMomentumCalo";
            case TAMass:        return "JetJMSScaleMomentumTA";
            case CombMassQCD:   return "JetJMSScaleMomentumCombQCD";
            case CombMassWZ:    return "JetJMSScaleMomentumCombWZ";
            case CombMassHbb:   return "JetJMSScaleMomentumCombHbb";
            case CombMassTop:   return "JetJMSScaleMomentumCombTop";
            default:            return "UNKNOWN_JET_MASS_SCALE";

        }
    }

} // end CompMassDef namespace

//////////////////////////////////////////////////
//                                              //
//  Component parametrization type enum methods //
//                                              //
//////////////////////////////////////////////////

namespace CompScaleVar
{
    TString enumToString(const TypeEnum type)
    {
        switch(type)
        {
            case FourVec:       return "FourVec";
            case Pt:            return "Pt";
            case Mass:          return "Mass";
            case D12:           return "Split12";
            case D23:           return "Split23";
            case Tau21:         return "Tau21";
            case Tau32:         return "Tau32";
            case Tau21WTA:      return "Tau21WTA";
            case Tau32WTA:      return "Tau32WTA";
            case D2Beta1:       return "D2Beta1";
            case C2Beta1:       return "C2Beta1";
            case Qw:            return "Qw";
            case MassRes:       return "MassRes";
            case MassResAbs:    return "MassResAbs";
            case PtRes:         return "PtRes";
            case PtResAbs:      return "PtResAbs";
            case FourVecRes:    return "FourVecRes";
            case FourVecResAbs: return "FourVecResAbs";
            default:            return "UNKNOWN";
        }
    }

    TypeEnum stringToEnum(const TString type)
    {
        if (!type.CompareTo("FourVec",TString::kIgnoreCase) || !type.CompareTo("4Vec",TString::kIgnoreCase) || !type.CompareTo("FourVector",TString::kIgnoreCase))
            return FourVec;
        if (!type.CompareTo("Pt",TString::kIgnoreCase))
            return Pt;
        if (!type.CompareTo("Mass",TString::kIgnoreCase))
            return Mass;
        if (!type.CompareTo("D12",TString::kIgnoreCase) || !type.CompareTo("Split12",TString::kIgnoreCase))
            return D12;
        if (!type.CompareTo("D23",TString::kIgnoreCase) || !type.CompareTo("Split23",TString::kIgnoreCase))
            return D23;
        if (!type.CompareTo("Tau21",TString::kIgnoreCase))
            return Tau21;
        if (!type.CompareTo("Tau32",TString::kIgnoreCase))
            return Tau32;
        if (!type.CompareTo("Tau21WTA",TString::kIgnoreCase))
            return Tau21WTA;
        if (!type.CompareTo("Tau32WTA",TString::kIgnoreCase))
            return Tau32WTA;
        if (!type.CompareTo("D2Beta1",TString::kIgnoreCase))
            return D2Beta1;
        if (!type.CompareTo("C2Beta1",TString::kIgnoreCase))
            return C2Beta1;
        if (!type.CompareTo("Qw",TString::kIgnoreCase))
            return Qw;
        if (!type.CompareTo("MassRes",TString::kIgnoreCase))
            return MassRes;
        if (!type.CompareTo("MassResAbs",TString::kIgnoreCase))
            return MassResAbs;
        if (!type.CompareTo("PtRes",TString::kIgnoreCase))
            return PtRes;
        if (!type.CompareTo("PtResAbs",TString::kIgnoreCase))
            return PtResAbs;
        if (!type.CompareTo("FourVecRes",TString::kIgnoreCase))
            return FourVecRes;
        if (!type.CompareTo("FourVecResAbs",TString::kIgnoreCase))
            return FourVecResAbs;
        return UNKNOWN;
    }

    bool isScaleType(const TypeEnum type)
    {
        switch (type)
        {
            case FourVec:
            case Pt:
            case Mass:
            case D12:
            case D23:
            case Tau21:
            case Tau32:
            case Tau21WTA:
            case Tau32WTA:
            case D2Beta1:
            case C2Beta1:
            case Qw:
                return true;

            default:
                return false;
        }
    }

    bool isResolutionType(const TypeEnum type)
    {
        switch (type)
        {
            case MassRes:
            case MassResAbs:
            case PtRes:
            case PtResAbs:
            case FourVecRes:
            case FourVecResAbs:
                return true;

            default:
                return false;
        }
    }

    bool isAbsResolutionType(const TypeEnum type)
    {
        switch (type)
        {
            case MassResAbs:
            case PtResAbs:
            case FourVecResAbs:
                return true;

            default:
                return false;
        }
    }

    bool isRelResolutionType(const TypeEnum type)
    {
        switch (type)
        {
            case MassRes:
            case PtRes:
            case FourVecRes:
                return true;

            default:
                return false;
        }
    }
}


//////////////////////////////////////////////////
//                                              //
//  Pileup component enumeration methods        //
//                                              //
//////////////////////////////////////////////////

namespace PileupComp
{
    TString enumToString(const TypeEnum type)
    {
        switch (type)
        {
            case OffsetNPV:     return "Pileup_OffsetNPV";
            case OffsetMu:      return "Pileup_OffsetMu";
            case PtTerm:        return "Pileup_PtTerm";
            case RhoTopology:   return "Pileup_RhoTopology";
            default:            return "UNKNOWN";
        }
    }

    TypeEnum stringToEnum(const TString type)
    {
        if (type.Contains("Pileup",TString::kIgnoreCase))
        {
            if (type.Contains("OffsetNPV",TString::kIgnoreCase))
                return OffsetNPV;
            if (type.Contains("OffsetMu",TString::kIgnoreCase))
                return OffsetMu;
            if (type.Contains("PtTerm",TString::kIgnoreCase))
                return PtTerm;
            if (type.Contains("RhoTopology",TString::kIgnoreCase))
                return RhoTopology;
        }
        return UNKNOWN;
    }
}


//////////////////////////////////////////////////
//                                              //
//  Flavour component enumeration methods       //
//                                              //
//////////////////////////////////////////////////

namespace FlavourComp
{
    TString enumToString(const TypeEnum type)
    {
        switch (type)
        {
            case Response:          return "FlavourResponse";
            case Composition:       return "FlavourComposition";
            case bJES:              return "bJES";
            case PerJetResponse:    return "PerJetFlavourResponse";
            default:                return "UNKNOWN";
        }
    }

    TypeEnum stringToEnum(const TString type)
    {
        if (type.Contains("Flavour",TString::kIgnoreCase) || type.Contains("Flavor",TString::kIgnoreCase))
        {
            if (type.Contains("Response",TString::kIgnoreCase))
            {
                if (type.Contains("PerJet",TString::kIgnoreCase))
                    return PerJetResponse;
                else
                    return Response;
            }
            if (type.Contains("Composition",TString::kIgnoreCase))
                return Composition;
        }
        if (type.Contains("bJES",TString::kIgnoreCase))
            return bJES;
        return UNKNOWN;
    }
}


//////////////////////////////////////////////////
//                                              //
//  Combined mass component enumeration methods //
//                                              //
//////////////////////////////////////////////////

namespace CombMassComp
{
    TString enumToString(const TypeEnum type)
    {
        switch(type)
        {
            case Calo:      return "Calo";
            case TA:        return "TA";
            case Both:      return "Both";
            default:        return "UNKNOWN";
        }
    }

    TypeEnum stringToEnum(const TString type)
    {
        if (type.Contains("Calo",TString::kIgnoreCase))
            return Calo;
        if (type.Contains("TA",TString::kIgnoreCase))
            return TA;
        if (type.Contains("Both",TString::kIgnoreCase))
            return Both;
        return UNKNOWN;
    }
}


//////////////////////////////////////////////////
//                                              //
//  Jet topology enumeration methods            //
//                                              //
//////////////////////////////////////////////////

namespace JetTopology
{
    TString enumToString(const TypeEnum type)
    {
        switch(type)
        {
            case QCD:   return "QCD";
            case WZ:    return "WZ";
            case Hbb:   return "Hbb";
            case Top:   return "Top";
            case MIXED: return "MIXED";
            default:    return "UNKNOWN";
        }
    }

    TypeEnum stringToEnum(const TString type)
    {
        if (!type.CompareTo("QCD",TString::kIgnoreCase))
            return QCD;
        if (!type.CompareTo("WZ",TString::kIgnoreCase))
            return WZ;
        if (!type.CompareTo("Hbb",TString::kIgnoreCase))
            return Hbb;
        if (!type.CompareTo("Top",TString::kIgnoreCase))
            return Top;
        return UNKNOWN;
    }
}

} // end jet namespace

