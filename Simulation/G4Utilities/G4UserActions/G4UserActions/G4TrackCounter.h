/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4UserActions_G4TrackCounter_H
#define G4UserActions_G4TrackCounter_H

#include <string>

#include "G4AtlasTools/UserActionBase.h"



class G4TrackCounter final: public UserActionBase {

  private:
   unsigned int ntracks;
   unsigned int ntracks_tot;
   unsigned int ntracks_en;
   unsigned int ntracks_sec;
   unsigned int nevts;
   double avtracks;
   double avtracks_en;
   double avtracks_sec;

  public:
   G4TrackCounter(const std::string& type, const std::string& name, const IInterface* parent):UserActionBase(type,name,parent),
     ntracks(0),ntracks_tot(0),ntracks_en(0),
                                 ntracks_sec(0),nevts(0),avtracks(0),avtracks_en(0),avtracks_sec(0){};

   virtual StatusCode initialize() override;
   virtual StatusCode queryInterface(const InterfaceID&, void**) override;

   virtual void BeginOfEvent(const G4Event*) override;
   virtual void EndOfRun(const G4Run*) override;
   virtual void PreTracking(const G4Track* aTrack) override;

};

#endif


#ifndef G4USERACTIONS__G4UA_G4TRACKCOUNTER_H
#define G4USERACTIONS__G4UA_G4TRACKCOUNTER_H

// Infrastructure includes
#include "G4AtlasInterfaces/IBeginEventAction.h"
#include "G4AtlasInterfaces/IEndRunAction.h"
#include "G4AtlasInterfaces/IPreTrackingAction.h"


namespace G4UA
{

  /// @class G4TrackCounter
  /// @brief A simple action which counts tracks.
  ///
  /// This action currently implements BeginEvent, EndRun, and PreTracking
  /// interfaces. It's not clear if we really want to implement EndRun here.
  /// Presumably we'll need to merge the counter results across threads, so it
  /// might make sense to move some of that functionality in the finalize
  /// method of the corresponding tool.
  ///
  class G4TrackCounter : public IBeginEventAction,
                         public IPreTrackingAction
  {

    public:

      /// @brief Simple struct for holding the counts
      /// Might want to use larger integral types for this...
      struct Report
      {
        /// Event counter. Might want a larger int for this
        unsigned int nEvents=0;
        /// Total number of tracks
        unsigned int nTotalTracks=0;
        /// Number of primary tracks
        unsigned int nPrimaryTracks=0;
        /// Number of secondary tracks
        unsigned int nSecondaryTracks=0;
        /// Number of tracks with kinetic E > 50 MeV
        unsigned int n50MeVTracks=0;

	void merge(const Report& rep);
      };

      /// @brief Increments event counter.
      /// I feel like there must be a better way to get this info.
      /// Hmm, the G4Run has a numberOfEvent field...
      virtual void beginOfEvent(const G4Event* event) override;

      /// Increments the track counters
      virtual void preTracking(const G4Track* track) override;

      /// Retrieve my counts
      const Report& getReport() const
      { return m_report; }

    private:

      /// Track counts for this thread.
      Report m_report;

  }; // class G4TrackCounter

} // namespace G4UA

#endif
