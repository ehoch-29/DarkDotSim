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
/// \file optical/LXe/src/LXePMTSD.cc
/// \brief Implementation of the LXePMTSD class
//
//
#include "LXeQDSD.hh"

#include "LXeDetectorConstruction.hh"
#include "LXeQDHit.hh"
#include "LXeUserTrackInformation.hh"

#include "G4ios.hh"
#include "G4LogicalVolume.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4TouchableHistory.hh"
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VTouchable.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LXeQDSD::LXeQDSD(G4String name)
  : G4VSensitiveDetector(name)

  , fQDPositionsX(nullptr)
  , fQDPositionsY(nullptr)
  , fQDPositionsZ(nullptr)
  , fHitCID(-1)
{
  fQDHitCollection = nullptr;
  collectionName.insert("qdHitCollection");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LXeQDSD::~LXeQDSD()
{
  delete fQDPositionsX;
  delete fQDPositionsY;
  delete fQDPositionsZ;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeQDSD::Initialize(G4HCofThisEvent* hitsCE)
{
  fQDHitCollection =
    new LXeQDHitsCollection(SensitiveDetectorName, collectionName[0]);

  if(fHitCID < 0)
  {
    fHitCID = G4SDManager::GetSDMpointer()->GetCollectionID(fQDHitCollection);
  }
  hitsCE->AddHitsCollection(fHitCID, fQDHitCollection);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4bool LXeQDSD::ProcessHits(G4Step *aStep, G4TouchableHistory*) 
{
  G4Track *track = aStep->GetTrack();
  G4double edep = aStep->GetTotalEnergyDeposit();
  G4cout << "no qd hit" << G4endl;
  if (edep == 0.)
    {
      return false;
    }
  G4StepPoint * preStepPoint = aStep->GetPreStepPoint();
  G4TouchableHistory* theTouchable =
    (G4TouchableHistory*) (aStep->GetPreStepPoint()->GetTouchable());
  G4StepPoint * postStepPoint = aStep->GetPostStepPoint();

  G4ThreeVector iposPhoton = preStepPoint->GetPosition();
  G4ThreeVector fposPhoton = postStepPoint->GetPosition();
  
  G4VPhysicalVolume* thePrePV = theTouchable->GetVolume();
  LXeQDHit* qdHit = new LXeQDHit(thePrePV);

  qdHit->SetEdep(edep);
  fQDHitCollection->insert(qdHit);  
  return true;
}

