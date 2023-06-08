from helpers.G4_runClasses import *
from helpers.G4_macHelpers import *

LYlow = 3000.


if __name__ == "__main__":

  baseMacro = "gammaCs137_base.mac"

  ## build the base objects to run off
  physPars = PhysicsParameters(macPath=baseMacro)
  # print(physPars.to_Mac_string())
  detPars = DetectorParameters(macPath = baseMacro)
  # print(detPars.to_Mac_string())
  runPars = RunParameters(macPath=baseMacro)
  # print(runPars.to_Mac_string())

  heights = {0, 1.5, 4.0}
  lys = {3000., 3500, 4000., 4500, 5000., 5500, 6000.}

  for ly in lys:
    physPars.ly = ly

    for h in heights:
      runPars.gammaHeight = h
      runPars.fname = "Cs137_ly_"+str(int(ly))+"_h_"+str(h)+"_ly"
      runPars.Nbeam = 10000

      build_mac_from_items(physPar=physPars, detPar=detPars, runPar=runPars, 
                            fname = runPars.fname, savePath="./test1")

      