"""
run with python3.11 (where tqdm is installed)

run from wherever

this is the 'add on' file to generate new run types in the existing folders (parameter space)

the G4 verbosities are NOT copied from the base macro file, they are done explicitly in G4_runClasses.py

1. pass the base macro and file associated
2. this makes new folders for each parameter set (starting after existing folders)

"""

import sys
import subprocess # to call mkdir and build .mac files we need
from multiprocessing import Pool
from tqdm import tqdm

from helpers.G4_runClasses import *
from helpers.G4_macHelpers import *
from helpers.FileGetters import *

path = "/Users/austindestcroix/Desktop/school_holders/projects/sbc/g4_cf4_updated/"
pBatch = path+"batchOutput_feb9_lowLY/"
pBuild = path+"build/"

TESTING = False

# for cleaner control
class RunControl:
  pass
## -- controls -- ###
rc = RunControl()
rc.verbose = False
rc.eventVerbose = True
rc.execName = "cf4Chamber"

rc.nbeam_end = 1e6 # gammas of maximum ly value.
rc.heightList = [0.0, 1.5, 4.0]
rc.baseMacro = "gammaCo57_base.mac" # this should be in same directory
rc.baseName = "Co57"

# unique to fillFolders
rc.pullFrom = "Cs137"
rc.Nscaling = 2 # how much to multiply Cs Nbeam by

if TESTING:
  # unique for runG4ManyMac
  # rc.lyList = rc.lyList[:2]
  # rc.heightList = rc.heightList[:2]
  # rc.nbeam_end = 1e4
  rc.Nscaling = 1e-3
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
  return parseStringMyWay(command)

if __name__ == "__main__":

  ## build the base objects to run off
  physPars = PhysicsParameters(macPath=rc.baseMacro)
  detPars = DetectorParameters(macPath = rc.baseMacro)
  runPars = RunParameters(macPath=rc.baseMacro)
  # print(physPars.to_Mac_string())
  # print(detPars.to_Mac_string())
  # print(runPars.to_Mac_string())

  geantCommandList = [] # list of things that should work as long as in G4build directory

  ## getting the list of folders that already exist
  all_items = os.listdir(pBatch)
  folders = [int(item) for item in all_items if os.path.isdir(os.path.join(pBatch, item))]
  folders = sorted(folders, key=lambda x: x)
  
  if TESTING:
    folders = folders[:2]


  for folder in folders: # this loops on LY right now
    macs = get_all_mac_files(pBatch+str(folder)+ "/")
    # print("folder %s" % f)
    # print(macs[0])
    otherPhysPars = PhysicsParameters(macPath = macs[0])
    otherRunPars = RunParameters(macPath = macs[0])
    
    ## loading in data from mac file
    gotit = False
    for name in macs:
      otherPhysPars = PhysicsParameters(macPath = name)
      otherRunPars = RunParameters(macPath = name)
      if otherRunPars.gammaSrc == rc.pullFrom:
        gotit = True
        break
    if gotit == False: # safety check
      print("couldn't get good .mac file to pull", rc.pullFrom, " from")
      exit(1)

    # modifying the parameters to match  
    newLY = otherPhysPars.ly
    otherNBEAM = int(otherRunPars.Nbeam)

    # setting new variables
    physPars.ly = newLY
    runPars.Nbeam = rc.Nscaling * otherNBEAM
    print()
    print(folder, newLY, runPars.Nbeam)

    for h in rc.heightList:
      runPars.gammaHeight = h  # set the run parameters

      # this is the important line for file naming and location
      height_string = str(h).replace(".", "d") # no floats in name
      runPars.fname = pBatch + f"{folder}/" + rc.baseName + "_ly_"+str(int(float(newLY)))+"_h_"+height_string
      print(runPars.fname)

      # it works with savepath empty. all info is in the runPars.fname member
      # runPars.fname also has no suffix
      macName = build_mac_from_items(physPar=physPars, detPar=detPars, runPar=runPars, 
                            fname = runPars.fname, savePath="")

      g4Command = f"cd {pBuild}; ./{rc.execName} {macName}"
      geantCommandList.append(g4Command)
  
  # done paramter space (nested) forloops
  # done building G4 command list
  
  #### executing G4 commands ####
  nProc = 4
  # Create a pool of worker processes
  with Pool(nProc) as pool:
      # Create a tqdm progress bar
      progress_bar = tqdm(total=len(geantCommandList))

      # Use pool.imap() to execute the function in parallel
      # Update the progress bar after each task completion
      for result in pool.imap(run_process, geantCommandList):
          print(f"finished: {result}") # put something here to write about what physics run was done
          progress_bar.update(1)
      
      # Close the progress bar
      progress_bar.close()

  # pool = Pool(processes = 4)
  # results = pool.imap_unordered(run_process, geantCommandList)
  # list(results)

  print("\n\n\n\n\n\n")
  print("----------------------------------------------------------------")
  print("-------- Done running G4 over paremeter space --------")
  print("----------------------------------------------------------------")