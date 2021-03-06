/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


#include "CLHEP/GenericFunctions/ACos.hh"
#include "CLHEP/GenericFunctions/ASin.hh"
#include "CLHEP/GenericFunctions/ATan.hh"
#include "CLHEP/GenericFunctions/Abs.hh"
#include "CLHEP/GenericFunctions/AbsFunction.hh"
#include "CLHEP/GenericFunctions/AbsFunctional.hh"
#include "CLHEP/GenericFunctions/AbsParameter.hh"
#include "CLHEP/GenericFunctions/AnalyticConvolution.hh"
#include "CLHEP/GenericFunctions/Argument.hh"
#include "CLHEP/GenericFunctions/ArrayFunction.hh"
#include "CLHEP/GenericFunctions/AssociatedLaguerre.hh"
#include "CLHEP/GenericFunctions/BivariateGaussian.hh"
#include "CLHEP/GenericFunctions/ConstMinusFunction.hh"
#include "CLHEP/GenericFunctions/ConstMinusParameter.hh"
#include "CLHEP/GenericFunctions/ConstOverFunction.hh"
#include "CLHEP/GenericFunctions/ConstOverParameter.hh"
#include "CLHEP/GenericFunctions/ConstPlusFunction.hh"
#include "CLHEP/GenericFunctions/ConstPlusParameter.hh"
#include "CLHEP/GenericFunctions/ConstTimesFunction.hh"
#include "CLHEP/GenericFunctions/ConstTimesParameter.hh"
#include "CLHEP/GenericFunctions/Cos.hh"
#include "CLHEP/GenericFunctions/CumulativeChiSquare.hh"
#include "CLHEP/GenericFunctions/DefiniteIntegral.hh"
#include "CLHEP/GenericFunctions/Erf.hh"
#include "CLHEP/GenericFunctions/Exp.hh"
#include "CLHEP/GenericFunctions/Exponential.hh"
#include "CLHEP/GenericFunctions/FixedConstant.hh"
#include "CLHEP/GenericFunctions/FloatingConstant.hh"
#include "CLHEP/GenericFunctions/FunctionComposition.hh"
#include "CLHEP/GenericFunctions/FunctionConvolution.hh"
#include "CLHEP/GenericFunctions/FunctionDifference.hh"
#include "CLHEP/GenericFunctions/FunctionDirectProduct.hh"
#include "CLHEP/GenericFunctions/FunctionNegation.hh"
#include "CLHEP/GenericFunctions/FunctionNoop.hh"
#include "CLHEP/GenericFunctions/FunctionNumDeriv.hh"
#include "CLHEP/GenericFunctions/FunctionPlusParameter.hh"
#include "CLHEP/GenericFunctions/FunctionProduct.hh"
#include "CLHEP/GenericFunctions/FunctionQuotient.hh"
#include "CLHEP/GenericFunctions/FunctionSum.hh"
#include "CLHEP/GenericFunctions/FunctionTimesParameter.hh"
#include "CLHEP/GenericFunctions/Gaussian.hh"
#include "CLHEP/GenericFunctions/IncompleteGamma.hh"
#include "CLHEP/GenericFunctions/Landau.hh"
#include "CLHEP/GenericFunctions/LikelihoodFunctional.hh"
#include "CLHEP/GenericFunctions/Ln.hh"
#include "CLHEP/GenericFunctions/LogGamma.hh"
#include "CLHEP/GenericFunctions/LogisticFunction.hh"
#include "CLHEP/GenericFunctions/Mod.hh"
#include "CLHEP/GenericFunctions/Parameter.hh"
#include "CLHEP/GenericFunctions/ParameterDifference.hh"
#include "CLHEP/GenericFunctions/ParameterNegation.hh"
#include "CLHEP/GenericFunctions/ParameterProduct.hh"
#include "CLHEP/GenericFunctions/ParameterQuotient.hh"
#include "CLHEP/GenericFunctions/ParameterSum.hh"
#include "CLHEP/GenericFunctions/PeriodicRectangular.hh"
#include "CLHEP/GenericFunctions/Power.hh"
#include "CLHEP/GenericFunctions/Psi2Hydrogen.hh"
#include "CLHEP/GenericFunctions/PtRelFcn.hh"
#include "CLHEP/GenericFunctions/RCBase.hh"
#include "CLHEP/GenericFunctions/RKIntegrator.hh"
#include "CLHEP/GenericFunctions/Rectangular.hh"
#include "CLHEP/GenericFunctions/ReverseExponential.hh"
#include "CLHEP/GenericFunctions/Sigma.hh"
#include "CLHEP/GenericFunctions/Sin.hh"
#include "CLHEP/GenericFunctions/Sqrt.hh"
#include "CLHEP/GenericFunctions/Square.hh"
#include "CLHEP/GenericFunctions/Tan.hh"
#include "CLHEP/GenericFunctions/TrivariateGaussian.hh"
#include "CLHEP/GenericFunctions/Variable.hh"
#include "CLHEP/GenericFunctions/X.hh"

/* excluded because of multipe defined symbols
   problem with *.icc files defining non inline functions
#include "CLHEP/GenericFunctions/Bessel.hh"
#include "CLHEP/GenericFunctions/EllipticIntegral.hh"
#include "CLHEP/GenericFunctions/SphericalBessel.hh"
#include "CLHEP/GenericFunctions/SphericalNeumann.hh"
*/

// not implemented functions in clhep 1.8.1.0
inline Genfun::Parameter& Genfun::LogGamma::sigma() { return *(new Parameter("dummy",1.0)); };
