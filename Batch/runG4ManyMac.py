from G4_runClasses import *
from G4_macHelpers import *

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

  heights = {-0.5, 0, 0.5, 1.5, 4.0, 5.5}
  ly = {3000., 4000., 5000., 6000.}

  build_mac_from_items(physPar=physPars, detPar=detPars, runPar=runPars)

  