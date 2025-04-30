#ifndef MY_PHOTON_HIT_HH
#define MY_PHOTON_HIT_HH

#pragma once

#include "G4ThreeVector.hh"
#include "G4Allocator.hh"
#include "globals.hh"

// PhotonHit speichert alle relevanten Infos eines optischen Photons, das einen Sensitive Detector trifft
class PhotonHit {
public:
    PhotonHit();
    ~PhotonHit();

    // Setters for the hit data
    void SetDetectorUID(G4int detectorUID);
    void SetOptPhotonEnergy(G4double optPhotonEnergy);
    void SetOptPhotonglobalTime(G4double optPhotonglobalTime);
    void SetOptPhotonPosition(G4ThreeVector optPhotonPosition);
    void SetOptPhotonMomentumDirection(G4ThreeVector optPhotonMomentumDirection);
    void SetnCTrackID(G4int nCTrackID);
    void SetnCPos(G4ThreeVector nCPos);
    void SetnCTime(G4double nCTime);
    void SetnCPhysVol(G4string nCPhysVolID);
    void SetnCMaterial(G4string nCMaterialID);
    void SetnCGammaAmount(G4int nCGammaAmount);
    void SetnCGammaTotalEnergy(G4double nCGammaTotalEnergy);
    void SetnCfGe77(G4bool nCfGe77lse);
    void SetGammaMomentumDirection(G4ThreeVector gammaMomentumDirection);
    void SetGammaKineticEnergy(G4double gammaKineticEnergy);


    // Allocator Overloads
    void* operator new(size_t);
    void operator delete(void*);

private:
    G4int detectorUID;                         // ID des getroffenen Detektors
    G4double optPhotonEnergy;                 // Energie des Photons (z.B. in eV)
    G4double optPhotonGlobalTime;             // Globalzeit des Treffers
    G4ThreeVector optPhotonPosition;           // Trefferposition
    G4ThreeVector optPhotonMomentumDirection;  // Flugrichtung beim Treffer

    G4int nCTrackID;
    G4ThreeVector nCPos;
    G4double nCTime;
    G4string nCPhysVol;
    G4string nCMaterial;
    G4int nCGammaAmount;
    G4double nCGammaTotalEnergy;
    G4bool nCfGe77;
    G4ThreeVector gammaMomentumDirection;
    G4double gammaKineticEnergy.;               

};

// Globale Deklaration des Allocators
extern G4Allocator<PhotonHit>* PhotonHitAllocator;
