#include "MyRunMappingAction.hh"

#include <nlohmann/json.hpp>
#include <map>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <string>

using json = nlohmann::json;

MyRunMappingAction::MyRunMappingAction() {
    G4cout << "Initialize MyRunMappingAction" << G4endl;
};

MyRunMappingAction::~MyRunMappingAction() {};


// Mappingparameter, an die Klasse gebunden aber nicht Teil einer Instanz, sodass sie nur 1mal existieren.
std::map<std::string, int> MyRunMappingAction::physVolumeMapping;
std::map<std::string, int> MyRunMappingAction::materialMapping;

int MyRunMappingAction::physVolumeMappingID = -1;
int MyRunMappingAction::materialMappingID = -1;

int MyRunMappingAction::maxIDPhysVolume = -1;
int MyRunMappingAction::maxIDMaterial = -1;

// Locke den Code der mit den Mappings zu tun hat.
std::mutex MyRunMappingAction::_m;


void MyRunMappingAction::BeginOfRunAction(const G4Run* aRun) {
    // Get Physical Volume and Material Mapping
    std::lock_guard<std::mutex> guard(MyRunMappingAction::_m);
    G4cout << "BeginOfRunAction: Hole Mappings" << G4endl;
    MyRunMappingAction::ReadComponentMapping(&MyRunMappingAction::physVolumeMapping, physVolumeFile);
    MyRunMappingAction::FindMaxID(&MyRunMappingAction::physVolumeMapping, &MyRunMappingAction::maxIDPhysVolume);

    MyRunMappingAction::ReadComponentMapping(&MyRunMappingAction::materialMapping, materialFile);
    MyRunMappingAction::FindMaxID(&MyRunMappingAction::materialMapping, &MyRunMappingAction::maxIDMaterial);
  
}

void MyRunMappingAction::EndOfRunAction(const G4Run* aRun) {
    std::lock_guard<std::mutex> guard(MyRunMappingAction::_m);
    G4cout << "EndOfRunAction: Schreibe Mappings weg" << G4endl;
    MyRunMappingAction::WriteComponentMapping(&MyRunMappingAction::physVolumeMapping, physVolumeFile);
    MyRunMappingAction::WriteComponentMapping(&MyRunMappingAction::materialMapping, materialFile);   
}

G4int MyRunMappingAction::GetPhysVolumeMappingID(G4String G4PhysVolumeName) {
    std::string physVolumeName = static_cast<std::string>(G4PhysVolumeName);
    bool found = MyRunMappingAction::SearchMapping(&MyRunMappingAction::physVolumeMapping, physVolumeName, &MyRunMappingAction::physVolumeMappingID);
    if (!found) {
        std::lock_guard<std::mutex> guard(MyRunMappingAction::_m);
        MyRunMappingAction::InsertMapping(&MyRunMappingAction::physVolumeMapping, physVolumeName, &MyRunMappingAction::physVolumeMappingID, &MyRunMappingAction::maxIDPhysVolume);
    }
    return static_cast<G4int>(MyRunMappingAction::physVolumeMappingID);
}

G4int MyRunMappingAction::GetMaterialMappingID(G4String G4materialName) {
    std::string materialName = static_cast<std::string>(G4materialName);
    bool found = MyRunMappingAction::SearchMapping(&MyRunMappingAction::materialMapping, materialName, &MyRunMappingAction::materialMappingID);
    if (!found) {
        std::lock_guard<std::mutex> guard(MyRunMappingAction::_m);
        MyRunMappingAction::InsertMapping(&MyRunMappingAction::materialMapping, materialName, &MyRunMappingAction::materialMappingID, &MyRunMappingAction::maxIDMaterial);
    }
    return static_cast<G4int>(MyRunMappingAction::materialMappingID);
}

// Laden der Mappings
void MyRunMappingAction::ReadComponentMapping(std::map<std::string, int>* cmap, const std::string& filename) {
    std::ifstream file(filename);
    G4cout << "Lese Mapping " << filename << G4endl;
  
    if (file.is_open()) {
        try {
            json j;
            file >> j;
            for (auto& [key, value] : j.items()) {
                (*cmap)[key] = value;
            }
        } catch (const std::exception& e) {
            G4cout << "Error reading JSON file: " << e.what() << G4endl;
        }
        file.close();
    } else {
        std::ofstream newFile(filename);
        if (newFile.is_open()) {
            newFile << "{}";
            newFile.close();
        } else {
            G4cout << "Failed to create file: " << filename << G4endl;
        }
    }
  }
  
  // suche nach Mapping. WEnn es keins gibt erweitere es.
bool MyRunMappingAction::SearchMapping(std::map<std::string, int>* cmap, const std::string& key, int* mappingID) {
    // Check if the key exists
    auto it = cmap->find(key);
    if (it != cmap->end()) {
        G4cout << "Mapping wurde gefunden für " << key << " mit ID " << it->second << G4endl; 
        *mappingID = it->second;
        return true;
    }
    else {
        *mappingID = -1;
        return false;
    }
  }

void MyRunMappingAction::InsertMapping(std::map<std::string, int>* cmap, const std::string& key, int* mappingID, int* maxID) {
    // Insert a new entry with a unique value
    (*cmap)[key] = ++(*maxID);
    G4cout << "Neues Mapping wurde eingetragen für " << key << " mit ID " << *maxID << G4endl; 
    *mappingID = *maxID;
}
  
  // Schreiben der Mappings.
void MyRunMappingAction::WriteComponentMapping(const std::map<std::string, int>* cmap, const std::string& filename) {
    // Convert the map to a JSON object
    G4cout << "Schreibe Mapping weg " << filename << G4endl;
    json j;
    for (const auto& [key, value] : *cmap) {
        j[key] = value;
    }

    // Write JSON to file
    std::ofstream file(filename);
    if (file.is_open()) {
        file << j.dump(4); // Pretty-print with 4 spaces
        file.close();
        G4cout << "JSON file " << filename << " written successfully!" << G4endl;
    } else {
        G4cout << "Failed to open file " << filename << " for writing." << G4endl;
    }
}
  
  // Finde den Größten Identifier eines Mappings
void MyRunMappingAction::FindMaxID(const std::map<std::string, int>* cmap, int* maxID) {
    G4cout << "Finde Maximale ID des Mappings:" << G4endl;
    if (cmap->empty()) return;  // Wenn Map leer ist, return 0

    auto maxElement = std::max_element(
        cmap->begin(), cmap->end(),
        [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
            return a.second < b.second;
        }
    );
    *maxID = maxElement->second; 
    G4cout << *maxID << G4endl;
}
  
