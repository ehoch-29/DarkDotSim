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
/// \file optical/LXe/src/LXeQDHit.cc
/// \brief Implementation of the LXeQDHit class
//
//
#include "LXeQDHit.hh"

#include "G4Colour.hh"
#include "G4ios.hh"
#include "G4LogicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VVisManager.hh"

G4ThreadLocal G4Allocator<LXeQDHit>* LXeQDHitAllocator = nullptr;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LXeQDHit::LXeQDHit()
  : fQdNumber(-1)
  , fPhotons(0)
  , fPhysVol(nullptr)
  , fDrawit(false)
  , fEdep(0.)
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LXeQDHit::~LXeQDHit() {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LXeQDHit::LXeQDHit(G4VPhysicalVolume* pVol)
  : fPhysVol(pVol)
{}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
LXeQDHit::LXeQDHit(const LXeQDHit& right)
  : G4VHit()
{
  fQdNumber  = right.fQdNumber;
  fPhotons   = right.fPhotons;
  fPhysVol   = right.fPhysVol;
  fDrawit    = right.fDrawit;
  fEdep      = right.fEdep;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

const LXeQDHit& LXeQDHit::operator=(const LXeQDHit& right)
{
  fQdNumber = right.fQdNumber;
  fPhotons   = right.fPhotons;
  fPhysVol   = right.fPhysVol;
  fDrawit    = right.fDrawit;
  fEdep      = right.fEdep;
  return *this;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4bool LXeQDHit::operator==(const LXeQDHit& right) const
{
  return (fQdNumber == right.fQdNumber);
}
void LXeQDHit::Draw()
{
  if(fDrawit && fPhysVol)
  {  // Redraw only the PMTs that have hit counts > 0
    // Also need a physical volume to be able to draw anything
    G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
    if(pVVisManager)
    {  // Make sure that the VisManager exists
      G4VisAttributes attribs(G4Colour(1., 0., 0.));
      attribs.SetForceSolid(true);
      G4RotationMatrix rot;
      if(fPhysVol->GetRotation())  // If a rotation is defined use it
        rot = *(fPhysVol->GetRotation());
      G4Transform3D trans(rot, fPhysVol->GetTranslation());  // Create transform
      pVVisManager->Draw(*fPhysVol, attribs, trans);         // Draw it
    }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void LXeQDHit::Print() {}
