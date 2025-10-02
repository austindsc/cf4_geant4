"""
run with python3.11 (where tqdm is installed)

run from wherever (bash calls cd to g4 directory)

this is the 'master' file to generate the first runs of the parameter space

the verbosities are NOT copied from the base macro file, they are done explicitly in G4_runClasses.py

1. pass the base macro and file associated
2. this makes new folders for each parameter set (starting after existing folders)

"""

import sys
import os
import subprocess # to call mkdir and build .mac files we need
from multiprocessing import Pool
from tqdm import tqdm

from helpers.G4_runClasses import *
from helpers.G4_macHelpers import *

# path = "/Users/austindestcroix/Desktop/school_holders/projects/sbc/g4cf4_newOptics/"
path = os.getenv("CF4G4")
# print("hello world")
pBatch = path+"batchOutput/alpha_sept25/"
pBuild = path+"build/"

TESTING = False

# for cleaner control
class RunControl:
  pass

## -- controls -- ###
rc = RunControl()
rc.verbose = True
rc.eventVerbose = 0 # 0 is silent, 1 is a little, 2 is a lot
rc.nbeam_end = 1e5 # gammas of maximum ly value.
rc.execName = "cf4Chamber"
# rc.lyList = np.arange(0.1, 5, 0.5).tolist()
rc.lyList = [0.1]
rc.heightList = [0.5]
# rc.fvisList = np.arange(0.5, 1.0, 0.1)
rc.fvisList = np.array([0.5])

rc.baseMacro = "alpha_base.mac" # this should be in same directory as this script
rc.baseName = "Alpha"
loopSelect = "alpha"

# unique for runG4ManyMac
if TESTING:
  rc.lyList = rc.lyList[:2]
  rc.heightList = rc.heightList[:2]
  rc.nbeam_end = 1e4
## -- Done Controls -- ##

# should improve the wrapping or ownership of the 'build_from_mac_items" function
def parseStringMyWay(sin, first = "cf4", second="geant4_cf4"):
  ind = sin.find(first)
  newOut = sin[ind+len(first):]
  ind = newOut.find(second)
  return newOut[ind+len(second):]

def run_process(command, verbose = rc.verbose):
  if verbose == False:
    subprocess.run(command, shell = True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)
  else:
    subprocess.run(command, shell = True)
  # mac = somehow build macro file from info
  # or pass a 2d array that holds physPar/runPar and the g4 command
  # or change this command to be more 'complex', taking in the mac object, and
  # building the G4 command here. Then I can have this return whatever info I need
  # to have a nice printed output
  return parseStringMyWay(command)

if __name__ == "__main__":

  ###### running gammas (forloop on height and light yield) ######
  if loopSelect == "gamma":

    ## build the base objects to run off
    physPars = PhysicsParameters(macPath=rc.baseMacro)
    detPars = DetectorParameters(macPath = rc.baseMacro)
    runPars = RunParameters(macPath=rc.baseMacro)
    
    geantCommandList = [] # list of things that should work as long as in G4build directory

    ## getting the list of folders that already exist
    all_items = os.listdir(pBatch)
    folders = [int(item) for item in all_items if os.path.isdir(os.path.join(pBatch, item))]
    lastFolder = 0
    if len(folders) > 0:
      folders = sorted(folders, key=lambda x: x)
      lastFolder = max(folders) + 1

    # make new folders
    for iID, ly in enumerate(rc.lyList):

      NBEAM = rc.nbeam_end * max(rc.lyList) / ly
      # set the physics parameters
      physPars.ly = ly

      folderID = iID + lastFolder
      # make a uniquely named directory
      subprocess.run(f"cd {pBatch} ; mkdir {folderID}", shell = True)
      parDirID = str(folderID)
      
      for h in rc.heightList:
        # set the run parameters
        runPars.gammaHeight = h

        # this is the important line for file naming and location
        hstr = str(h).replace(".", "d")
        runPars.fname = pBatch + f"{parDirID}/" + rc.baseName + "_ly_"+str(int(ly))+"_h_"+hstr
        runPars.Nbeam = int(NBEAM)

        # it works with savepath empty. all info is in the fname member
        macName = build_mac_from_items(physPar=physPars, detPar=detPars, runPar=runPars, 
                              fname = runPars.fname, savePath="", myVerbosity = rc.eventVerbose)

        g4Command = f"cd {pBuild}; ./{rc.execName} {macName}"
        geantCommandList.append(g4Command)
  ## done the gamma loop

  elif loopSelect == "alpha":
    ## build the base objects to run off
    physPars = PhysicsParameters(macPath=rc.baseMacro)
    detPars = DetectorParameters(macPath = rc.baseMacro)
    runPars = RunParameters(macPath=rc.baseMacro)
    runPars.Nbeam = int(rc.nbeam_end)
    
    geantCommandList = [] # list of things that should work as long as in G4build directory

    ## getting the list of folders that already exist
    all_items = os.listdir(pBatch)
    folders = [int(item) for item in all_items if os.path.isdir(os.path.join(pBatch, item))]
    lastFolder = 0
    if len(folders) > 0:
      folders = sorted(folders, key=lambda x: x)
      lastFolder = max(folders) + 1

    # make new folders
    for iID, visFrac in enumerate(rc.fvisList):

      # set the physics parameters
      physPars.vf = visFrac

      folderID = iID + lastFolder
      # make a uniquely named directory
      subprocess.run(f"cd {pBatch} ; mkdir {folderID}", shell = True)
      parDirID = str(folderID)

      vfStr = str(round(visFrac, 3)).replace(".", "d")
      ## bare run
      runPars.lpFilter = False
      runPars.fname = pBatch + f"{parDirID}/" + rc.baseName + "_vf_" + vfStr + "_bare"

      # it works with savepath empty. all info is in the fname member
      macName = build_mac_from_items(physPar=physPars, detPar=detPars, runPar=runPars, 
                          fname = runPars.fname, savePath="", myVerbosity = rc.eventVerbose)

      g4Command = f"cd {pBuild}; ./{rc.execName} {macName}"
      geantCommandList.append(g4Command)
      
      ## LP run
      runPars.lpFilter = True
      runPars.fname = pBatch + f"{parDirID}/" + rc.baseName + "_vf_" + vfStr + "_LP"
      macName = build_mac_from_items(physPar=physPars, detPar=detPars, runPar=runPars, 
                          fname = runPars.fname, savePath="", myVerbosity = rc.eventVerbose)
      g4Command = f"cd {pBuild}; ./{rc.execName} {macName}"
      geantCommandList.append(g4Command)

  else:
    print(f"ERROR: Unknown loopSelect value: {loopSelect}")
    print("Valid options are: 'gamma' or 'alpha'")
    sys.exit(1)
    
  ## now doing the commands
  nProc = 4
  with Pool(nProc) as pool:   # Create a pool of worker processes
    progress_bar = tqdm(total=len(geantCommandList))       # Create a tqdm progress bar

    # Use pool.imap() to execute the function in parallel and update progress bar
    for result in pool.imap(run_process, geantCommandList):
        print(f"finished: {result}") # put something here to write about what physics run was done
        progress_bar.update(1)
      
    # results = pool.imap(run_process, geantCommandList)
    # for result in results:
    #   print(type(result), end="\r")
    #   progress_bar.update(1)
      
    # Close the progress bar
    progress_bar.close()

  # pool = Pool(processes = 4)
  # results = pool.imap_unordered(run_process, geantCommandList)
  # list(results)

  print("\n\n\n\n\n\n")
  print("----------------------------------------------------------------")
  print("-------- Done running G4 over paremeter space --------")
  print("----------------------------------------------------------------")