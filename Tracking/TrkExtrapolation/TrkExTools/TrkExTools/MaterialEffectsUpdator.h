/*
   Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
 */

///////////////////////////////////////////////////////////////////
// MaterialEffectsUpdator.h, c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKEXTOOLS_MATERIALEFFECTSUPDATOR_H
#define TRKEXTOOLS_MATERIALEFFECTSUPDATOR_H

// Gaudi
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
// Trk
#include "TrkExInterfaces/IMaterialEffectsUpdator.h"
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkExUtils/MaterialUpdateMode.h"
#include <vector>
#include <string>
#define TRKEXTOOLS_MAXUPDATES 100

#ifndef COVARIANCEUPDATEWITHCHECK
#define COVARIANCEUPDATEWITHCHECK(cov, sign, value) cov += ( sign > 0 ? value : ( value > cov ? 0 : sign*value ) )
#endif


namespace Trk {

class Layer;
//class TrackParameters;
class MaterialProperties;
class IEnergyLossUpdator;
class IMultipleScatteringUpdator;
class IMaterialMapper;

/** @class MaterialEffectsUpdator

  Point-like (also called surface-based) u
  pdate of TrackParameters and associated errors.

  It extends the TrkParametersManipulator, for applying modifications on 
  the track parameters:

  - if the update is done on a pointer to a TrackParameters object,
  the object itself is modified

  - if the update is done on a reference to a TrackParameters object,
  the object is cloned and the the modification is done on the clone

  @author Andreas.Salzburger@cern.ch
  */

class MaterialEffectsUpdator : public AthAlgTool,
  //public TrkParametersManipulator,
  virtual public IMaterialEffectsUpdator {

  public:
    /** AlgTool like constructor */
    MaterialEffectsUpdator(const std::string&,const std::string&,const IInterface*);

    /**Virtual destructor*/
    virtual ~MaterialEffectsUpdator();

    /** AlgTool initailize method.*/
    StatusCode initialize() override;
    /** AlgTool finalize method */
    StatusCode finalize()  override;

    /*
     * The concrete cache class for this specialization of the IMaterialEffectsUpdator
     */ 
    typedef IMaterialEffectsUpdator::ICache ICache;                                          
    class Cache : public ICache{
    public :
      Cache():validationLayer{nullptr},
        validationSteps{0},
        validationPhi{0.},
        validationEta{0.},
        accumulatedElossSigma{0.}{
        }
      const Trk::Layer*           validationLayer{nullptr};     //!< layer in the current validation setp
      int                         validationSteps{0};           //!< number of validation steps
      double                      validationPhi{0.};            //!< theta
      double                      validationEta{0.};            //!< eta
      double                      accumulatedElossSigma{0.};    //!< Sigma of the eloss accumulated so far in the extrapolation. Used in Landau mode
    };

    virtual std::unique_ptr<ICache> getCache() const override {
      return std::make_unique<Cache>();
    }

    /** Updator interface (full update for a layer)
      ---> ALWAYS the same pointer is returned
      the pointer to the same TrackParameters object is returned,
      it is manipulated (if sf.fullUpdateMaterial())
      */ 
    virtual const TrackParameters*  update(ICache& icache, const TrackParameters* parm,
                                           const Layer& sf,
                                           PropDirection dir=alongMomentum,
                                           ParticleHypothesis particle=pion,
                                           MaterialUpdateMode matupmode=addNoise) const override{

      Cache& cache= dynamic_cast<Cache&> (icache);
      const TrackParameters* outparam = updateImpl(cache,parm,sf,dir,particle,matupmode);
      return outparam;
    }
    /** Updator interface (full update for a layer) according to user
      input through MaterialEffectsOnTrack
      ---> ALWAYS the same pointer is returned
      the pointer to the same TrackParameters object is returned,
      it is manipulated
      */ 
    virtual const TrackParameters*  update(ICache& icache, const TrackParameters* parm,
                                           const MaterialEffectsOnTrack& meff,
                                           Trk::ParticleHypothesis particle=pion,
                                           MaterialUpdateMode matupmode=addNoise) const override{

      Cache& cache= dynamic_cast<Cache&> (icache);
      const TrackParameters* outparam = updateImpl(cache,parm,meff,particle,matupmode);
      return outparam;
    }

    /** Updator interface (pre-update for a layer):
      ---> ALWAYS the same pointer is returned 
      the pointer to the same TrackParametes object is returned,
      it is manipulated (if.preUpdateMaterial())
      */ 
    virtual const TrackParameters*   preUpdate(ICache& icache, const TrackParameters* parm,
                                               const Layer& sf,
                                               PropDirection dir=alongMomentum,
                                               ParticleHypothesis particle=pion,
                                               MaterialUpdateMode matupmode=addNoise) const override{

      Cache& cache= dynamic_cast<Cache&> (icache);
      const TrackParameters* outparam = preUpdateImpl(cache,parm,sf,dir,particle,matupmode);
      return outparam;
    }

    /** Updator interface (post-update for a layer): 
      ---> ALWAYS pointer to new TrackParameters are returned
      if no postUpdate is to be done : return 0
      */ 
    virtual const TrackParameters*   postUpdate(ICache& icache,const TrackParameters& parm,
                                                const Layer& sf,
                                                PropDirection dir=alongMomentum,
                                                ParticleHypothesis particle=pion,
                                                MaterialUpdateMode matupmode=addNoise) const override{

      Cache& cache= dynamic_cast<Cache&> (icache);
      const TrackParameters* outparam = postUpdateImpl(cache,parm,sf,dir,particle,matupmode);
      return outparam;
    }
    /** Dedicated Updator interface:-> create new track parameters*/
    virtual const TrackParameters*      update(ICache& icache, const TrackParameters& parm,
                                               const MaterialProperties& mprop,
                                               double pathcorrection,
                                               PropDirection dir=alongMomentum,
                                               ParticleHypothesis particle=pion,
                                               MaterialUpdateMode matupmode=addNoise) const override{

      Cache& cache= dynamic_cast<Cache&> (icache);
      const TrackParameters* outparam = updateImpl(cache,parm,mprop,pathcorrection,dir,particle,matupmode);
      return outparam;
    }

    /** Validation Action - calls the writing and resetting of the TTree variables */
    virtual void validationAction(ICache& icache) const override{

      Cache& cache= dynamic_cast<Cache&> (icache);
      validationActionImpl(cache);
      return ;
    }

    /** Only has an effect if m_landauMode == true.
      Resets mutable variables used for non-local calculation of energy loss if
      parm == 0. Otherwise, modifies parm with the final update of the covariance matrix*/
    virtual void modelAction(ICache& icache, const TrackParameters* parm = 0) const override{

      Cache& cache= dynamic_cast<Cache&> (icache);
      modelActionImpl(cache,parm);
      return;
    }

  private:
    /*
     * Each instance (out of N) needs a have cache in order to accumulate calculation 
     * between calls to its methods. In a Multi Thread environment we will 
     * have pontially M threads running. So we need MxN cache instance.
     *
     * The solution adopted here is an effort to implement 
     * "Schmidt, Douglas & Pryce, Nat & H. Harrison, Timothy. (1998). 
     * Thread-Specific Storage for C/C++ - An Object
     * Behavioral Pattern for Accessing per-Thread State Efficiently."
     * Published in "More C++ Gems (SIGS Reference Library)".
     * The solution in Figure 5 is adopted here. One could also adopt Figure 4 as 
     * an alternative
     *
     * We have  M thread_local ptr to vectors. So one vector for each thread.  
     * For the lookup each object needs a uniqueID. This is created in the initialize 
     * by hashing the unique's instance full name. 
     *
     * Following https://google.github.io/styleguide/cppguide.html#thread_local
     * the thread local is only exposed from inside a function. 
     *
     * Using thread_local unique_ptr<T>  achieves 
     * the same behaviour as boost::thread_specific_ptr<T>
     */

    Cache* getTLSCache() const{ 

      typedef std::vector<std::unique_ptr<Cache>> TLSCollection;
      /* Initialize a unique ptr to default constructed TLSCollection*/
      thread_local std::unique_ptr<TLSCollection> s_cacheStore= std::make_unique<TLSCollection>();        
      
      /*This should never happen*/
      if(s_cacheStore.get()==nullptr){
        return nullptr;
      }
      /* Get the actual cache*/
      size_t index= m_uniqueID-1;
      if (m_uniqueID>s_cacheStore->size()) {
        s_cacheStore->resize(m_uniqueID);
      }
      if (s_cacheStore->operator[](index).get()==nullptr) {
        s_cacheStore->operator[](index)= std::make_unique<Cache>();
      }
      return s_cacheStore->operator[](index).get(); 
    }

  public:
    /* 
     * Public methods using the TLS cache.
     */
    virtual const TrackParameters*  update( const TrackParameters* parm,
                                            const Layer& sf,
                                            PropDirection dir=alongMomentum,
                                            ParticleHypothesis particle=pion,
                                            MaterialUpdateMode matupmode=addNoise) const override{

      Cache& cache = (*getTLSCache());
      const TrackParameters* outparam = updateImpl(cache,parm,sf,dir,particle,matupmode);
      return outparam;
    }

    virtual const TrackParameters*  update(const TrackParameters* parm,
                                           const MaterialEffectsOnTrack& meff,
                                           Trk::ParticleHypothesis particle=pion,
                                           MaterialUpdateMode matupmode=addNoise) const override{
      Cache& cache = (*getTLSCache());
      const TrackParameters* outparam = updateImpl(cache,parm,meff,particle,matupmode);
      return outparam;
    }

    virtual const TrackParameters*   preUpdate( const TrackParameters* parm,
                                                const Layer& sf,
                                                PropDirection dir=alongMomentum,
                                                ParticleHypothesis particle=pion,
                                                MaterialUpdateMode matupmode=addNoise) const override{
      Cache& cache = (*getTLSCache());
      const TrackParameters* outparam = preUpdateImpl(cache,parm,sf,dir,particle,matupmode);
      return outparam;
    }

    virtual const TrackParameters*   postUpdate(const TrackParameters& parm,
                                                const Layer& sf,
                                                PropDirection dir=alongMomentum,
                                                ParticleHypothesis particle=pion,
                                                MaterialUpdateMode matupmode=addNoise) const override{
      Cache& cache = (*getTLSCache());
      const TrackParameters* outparam = postUpdateImpl(cache,parm,sf,dir,particle,matupmode);
      return outparam;
    }

    virtual const TrackParameters*  update(const TrackParameters& parm,
                                           const MaterialProperties& mprop,
                                           double pathcorrection,
                                           PropDirection dir=alongMomentum,
                                           ParticleHypothesis particle=pion,
                                           MaterialUpdateMode matupmode=addNoise) const override{
      Cache& cache = (*getTLSCache());
      const TrackParameters* outparam = updateImpl(cache,parm,mprop,pathcorrection,dir,particle,matupmode);
      return outparam;
    }

    virtual void validationAction() const override{
      Cache& cache = (*getTLSCache());
      validationActionImpl(cache);

      return;
    }

    virtual void modelAction(const TrackParameters* parm = 0) const override{
      Cache& cache = (*getTLSCache());
      modelActionImpl(cache,parm);  
      return;
    }

  private:
    /* The acutal implementation methods using the tool's 
     * concrete  Cache*/ 
    const TrackParameters*  updateImpl(Cache& cache, const TrackParameters* parm,
                                       const Layer& sf,
                                       PropDirection dir=alongMomentum,
                                       ParticleHypothesis particle=pion,
                                       MaterialUpdateMode matupmode=addNoise) const ;

    const TrackParameters*  updateImpl(Cache& cache, const TrackParameters* parm,
                                       const MaterialEffectsOnTrack& meff,
                                       Trk::ParticleHypothesis particle=pion,
                                       MaterialUpdateMode matupmode=addNoise) const ;

    const TrackParameters*   preUpdateImpl(Cache& cache, const TrackParameters* parm,
                                           const Layer& sf,
                                           PropDirection dir=alongMomentum,
                                           ParticleHypothesis particle=pion,
                                           MaterialUpdateMode matupmode=addNoise) const ;

    const TrackParameters*   postUpdateImpl(Cache& cache,const TrackParameters& parm,
                                            const Layer& sf,
                                            PropDirection dir=alongMomentum,
                                            ParticleHypothesis particle=pion,
                                            MaterialUpdateMode matupmode=addNoise) const ;

    const TrackParameters*   updateImpl(Cache& cache, const TrackParameters* parm,
                                        const MaterialProperties& mprop,
                                        double pathcorrection,
                                        PropDirection dir=alongMomentum,
                                        ParticleHypothesis particle=pion,
                                        MaterialUpdateMode matupmode=addNoise) const ;

    const TrackParameters*   updateImpl(Cache& cache, const TrackParameters& parm,
                                        const MaterialProperties& mprop,
                                        double pathcorrection,
                                        PropDirection dir=alongMomentum,
                                        ParticleHypothesis particle=pion,
                                        MaterialUpdateMode matupmode=addNoise) const ;


    void validationActionImpl(Cache& cache) const;

    void modelActionImpl(Cache& cache, const TrackParameters* parm = 0) const;

    /** A simple check method for the 'removeNoise' update model */
    bool checkCovariance(AmgSymMatrix(5)& updated) const ;
    const TrackParameters* finalLandauCovarianceUpdate (const TrackParameters* parm) const;

    /* Private Class members*/
    bool                         m_doCompoundLayerCheck;  //!< turn on/off the necessary checks when we may have compound layers
    bool                         m_doEloss;               //!< steer energy loss On/Off from outside
    bool                         m_doMs;                  //!< steer multiple scattering On/Off from outside

    bool                         m_forceMomentum;         //!< Force the momentum to be a specific value
    bool                         m_xKalmanStraggling;     //!< the momentum Error as calculated in xKalman
    bool                         m_useMostProbableEloss;  //!< use the most probable energy loss

    bool                         m_msgOutputValidationDirection; //!< validation direction used for screen output
    bool                         m_msgOutputCorrections;          //!< screen output of actual corrections

    // ------------ validation variables -------------------------------------------------
    bool                         m_validationMode;                     //!< Switch for validation mode
    bool                         m_validationIgnoreUnmeasured;         //!< Ignore unmeasured TrackParameters (Navigation!)
    bool                         m_landauMode;                         //!< If in Landau mode, error propagation is done as for landaus 
    int                          m_validationDirection;                //!< validation direction
    //  ------------------------------
    std::size_t                  m_uniqueID;              //!< UniqueID Hash based on the unique instance name
    double                       m_momentumCut;           //!< Minimal momentum cut for update
    double                       m_momentumMax;           //!< Maximal momentum cut for update
    double                       m_forcedMomentum;        //!< Forced momentum value

    ToolHandle< IEnergyLossUpdator >  m_eLossUpdator;     //!< AlgoTool for EnergyLoss updates
    ToolHandle< IMultipleScatteringUpdator > m_msUpdator; //!< AlgoTool for MultipleScatterin effects
    // the material mapper for the validation process
    ToolHandle< IMaterialMapper > m_materialMapper;            //!< the material mapper for recording the layer material 

  };
} // end of namespace


#endif // TRKEXTOOLS_MATERIALEFFECTSUPDATOR_H

