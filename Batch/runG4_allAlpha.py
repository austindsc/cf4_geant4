"""
run with python3.11 (where tqdm is installed)

run from wherever (bash calls cd to g4 directory)

this is the 'master' file to generate the first runs of the parameter space

the verbosities are NOT copied from the base macro file, they are done explicitly in G4_runClasses.py

1. pass the base macro and file associated
2. this makes new folders for each parameter set (starting after existing folders)

-- 
to run verbosely (see all g4 output in terminal), set rc.verbose = True
to log all that output:
p311 runG4_allAlpha.py > output.log 2>&1

The above will get big and crash for large runs.
to run quietly (no g4 output, just progress bar), set rc.verbose = False

to only save last 1000 lines (avoid crash), with rc.verbose = True:
p311 runG4_allAlpha.py 2>&1 | (tail -n 1000 > last1000.log; cat > /dev/null)
# p311 runG4_allAlpha.py 2>&1 | tail -n 1000 > last1000.log  # This waits until end, can crash
"""

# Suppress macOS secure coding warning
import os

import sys
import subprocess # to call mkdir and build .mac files we need
from multiprocessing import Pool
import multiprocessing as mp

from tqdm import tqdm

from helpers.G4_runClasses import *
from helpers.G4_macHelpers import *

# import batchFunctions as BF

path = os.getenv("CF4G4")
pBatch = path+"batchOutput/alphaAll_oct2025/"
pBuild = path+"build/"


## -- controls -- ###
# Import RunControl class (or you could define it here)
class RunControl:
  pass
rc = RunControl()

rc.verbose = False  # Simple: suppress G4 output, show only progress bar
rc.eventVerbose = 0 # 0 is silent, 1 is a little, 2 is a lot

rc.nbeam_end = 1e6
rc.execName = "cf4Chamber"

rc.baseMacro = "alpha_base.mac" # this should be in same directory as this script
rc.baseName = "alpha_base"

## physics parameters
rc.fvisList = np.arange(0.3, 0.8, 0.05)
rc.fvisList = np.round(rc.fvisList, 2) # avoid weird float things
rc.voltageList = [3.0, 4.0, 5.0]


## -- Done Controls -- ##
def run_process_quiet(command):
  subprocess.run(command, shell = True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
  return command

def run_process_verbose(command):
  subprocess.run(command, shell = True)
  return command

if rc.verbose:
  run_process = run_process_verbose
else:
  run_process = run_process_quiet


if __name__ == "__main__":

  ###### running gammas (forloop on height and light yield) ######

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
    
    folderID = iID + lastFolder
    # make a uniquely named directory (in lowest for loop)
    subprocess.run(f"cd {pBatch} ; mkdir {folderID}", shell = True)
    parDirID = str(folderID)
    
    # set the physics parameters
    physPars.vf = visFrac
    vfStr = str(round(visFrac, 3)).replace(".", "d")
    
    for voltage in rc.voltageList:
      # set the detector parameters
      detPars.voltage = voltage
      
      #### bare run
      runPars.lpFilter = False
      runPars.fname = pBatch + f"{parDirID}/" + rc.baseName + "_vf_" + vfStr + "_bare_" + str(int(voltage)) + "V"
      # it works with savepath empty. all info is in the fname member
      macName = build_mac_from_items(physPar=physPars, detPar=detPars, runPar=runPars, 
                          fname = runPars.fname, savePath="", myVerbosity = rc.eventVerbose)
      g4Command = f"cd {pBuild}; ./{rc.execName} {macName}"
      geantCommandList.append(g4Command)
      
      #### LP run
      runPars.lpFilter = True
      runPars.fname = pBatch + f"{parDirID}/" + rc.baseName + "_vf_" + vfStr + "_LP_" + str(int(voltage)) + "V"
      macName = build_mac_from_items(physPar=physPars, detPar=detPars, runPar=runPars, 
                          fname = runPars.fname, savePath="", myVerbosity = rc.eventVerbose)
      g4Command = f"cd {pBuild}; ./{rc.execName} {macName}"
      geantCommandList.append(g4Command)
  ## done loops on physics pars and building command list
  
  # Add dummy commands to absorb NSApplication warnings from worker process startup
  print("------------------------------------------------------------")
  nProc = 4
  dummy_commands = [f"echo 'Starting worker {i+1}'" for i in range(nProc+1)]
  geantCommandList = dummy_commands + geantCommandList
  
  ## now doing the commands
  try:
    with Pool(nProc) as pool:   # Create a pool of worker processes
      progress_bar = tqdm(total=len(geantCommandList), desc="Running G4 simulations")

      # Use pool.imap() to execute the function in parallel and update progress bar
      for result in pool.imap(run_process, geantCommandList):
        progress_bar.update(1)
        stringAfter_batchOutput = result.split(pBatch)[-1]
        progress_bar.set_postfix_str(f"Completed: {stringAfter_batchOutput}")  # Updates inline

      # Close the progress bar
      progress_bar.close()
      
  except KeyboardInterrupt:
    print("\n⚠️  Interrupted by user (Ctrl+C)")
    pool.terminate()  # Forcefully terminate worker processes
    pool.join()       # Wait for termination
    progress_bar.close()
    sys.exit(1)
  except Exception as e:
    print(f"\n❌ Error during execution: {e}")
    pool.terminate()
    pool.join()
    progress_bar.close()
    sys.exit(1)

  # pool = Pool(processes = 4)
  # results = pool.imap_unordered(run_process, geantCommandList)
  # list(results)

  print("\n\n\n\n\n\n")
  print("----------------------------------------------------------------")
  print("-------- Done running G4 over paremeter space --------")
  print("----------------------------------------------------------------")