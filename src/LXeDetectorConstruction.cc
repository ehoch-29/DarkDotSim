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
/// \file optical/LXe/src/LXeDetectorConstruction.cc
/// \brief Implementation of the LXeDetectorConstruction class
//
//
#include "LXeDetectorConstruction.hh"

#include "LXeDetectorMessenger.hh"
#include "LXeMainVolume.hh"
#include "LXePMTSD.hh"
#include "LXeScintSD.hh"
#include "LXeQDSD.hh"
#include "LXeWLSSlab.hh"

#include "globals.hh"
#include "G4Box.hh"
#include "G4GeometryManager.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4MaterialTable.hh"
#include "G4OpticalSurface.hh"
#include "G4PhysicalConstants.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4PVPlacement.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4SolidStore.hh"
#include "G4Sphere.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Tubs.hh"
#include "G4UImanager.hh"
#include "G4VisAttributes.hh"
#include "G4NistManager.hh"


G4bool LXeDetectorConstruction::fSphereOn = false;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LXeDetectorConstruction::LXeDetectorConstruction()
  : fLXe_mt(nullptr)
  , fMPTPStyrene(nullptr)
{
  std::ifstream datafile;
  datafile.open("abs.dat");

  while(1)
    {
      G4double wlen, queff;
      datafile >> wlen >> queff;

      if(datafile.eof())
	break;
      G4cout << wlen << " " << queff << G4endl;
      
      e_abs.insert(e_abs.end(), {1.239841939*eV/(wlen/1000)});
      AbsPbS.insert(AbsPbS.end(), {queff *m});
    }
  std::reverse(e_abs.begin(), e_abs.end());
  std::reverse(AbsPbS.begin(), AbsPbS.end());
  datafile.close();

  fExperimentalHall_box  = nullptr;
  fExperimentalHall_log  = nullptr;
  fExperimentalHall_phys = nullptr;

  fQD = fLXe = fAl = fAir = fVacuum = fGlass = nullptr;
  fPMMA2 = fPC = fPstyrene = fPMMA = fPethylene1 = fPethylene2 = nullptr;

  fN = fO = fC = fH = fPb = fS = nullptr;

  fSaveThreshold = 0;
  SetDefaults();

  DefineMaterials();
  fDetectorMessenger = new LXeDetectorMessenger(this);

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LXeDetectorConstruction::~LXeDetectorConstruction()
{
  if(fMainVolume)
  {
    delete fMainVolume;
  }
  delete fLXe_mt;
  delete fDetectorMessenger;
  delete fMPTPStyrene;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeDetectorConstruction::DefineMaterials()
{
  G4double a;  // atomic mass
  G4double z;  // atomic number
  G4double density;

  G4int polyPMMA = 1;
  G4int nC_PMMA  = 3 + 2 * polyPMMA;
  G4int nH_PMMA  = 6 + 2 * polyPMMA;

  G4int polyeth = 1;
  G4int nC_eth  = 2 * polyeth;
  G4int nH_eth  = 4 * polyeth;

  //***Elements
  fH  = new G4Element("H" , "H" , z = 1. , a = 1.01  * g / mole);
  fC  = new G4Element("C" , "C" , z = 6. , a = 12.01 * g / mole);
  fN  = new G4Element("N" , "N" , z = 7. , a = 14.01 * g / mole);
  fO  = new G4Element("O" , "O" , z = 8. , a = 16.00 * g / mole);
  fPb = new G4Element("Pb", "Pb", z = 82., a = 207.2 * g / mole);
  fS  = new G4Element("S ", "S ", z = 16., a = 32.07 * g / mole);
  
  //***Materials
  // Liquid Xenon
  fLXe = new G4Material("LXe", z = 54., a = 131.29 * g / mole,
                        density = 3.020 * g / cm3);

  // Polycarbonate
  fPC = new G4Material("PC", density = 1.2 * g / cm3, 3);
  fPC->AddElement(fC, 15);
  fPC->AddElement(fH, 16);
  fPC->AddElement(fO, 2);

  //Quantum Dots
  fPbS = new G4Material("PbS", density = 1.2 * g/ cm3, 2);
  fPbS->AddElement(fPb, 1);
  fPbS->AddElement(fS, 1);
  
  fQD = new G4Material("QD", density = 1.18 * g/ cm3, 5);
  fQD->AddElement(fPb, 2.5 * perCent);
  fQD->AddElement(fS, 2.5 * perCent);
  fQD->AddElement(fH, 50.67 * perCent);
  fQD->AddElement(fC, 31.66 * perCent);
  fQD->AddElement(fO, 12.67 * perCent);
  
  // Aluminum
  fAl = new G4Material("Al", z = 13., a = 26.98 * g / mole,
                       density = 2.7 * g / cm3);
  // Vacuum
  fVacuum = new G4Material("Vacuum", z = 1., a = 1.01 * g / mole,
                           density = universe_mean_density, kStateGas,
                           0.1 * kelvin, 1.e-19 * pascal);
  // Air
  fAir = new G4Material("Air", density = 1.29 * mg / cm3, 2);
  fAir->AddElement(fN, 70 * perCent);
  fAir->AddElement(fO, 30 * perCent);
  // Glass
  fGlass = new G4Material("Glass", density = 1.032 * g / cm3, 2);
  fGlass->AddElement(fC, 91.533 * perCent);
  fGlass->AddElement(fH, 8.467 * perCent);

  // Polystyrene
  fPstyrene = new G4Material("Polystyrene", density = 1.03 * g / cm3, 2);
  fPstyrene->AddElement(fC, 8);
  fPstyrene->AddElement(fH, 8);

  // Fiber(PMMA)
  fPMMA = new G4Material("PMMA", density = 1190. * kg / m3, 3);
  fPMMA->AddElement(fH, nH_PMMA);
  fPMMA->AddElement(fC, nC_PMMA);
  fPMMA->AddElement(fO, 2);

  //Scintillator(PMMA)
  fPMMA2 = new G4Material("PMMA2", density = 1190. * kg / m3, 3);
  fPMMA2->AddElement(fH, nH_PMMA);
  fPMMA2->AddElement(fC, nC_PMMA);
  fPMMA2->AddElement(fO, 2);
  
  // Cladding(polyethylene)
  fPethylene1 = new G4Material("Pethylene1", density = 1200. * kg / m3, 2);
  fPethylene1->AddElement(fH, nH_eth);
  fPethylene1->AddElement(fC, nC_eth);
  // Double cladding(flourinated polyethylene)
  fPethylene2 = new G4Material("Pethylene2", density = 1400. * kg / m3, 2);
  fPethylene2->AddElement(fH, nH_eth);
  fPethylene2->AddElement(fC, nC_eth);

  //***Material properties tables

  std::vector<G4double> lxe_Energy = { 7.0 * eV, 7.07 * eV, 7.14 * eV };
  std::vector<G4double> lxe_SCINT = { 0.1, 1.0, 0.1 };

  std::vector<G4double> lxe_RIND  = { 1.59, 1.57, 1.54 };

  std::vector<G4double> lxe_ABSL  = { 35. * cm, 35. * cm, 35. * cm };

  fLXe_mt = new G4MaterialPropertiesTable();
  fLXe_mt->AddProperty("SCINTILLATIONCOMPONENT1", lxe_Energy, lxe_SCINT);
  fLXe_mt->AddProperty("SCINTILLATIONCOMPONENT2", lxe_Energy, lxe_SCINT);
  fLXe_mt->AddProperty("RINDEX", lxe_Energy, lxe_RIND);
  fLXe_mt->AddProperty("ABSLENGTH", lxe_Energy, lxe_ABSL);
  fLXe_mt->AddConstProperty("SCINTILLATIONYIELD", 1000. / MeV);
  fLXe_mt->AddConstProperty("RESOLUTIONSCALE", 1.0);
  fLXe_mt->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 20. * ns);
  fLXe_mt->AddConstProperty("SCINTILLATIONTIMECONSTANT2", 45. * ns);
  fLXe_mt->AddConstProperty("SCINTILLATIONYIELD1", 1.0);
  fLXe_mt->AddConstProperty("SCINTILLATIONYIELD2", 0.0);
  fLXe->SetMaterialPropertiesTable(fLXe_mt);

  // Set the Birks Constant for the LXe scintillator
  fLXe->GetIonisation()->SetBirksConstant(0.126 * mm / MeV);
  std::vector<G4double> pc_Energy    = {1.03 * eV, 1.08 *eV, 1.127 * eV, 1.18 * eV, 1.23 * eV,
					1.305 *eV, 1.377 * eV, 1.46* eV, 1.55* eV, 1.65 * eV,
					1.77 * eV, 1.907 * eV, 2.07 * eV, 2.25 * eV, 2.47 * eV,
					2.75 * eV, 3.099 * eV};
  std::vector<G4double> pc_AbsLength = {5.33 * cm, 2.53 * cm, 15.14 * cm, 20.47 * cm, 17.08 * cm,
					15.62 * cm, 15.63 * cm, 63.81 * cm, 1015 * cm, 173. * cm,
					106.71 * cm, 21.91 * cm, 9.94 * cm, 10.72 * cm, 13.48 * cm,
					13.61 * cm, 0.28 * cm};
  //std::vector<G4double> pc_AbsLength = {2. * cm, 19.53 * cm, 100. * cm};
  std::vector<G4double> pc_rindex    = {1.57, 1.57, 1.57, 1.57, 1.57,
					1.57, 1.57, 1.58, 1.58, 1.58,
					1.58, 1.58, 1.59, 1.60, 1.60,
					1.61, 1.63};
  //std::vector<G4double> pc_Energy = {1.03 * eV, 1.08* eV, 1.127 * eV, 1.3 * eV};
  //std::vector<G4double> pc_AbsLength = {5.33 * cm, 2.53 * cm, 15.14 * cm, 15.14 * cm};
  //std::vector<G4double> pc_rindex = {1.57, 1.57, 1.57, 1.57};
  G4MaterialPropertiesTable* pc_mt  = new G4MaterialPropertiesTable();
  pc_mt->AddProperty("ABSLENGTH", pc_Energy, pc_AbsLength);
  pc_mt->AddProperty("RINDEX", pc_Energy, pc_rindex);
  fPC->SetMaterialPropertiesTable(pc_mt);

  std::vector<G4double> pmma_AbsLength = {2.6 * cm, 1.5* cm, 6.04* cm, 6.33* cm, 10.96* cm,
					  9.88* cm, 7.84* cm, 24.86* cm, 23.75* cm, 22.35* cm,
					  21.18* cm, 20.21* cm, 17.2* cm, 19.19* cm,
					  17.76* cm, 15.98* cm, 8.33* cm};
  std::vector<G4double> pmma_rindex    = {1.47, 1.47, 1.47, 1.47, 1.48,
					  1.48, 1.48, 1.48, 1.48, 1.48,
					  1.48, 1.48, 1.48, 1.49, 1.49,
					  1.49, 1.50};
  //std::vector<G4double> pmma_AbsLength = {5.33 * cm, 2.53 * cm, 15.14 * cm, 14. * cm};
  //std::vector<G4double> pmma_rindex = {1.57, 1.57, 1.57, 1.57};

  G4MaterialPropertiesTable* pmma_mt = new G4MaterialPropertiesTable();
  pmma_mt->AddProperty("ABSLENGTH", pc_Energy, pmma_AbsLength);
  pmma_mt->AddProperty("RINDEX",  pc_Energy, pmma_rindex);
  fPMMA2->SetMaterialPropertiesTable(pmma_mt);
  
  std::vector<G4double> glass_AbsLength = { 420. * cm, 420.  * cm, 420. * cm};
  G4MaterialPropertiesTable* glass_mt   = new G4MaterialPropertiesTable();
  glass_mt->AddProperty("ABSLENGTH", lxe_Energy, glass_AbsLength);
  glass_mt->AddProperty("RINDEX", "Fused Silica");
  fGlass->SetMaterialPropertiesTable(glass_mt);

  G4MaterialPropertiesTable* vacuum_mt = new G4MaterialPropertiesTable();
  vacuum_mt->AddProperty("RINDEX", "Air");
  fVacuum->SetMaterialPropertiesTable(vacuum_mt);
  fAir->SetMaterialPropertiesTable(vacuum_mt);  // Give air the same rindex

  std::vector<G4double> wls_Energy = { 2.00 * eV, 2.87 * eV, 2.90 * eV,
                                       3.47 * eV };

  std::vector<G4double> rIndexPstyrene = { 1.5, 1.5, 1.5, 1.5 };
  std::vector<G4double> absorption1    = { 2. * cm, 2. * cm, 2. * cm, 2. * cm };
  std::vector<G4double> scintilFast    = { 0.0, 0.0, 1.0, 1.0 };
  fMPTPStyrene = new G4MaterialPropertiesTable();
  fMPTPStyrene->AddProperty("RINDEX", wls_Energy, rIndexPstyrene);
  fMPTPStyrene->AddProperty("ABSLENGTH", wls_Energy, absorption1);
  fMPTPStyrene->AddProperty("SCINTILLATIONCOMPONENT1", wls_Energy, scintilFast);
  fMPTPStyrene->AddConstProperty("SCINTILLATIONYIELD", 10. / keV);
  fMPTPStyrene->AddConstProperty("RESOLUTIONSCALE", 1.0);
  fMPTPStyrene->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 10. * ns);
  fPstyrene->SetMaterialPropertiesTable(fMPTPStyrene);

  // Set the Birks Constant for the Polystyrene scintillator
  fPstyrene->GetIonisation()->SetBirksConstant(0.126 * mm / MeV);

  std::vector<G4double> AbsFiber    = { 19.0 * cm, 15.47 * cm, 15.47 * cm, 8.3 * cm };
  //std::vector<G4double> AbsFiber    = { 11.1 * cm, 14.06 * cm, 22.12 * cm};
  std::vector<G4double> EmissionFib = { 1.0, 1.0, 0.0, 0.0 };
  std::vector<G4double> rPMMA       = {1.5, 1.5, 1.5, 1.5};
  G4MaterialPropertiesTable* fiberProperty = new G4MaterialPropertiesTable();
  fiberProperty->AddProperty("RINDEX", "PMMA");
  fiberProperty->AddProperty("WLSABSLENGTH", wls_Energy, AbsFiber);
  //fiberProperty->AddProperty("WLSCOMPONENT", wls_Energy, EmissionFib);
  //fiberProperty->AddConstProperty("WLSTIMECONSTANT", 0.5 * ns);
  fPMMA->SetMaterialPropertiesTable(fiberProperty);

  std::vector<G4double> RefractiveIndexClad1 = { 1.49, 1.49, 1.49, 1.49 };
  G4MaterialPropertiesTable* clad1Property   = new G4MaterialPropertiesTable();
  clad1Property->AddProperty("RINDEX", wls_Energy, RefractiveIndexClad1);
  clad1Property->AddProperty("ABSLENGTH", wls_Energy, AbsFiber);
  fPethylene1->SetMaterialPropertiesTable(clad1Property);

  std::vector<G4double> RefractiveIndexClad2 = { 1.42, 1.42, 1.42, 1.42 };
  G4MaterialPropertiesTable* clad2Property   = new G4MaterialPropertiesTable();
  clad2Property->AddProperty("RINDEX", wls_Energy, RefractiveIndexClad2);
  clad2Property->AddProperty("ABSLENGTH", wls_Energy, AbsFiber);
  fPethylene2->SetMaterialPropertiesTable(clad2Property);

  std::vector<G4double> RefractiveIndexPbS = {4, 4, 4, 4};
  std::vector<G4double> AbsPbS = {100. * cm, 100. *  cm, 100. * cm, 100. * cm};
  G4MaterialPropertiesTable* pbsProperty     = new G4MaterialPropertiesTable();
  pbsProperty->AddProperty("RINDEX", wls_Energy, RefractiveIndexPbS);
  pbsProperty->AddProperty("ABSLENGTH", wls_Energy, AbsPbS);
  fPbS->SetMaterialPropertiesTable(pbsProperty);
 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* LXeDetectorConstruction::Construct()
{
  // The experimental hall walls are all 1m away from housing walls
  G4double expHall_x = fScint_x + fD_mtl + 1. * m;
  G4double expHall_y = fScint_y + fD_mtl + 1. * m;
  G4double expHall_z = fScint_z + fD_mtl + 1. * m;

  // Create experimental hall
  fExperimentalHall_box =
    new G4Box("expHall_box", expHall_x, expHall_y, expHall_z);
  fExperimentalHall_log =
    new G4LogicalVolume(fExperimentalHall_box, fVacuum, "expHall_log", 0, 0, 0);
  fExperimentalHall_phys = new G4PVPlacement(
    0, G4ThreeVector(), fExperimentalHall_log, "expHall", 0, false, 0);

  fExperimentalHall_log->SetVisAttributes(G4VisAttributes::GetInvisible());

  // Place the main volume
  if(fMainVolumeOn)
  {
    fMainVolume = new LXeMainVolume(0, G4ThreeVector(), fExperimentalHall_log,
                                    false, 0, this);
  }

  // Place the WLS slab
  if(fWLSslab)
  {
    G4VPhysicalVolume* slab = new LXeWLSSlab(
      0, G4ThreeVector(0., 0., -fScint_z / 2. - fSlab_z - 1. * cm),
      fExperimentalHall_log, false, 0, this);

    // Surface properties for the WLS slab
    G4OpticalSurface* scintWrap = new G4OpticalSurface("ScintWrap");

    new G4LogicalBorderSurface("ScintWrap", slab, fExperimentalHall_phys,
                               scintWrap);

    scintWrap->SetType(dielectric_metal);
    scintWrap->SetFinish(polished);
    scintWrap->SetModel(glisur);

    std::vector<G4double> pp           = { 2.0 * eV, 3.5 * eV };
    std::vector<G4double> reflectivity = { 1.0, 1.0 };
    std::vector<G4double> efficiency   = { 0.0, 0.0 };

    G4MaterialPropertiesTable* scintWrapProperty =
      new G4MaterialPropertiesTable();

    scintWrapProperty->AddProperty("REFLECTIVITY", pp, reflectivity);
    scintWrapProperty->AddProperty("EFFICIENCY", pp, efficiency);
    scintWrap->SetMaterialPropertiesTable(scintWrapProperty);
  }

  return fExperimentalHall_phys;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeDetectorConstruction::ConstructSDandField()
{
  if(!fMainVolume)
    return;

  // PMT SD

  LXePMTSD* pmt = fPmt_SD.Get();
  if(!pmt)
  {
    // Created here so it exists as pmts are being placed
    G4cout << "Construction /LXeDet/pmtSD" << G4endl;
    LXePMTSD* pmt_SD = new LXePMTSD("/LXeDet/pmtSD");
    fPmt_SD.Put(pmt_SD);

    pmt_SD->InitPMTs();
    pmt_SD->SetPmtPositions(fMainVolume->GetPmtPositions());
  }
  else
  {
    pmt->InitPMTs();
    pmt->SetPmtPositions(fMainVolume->GetPmtPositions());
  }
  G4SDManager::GetSDMpointer()->AddNewDetector(fPmt_SD.Get());
  // sensitive detector is not actually on the photocathode.
  // processHits gets done manually by the stepping action.
  // It is used to detect when photons hit and get absorbed & detected at the
  // boundary to the photocathode (which doesn't get done by attaching it to a
  // logical volume.
  // It does however need to be attached to something or else it doesn't get
  // reset at the begining of events

  SetSensitiveDetector(fMainVolume->GetLogPhotoCath(), fPmt_SD.Get());

  // Scint SD

  if(!fScint_SD.Get())
  {
    G4cout << "Construction /LXeDet/scintSD" << G4endl;
    LXeScintSD* scint_SD = new LXeScintSD("/LXeDet/scintSD");
    fScint_SD.Put(scint_SD);
  }
  G4SDManager::GetSDMpointer()->AddNewDetector(fScint_SD.Get());
  SetSensitiveDetector(fMainVolume->GetLogScint(), fScint_SD.Get());
  //QD SD
  if(!fQD_SD.Get())
  {
    G4cout << "Construction /LXeDet/qdSD" << G4endl;
    LXeQDSD* qd_SD = new LXeQDSD("/LXeDet/qdSD");
    fQD_SD.Put(qd_SD);
  }
  G4SDManager::GetSDMpointer()->AddNewDetector(fQD_SD.Get());
  SetSensitiveDetector(fMainVolume->GetLogQD(), fQD_SD.Get());

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeDetectorConstruction::SetDimensions(G4ThreeVector dims)
{
  fScint_x = dims[0];
  fScint_y = dims[1];
  fScint_z = dims[2];
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeDetectorConstruction::SetHousingThickness(G4double d_mtl)
{
  fD_mtl = d_mtl;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeDetectorConstruction::SetNX(G4int nx)
{
  fNx = nx;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeDetectorConstruction::SetNY(G4int ny)
{
  fNy = ny;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeDetectorConstruction::SetNZ(G4int nz)
{
  fNz = nz;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeDetectorConstruction::SetPMTRadius(G4double outerRadius_pmt)
{
  fOuterRadius_pmt = outerRadius_pmt;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeDetectorConstruction::SetDefaults()
{
  // Resets to default values
  fD_mtl = 0.0635 * cm;

  fScint_x = 10. * cm;
  fScint_y = 10. * cm;
  fScint_z = 8. * cm;
  //fScint_z = 22.6 * cm;

  fNx = 1;
  fNy = 1;
  fNz = 1;

  fOuterRadius_pmt = 6.35 * cm;

  fSphereOn = false;
  fRefl     = 1.0;

  fNfibers      = 15;
  fWLSslab      = false;
  fMainVolumeOn = true;
  fMainVolume   = nullptr;
  fSlab_z       = 2.5 * mm;

  G4UImanager::GetUIpointer()->ApplyCommand(
    "/LXe/detector/scintYieldFactor 1.");

  if(fLXe_mt)
    fLXe_mt->AddConstProperty("SCINTILLATIONYIELD", 12000. / MeV);
  if(fMPTPStyrene)
    fMPTPStyrene->AddConstProperty("SCINTILLATIONYIELD", 10. / keV);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeDetectorConstruction::SetSphereOn(G4bool b)
{
  fSphereOn = b;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeDetectorConstruction::SetHousingReflectivity(G4double r)
{
  fRefl = r;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeDetectorConstruction::SetWLSSlabOn(G4bool b)
{
  fWLSslab = b;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeDetectorConstruction::SetMainVolumeOn(G4bool b)
{
  fMainVolumeOn = b;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeDetectorConstruction::SetNFibers(G4int n)
{
  fNfibers = n;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeDetectorConstruction::SetMainScintYield(G4double y)
{
  fLXe_mt->AddConstProperty("SCINTILLATIONYIELD", y / MeV);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeDetectorConstruction::SetWLSScintYield(G4double y)
{
  fMPTPStyrene->AddConstProperty("SCINTILLATIONYIELD", y / MeV);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeDetectorConstruction::SetSaveThreshold(G4int save)
{
  // Sets the save threshold for the random number seed. If the number of
  // photons generated in an event is lower than this, then save the seed for
  // this event in a file called run###evt###.rndm

  fSaveThreshold = save;
  G4RunManager::GetRunManager()->SetRandomNumberStore(true);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
