# FullCosmogenics
Features so far:
* Deploys Warwick-legend geometry via gdml file and overwrites detector efficiency with the supplied datasheet
* Registers Germanium detectors (without UID for now) and PMTs (with UID)
* Overwrites the IsotopeFilter to reduce memory consumption due to optical photons
* Saves the random seed of an event upon production of specified isotope
* Shell script that allows for reproduction of saved events
    * Due to Geant4 this only works on MT (the macro that reads in the seeds works only in MT for no reason at all)
    * Due to a Geant4 bug the seed files need to be in the working directory. The script manages this and avoids clutter of files.
