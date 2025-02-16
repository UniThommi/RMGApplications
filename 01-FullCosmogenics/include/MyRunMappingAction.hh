#ifndef MyRunAction_HH
#define MyRunAction_HH 

#include "G4UserRunAction.hh"
#include "G4Run.hh"
#include "G4AnalysisManager.hh"

#include <map>
#include <string>
#include <mutex>

class G4Run;
class MyRunMappingAction: public G4UserRunAction {
  public:
    MyRunMappingAction();
    ~MyRunMappingAction();

  public:

    virtual void BeginOfRunAction(const G4Run* aRun) override;
    virtual void EndOfRunAction(const G4Run* aRun) override;

    static G4int GetPhysVolumeMappingID(G4String G4physVolumeName);
    static G4int GetMaterialMappingID(G4String G4materialName);


    // Mappings: Physisches Volumen und Material
    static std::map<std::string, int> physVolumeMapping;
    static std::map<std::string, int> materialMapping;

    static int physVolumeMappingID;
    static int materialMappingID;

    // Maximum ID der Mappings:
    static int maxIDPhysVolume;
    static int maxIDMaterial;


  private:

    // Um Mapping Operationen zu locken weil Multithread.
    static std::mutex _m;

    // Funktionen um ein Mapping für die Logical Volumes, Physical Volumes und MAterialien zu erstellen, indem das Neutron entsteht.
    static void ReadComponentMapping(std::map<std::string, int>* cmap,const std::string& filename);
    static void WriteComponentMapping(const std::map<std::string, int>* cmap, const std::string& filename);
    static void FindMaxID(const std::map<std::string, int>* cmap, int* maxElement);
    static bool SearchMapping(std::map<std::string, int>* cmap, const std::string& key, int* mappingID); 
    static void InsertMapping(std::map<std::string, int>* cmap, const std::string& key, int* mappingID, int* maxID); 

    // Filenamen der Mappings:
    std::string physVolumeFile = "PhysVolumeMapping.json";
    std::string materialFile = "MaterialMapping.json";
};

#endif