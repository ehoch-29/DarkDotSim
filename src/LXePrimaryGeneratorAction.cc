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
/// \file optical/LXe/src/LXePrimaryGeneratorAction.cc
/// \brief Implementation of the LXePrimaryGeneratorAction class
//
//
#include "LXePrimaryGeneratorAction.hh"

#include "LXePrimaryMessenger.hh"
#include "LXeEventAction.hh"

#include "LXeDetectorConstruction.hh"
#include "LXeHistoManager.hh"
#include "LXePMTHit.hh"
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

#include "LXeHistoManager.hh"
#include "globals.hh"
#include "G4Event.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4RandomDirection.hh"
#include "G4EventManager.hh"
#include "G4Runmanager.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LXePrimaryGeneratorAction::LXePrimaryGeneratorAction()
{
  fPrimaryMessenger = new LXePrimaryMessenger(this);
  
  G4int n_particle = 1;
  fParticleGun     = new G4ParticleGun(n_particle);
  G4double fPeak = 1.427 * eV;
  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  G4String particleName;
  fParticleGun->SetParticleDefinition(
  particleTable->FindParticle(particleName = "opticalphoton"));
  //fOuterRadius_pmt = fConstructor->GetPMTRadius();
  // Default energy,position,momentum
  fParticleGun->SetParticlePolarization(G4ThreeVector(0., 1., 0.));

  fParticleGun->SetParticlePosition(G4ThreeVector(5 * cm, 5* cm, -5. * cm));
  fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LXePrimaryGeneratorAction::~LXePrimaryGeneratorAction()
{
  delete fParticleGun;
  delete fPrimaryMessenger;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void LXePrimaryGeneratorAction::SetEnergy()
{
  //G4double energy  = G4RandGauss::shoot(fPeak, 0.08 * eV);  
  G4double energy = G4RandFlat::shoot(1.77* eV, 3.0 *eV);
  fParticleGun->SetParticleEnergy(energy);
  G4AnalysisManager::Instance()->FillH1(8, energy);
  G4AnalysisManager::Instance()->FillNtupleDColumn(0, 0, energy);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXePrimaryGeneratorAction::SetGeneratePhotons(G4double peak)
{
  fPeak = peak;

}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXePrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  SetEnergy();


  G4bool circle = true;
  G4double x_pos = G4RandFlat::shoot(-6.35, 6.35);
  G4double y_pos = G4RandFlat::shoot(-6.35, 6.35);
  G4double z_pos = G4RandFlat::shoot(-4, 4);
  if (x_pos*x_pos + y_pos*y_pos < (5*5))
    {
      circle = false;
    }
  while(circle)
  {
    x_pos = G4RandFlat::shoot(-6.35, 6.35);
    y_pos = G4RandFlat::shoot(-6.35, 6.35);

    if (x_pos*x_pos + y_pos*y_pos < (5*5))
      {
	circle = false;
      }
  }

  G4AnalysisManager::Instance()->FillNtupleDColumn(0, 4, x_pos);
  G4AnalysisManager::Instance()->FillNtupleDColumn(0, 5, y_pos);
  G4AnalysisManager::Instance()->FillNtupleDColumn(0, 6, z_pos);


  
  G4ThreeVector momentumUnitVector = G4RandomDirection();
  fParticleGun->SetParticleMomentumDirection(momentumUnitVector);
  fParticleGun->SetParticlePosition(G4ThreeVector(x_pos *cm, y_pos *cm, z_pos * cm));      
    
  fParticleGun->GeneratePrimaryVertex(anEvent);
}
