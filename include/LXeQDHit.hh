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
/// \file optical/LXe/include/LXeQDHit.hh
/// \brief Definition of the LXeQDHit class
//
//
#ifndef LXeQDHit_h
#define LXeQDHit_h 1

#include "G4Allocator.hh"
#include "G4LogicalVolume.hh"
#include "G4THitsCollection.hh"
#include "G4VHit.hh"
#include "G4VPhysicalVolume.hh"

class LXeQDHit : public G4VHit
{
 public:
  LXeQDHit();
  LXeQDHit(G4VPhysicalVolume* pVol);
  LXeQDHit(const LXeQDHit& right);
  ~LXeQDHit();

  const LXeQDHit& operator=(const LXeQDHit& right);
  G4bool operator==(const LXeQDHit& right) const;

  inline void* operator new(size_t);
  inline void operator delete(void* aHit);

  virtual void Draw();
  virtual void Print();

  inline void SetDrawit(G4bool b) { fDrawit = b; }
  inline G4bool GetDrawit() { return fDrawit; }

  inline void IncPhotonCount() { ++fPhotons; }
  inline G4int GetPhotonCount() { return fPhotons; }

  inline void SetEdep(G4double de) { fEdep = de; }
  inline void AddEdep(G4double de) { fEdep += de; }
  inline G4double GetEdep() { return fEdep; }

  inline void SetQDNumber(G4int n) { fQdNumber = n; }
  inline G4int GetQDNumber() { return fQdNumber; }

  inline void SetQDPhysVol(G4VPhysicalVolume* physVol)
  {
    this->fPhysVol = physVol;
  }
  inline void SetQDPos(G4double x, G4double y, G4double z)
  {
    fPos = G4ThreeVector(x, y, z);
  }
  
  inline G4VPhysicalVolume* GetQDPhysVol() { return fPhysVol; }

  inline G4ThreeVector GetQDPos() { return fPos; }

 private:
  G4int fQdNumber;
  G4int fPhotons;
  G4ThreeVector fPos;
  G4VPhysicalVolume* fPhysVol;
  G4bool fDrawit;
  G4double fEdep;
};

typedef G4THitsCollection<LXeQDHit> LXeQDHitsCollection;

extern G4ThreadLocal G4Allocator<LXeQDHit>* LXeQDHitAllocator;

inline void* LXeQDHit::operator new(size_t)
{
  if(!LXeQDHitAllocator)
    LXeQDHitAllocator = new G4Allocator<LXeQDHit>;
  return (void*) LXeQDHitAllocator->MallocSingle();
}

inline void LXeQDHit::operator delete(void* aHit)
{
  LXeQDHitAllocator->FreeSingle((LXeQDHit*) aHit);
}

#endif
