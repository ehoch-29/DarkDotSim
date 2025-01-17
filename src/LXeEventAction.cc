//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file optical/LXe/src/LXeEventAction.cc
/// \brief Implementation of the LXeEventAction class
//
//
#include "LXeEventAction.hh"

#include "LXeDetectorConstruction.hh"
#include "LXeHistoManager.hh"
#include "LXePMTHit.hh"
#include "LXeQDHit.hh"
#include "LXeRun.hh"
#include "LXeScintHit.hh"
#include "LXeTrajectory.hh"

#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4ios.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4Trajectory.hh"
#include "G4TrajectoryContainer.hh"
#include "G4UImanager.hh"
#include "G4VVisManager.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LXeEventAction::LXeEventAction(const LXeDetectorConstruction* det)
  : fDetector(det)
  , fScintCollID(-1)
  , fPMTCollID(-1)
  , fVerbose(0)
  , fPMTThreshold(1)
  , fForcedrawphotons(false)
  , fForcenophotons(false)
{
  fEventMessenger = new LXeEventMessenger(this);

  fHitCount                = 0;
  fPhotonCount_Scint       = 0;
  fPhotonCount_Ceren       = 0;
  fAbsorptionCount         = 0;
  fBoundaryAbsorptionCount = 0;
  fTotE                    = 0.0;
  fTotPMT                  = 0.0;
  fConvPosSet = false;
  fEdepMax    = 0.0;

  fPMTsAboveThreshold = 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LXeEventAction::~LXeEventAction() { delete fEventMessenger; }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeEventAction::BeginOfEventAction(const G4Event*)
{
  fHitCount                = 0;
  fPhotonCount_Scint       = 0;
  fPhotonCount_Ceren       = 0;
  fAbsorptionCount         = 0;
  fBoundaryAbsorptionCount = 0;
  fTotE                    = 0.0;
  fTotPMT                  = 0.0;
  fTotQD                   = 0.0;
  fConvPosSet = false;
  fEdepMax    = 0.0;

  fPMTsAboveThreshold = 0;

  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  if(fScintCollID < 0)
    fScintCollID = SDman->GetCollectionID("scintCollection");
  if(fPMTCollID < 0)
    fPMTCollID = SDman->GetCollectionID("pmtHitCollection");
  if(fQDCollID < 0)
    fQDCollID = SDman->GetCollectionID("qdHitCollection");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeEventAction::EndOfEventAction(const G4Event* anEvent)
{
  G4TrajectoryContainer* trajectoryContainer =
    anEvent->GetTrajectoryContainer();

  G4int n_trajectories = 0;
  if(trajectoryContainer)
    n_trajectories = trajectoryContainer->entries();

  // extract the trajectories and draw them
  if(G4VVisManager::GetConcreteInstance())
  {
    for(G4int i = 0; i < n_trajectories; ++i)
    {
      LXeTrajectory* trj =
        (LXeTrajectory*) ((*(anEvent->GetTrajectoryContainer()))[i]);
      if(trj->GetParticleName() == "opticalphoton")
      {
        trj->SetForceDrawTrajectory(true);
	//trj->SetForceDrawTrajectory(fForcedrawphotons);
        trj->SetForceNoDrawTrajectory(false);
	//trj->SetForceNoDrawTrajectory(fForcenophotons);
      }
      trj->DrawTrajectory();
    }
  }

  LXeScintHitsCollection* scintHC = nullptr;
  LXePMTHitsCollection* pmtHC     = nullptr;
  LXeQDHitsCollection* qdHC       = nullptr;
  G4HCofThisEvent* hitsCE         = anEvent->GetHCofThisEvent();

  // Get the hit collections
  if(hitsCE)
  {
    if(fScintCollID >= 0)
    {
      scintHC = (LXeScintHitsCollection*) (hitsCE->GetHC(fScintCollID));
    }
    if(fPMTCollID >= 0)
    {
      pmtHC = (LXePMTHitsCollection*) (hitsCE->GetHC(fPMTCollID));
    }
    if(fQDCollID >= 0)
    {
      qdHC = (LXeQDHitsCollection*) (hitsCE->GetHC(fQDCollID));
    }
    
  }
  // Hits in scintillator
  if(scintHC)
  {
    size_t n_hit = scintHC->entries();
    G4cout << "SCINT" << n_hit << G4endl;
    G4ThreeVector eWeightPos(0.);
    G4double edep;
    G4double edepMax = 0;

    for(size_t i = 0; i < n_hit; ++i)
    {  // gather info on hits in scintillator
      edep = (*scintHC)[i]->GetEdep();

      if (edep != 0)
	{
	  G4AnalysisManager::Instance()->FillNtupleDColumn(0, 3, 1);
	}
    
      fTotE += edep;
      eWeightPos +=
        (*scintHC)[i]->GetPos() * edep;  // calculate energy weighted pos
      if(edep > edepMax)
      {
        edepMax = edep;  // store max energy deposit
        G4ThreeVector posMax = (*scintHC)[i]->GetPos();
        fPosMax              = posMax;
        fEdepMax             = edep;
      }
    }
    G4AnalysisManager::Instance()->FillNtupleDColumn(0, 1, fTotE);
    G4AnalysisManager::Instance()->FillH1(7, fTotE);

    if(fTotE == 0.)
    {
      if(fVerbose > 0)
        G4cout << "No hits in the scintillator this event." << G4endl;
    }
    else
    {
      // Finish calculation of energy weighted position
      eWeightPos /= fTotE;
      fEWeightPos = eWeightPos;
      if(fVerbose > 0)
      {
        G4cout << "\tEnergy weighted position of hits in LXe : "
               << eWeightPos / mm << G4endl;
      }
    }
    if(fVerbose > 0)
    {
      G4cout << "\tTotal energy deposition in scintillator : " << fTotE / keV
             << " (keV)" << G4endl;
    }
  }

  if(pmtHC)
  {
    G4ThreeVector reconPos(0., 0., 0.);
    size_t pmts = pmtHC->entries();

    G4double edep;
    G4double hit;
    // Gather info from all PMTs
    for(size_t i = 0; i < pmts; ++i)
    {
      edep = (*pmtHC)[i]->GetEdep();
      hit = (*pmtHC)[i]->GetPhotonCount();
      if (hit == 1.)
	{
	  G4AnalysisManager::Instance()->FillNtupleDColumn(0, 2, hit);
	  fTotPMT += edep;
	  G4AnalysisManager::Instance()->FillNtupleDColumn(1, 0, edep);
	  G4AnalysisManager::Instance()->AddNtupleRow(1);

	}
      fHitCount += (*pmtHC)[i]->GetPhotonCount();
      reconPos += (*pmtHC)[i]->GetPMTPos() * (*pmtHC)[i]->GetPhotonCount();
      if((*pmtHC)[i]->GetPhotonCount() >= fPMTThreshold)
      {
        ++fPMTsAboveThreshold;
      }
      else
      {  // wasn't above the threshold, turn it back off
        (*pmtHC)[i]->SetDrawit(true);
      }
    }


    G4AnalysisManager::Instance()->FillH1(9, fTotPMT);
    G4AnalysisManager::Instance()->FillH1(1, fHitCount);
    G4AnalysisManager::Instance()->FillH1(2, fPMTsAboveThreshold);
    G4AnalysisManager::Instance()->AddNtupleRow(0);
    if(fHitCount > 0)
    {  // don't bother unless there were hits
      reconPos /= fHitCount;
      if(fVerbose > 0)
      {
        G4cout << "\tReconstructed position of hits in LXe : " << reconPos / mm
               << G4endl;
      }
      fReconPos = reconPos;
    }
    pmtHC->DrawAllHits();
  }

  if(qdHC)
  {
    G4ThreeVector qd_reconPos(0., 0., 0.);
    size_t qds = qdHC->entries();
    G4cout << qds << G4endl;
    G4double edep;
    G4double qd_hit;
    // Gather info from all QDs
    for(size_t i = 0; i < qds; ++i)
    {
      edep = (*qdHC)[i]->GetEdep();
      qd_hit = (*qdHC)[i]->GetPhotonCount();
      G4cout << edep << G4endl;
      qd_fHitCount += (*qdHC)[i]->GetPhotonCount();
      qd_reconPos += (*qdHC)[i]->GetQDPos() * (*qdHC)[i]->GetPhotonCount();
      fTotQD += edep;
    }
    
    G4AnalysisManager::Instance()->FillH1(10, fTotQD);
  }
  // update the run statistics
  LXeRun* run = static_cast<LXeRun*>(
    G4RunManager::GetRunManager()->GetNonConstCurrentRun());

  run->IncHitCount(fHitCount);
  run->IncPhotonCount_Scint(fPhotonCount_Scint);
  run->IncPhotonCount_Ceren(fPhotonCount_Ceren);
  run->IncEDep(fTotE);
  run->IncAbsorption(fAbsorptionCount);
  run->IncBoundaryAbsorption(fBoundaryAbsorptionCount);
  run->IncHitsAboveThreshold(fPMTsAboveThreshold);

  // If we have set the flag to save 'special' events, save here
  if(fPhotonCount_Scint + fPhotonCount_Ceren < fDetector->GetSaveThreshold())
  {
    G4RunManager::GetRunManager()->rndmSaveThisEvent();
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
