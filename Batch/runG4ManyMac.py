"""
run with python3.11 (where tqdm is installed)

run from

this is the 'master' file to generate the first runs of the parameter space

the verbosities are NOT copied from the base macro file, they are done explicitly in G4_runClasses.py

1. pass the base macro and file associated
2. this makes new folders for each parameter set (starting after existing folders)

"""

import sys
import subprocess # to call mkdir and build .mac files we need
from multiprocessing import Pool
from tqdm import tqdm

from helpers.G4_runClasses import *
from helpers.G4_macHelpers import *

path = "/Users/austindestcroix/Desktop/school_holders/projects/sbc/g4_cf4_updated/"
# pBatch = path+"batchOutput/"
pBatch = path+"batchOutput_feb2/"
pBuild = path+"build/"

TESTING = False

# just for cleaner control
class RunControl:
  pass

### controls ###
rc = RunControl()
rc.verbose = False
rc.nbeam_end = 1e6 # gammas of maximum ly value.
rc.execName = "cf4Chamber"
rc.lyList = np.arange(4000, 8000, 500).tolist() + np.arange(8000, 13000, 1000).tolist()
rc.heightList = [0.0, 1.5, 4.0, 5.5]

if TESTING:
  rc.lyList = rc.lyList[:2]
  rc.heightList = rc.heightList[:2]
  rc.nbeam_end = 1e4

# why not just put the parameter ID in the mac file as a comment!?
# don't need unique directories, but i like that for organization
# will make another code that builds the 'master' csv to track parameter space and assign ID

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

  # baseMacro = "gammaCo57_base.mac"
  # baseName = "Co57"
  baseMacro = "gammaCs137_base.mac"
  baseName = "Cs137"

  ## build the base objects to run off
  physPars = PhysicsParameters(macPath=baseMacro)
  detPars = DetectorParameters(macPath = baseMacro)
  runPars = RunParameters(macPath=baseMacro)
  # print(physPars.to_Mac_string())
  # print(detPars.to_Mac_string())
  # print(runPars.to_Mac_string())
  
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
    # print(folderID)
    # break

    # make a uniquely named directory
    subprocess.run(f"cd {pBatch} ; mkdir {folderID}", shell = True)
    parDirID = str(folderID)
    # control parameter ID here
    for h in rc.heightList:

      # set the run parameters
      runPars.gammaHeight = h

      # this is the important line for file naming and location
      hstr = str(h).replace(".", "d")
      runPars.fname = pBatch + f"{parDirID}/" + baseName + "_ly_"+str(int(ly))+"_h_"+hstr
      runPars.Nbeam = int(NBEAM)

      # it works with savepath empty. all info is in the fname member
      # runPars.fname has no suffix
      macName = build_mac_from_items(physPar=physPars, detPar=detPars, runPar=runPars, 
                            fname = runPars.fname, savePath="")

      g4Command = f"cd {pBuild}; ./{rc.execName} {macName}"
      geantCommandList.append(g4Command)

      # subprocess.run(g4Command, shell=True)
  
  # for command in geantCommandList:
  #   print(command)
  # exit(0)
  # done paramter space (nested) forloops
  # done building G4 command list
  
  #### executing G4 commands ####
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