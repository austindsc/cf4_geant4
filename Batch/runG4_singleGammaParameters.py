"""
run with python3.11 (where tqdm is installed)

run from wherever (bash calls cd to g4 directory)

this is the 'master' file to generate the first runs of the parameter space

the verbosities are NOT copied from the base macro file, they are done explicitly in G4_runClasses.py

1. pass the base macro and file associated
2. this makes new folders for each parameter set (starting after existing folders)

-- 
to run verbosely (see all g4 output in terminal), set rcGlobal.verbose = True
to log all that output:
p311 runG4_allAlpha.py > output.log 2>&1

The above will get big and crash for large runs.
to run quietly (no g4 output, just progress bar), set rcGlobal.verbose = False

to only save last 1000 lines (avoid crash), with rcGlobal.verbose = True:
p311 runG4_allAlpha.py 2>&1 | (tail -n 1000 > last1000.log; cat > /dev/null)
# p311 runG4_allAlpha.py 2>&1 | tail -n 1000 > last1000.log  # This waits until end, can crash
"""

# Suppress macOS secure coding warning
import os

import math
import sys
import subprocess # to call mkdir and build .mac files we need
from multiprocessing import Pool
import multiprocessing as mp

from tqdm import tqdm

from helpers.G4_runClasses import *
from helpers.G4_macHelpers import *

# import batchFunctions as BF

path = os.getenv("CF4G4")

pBuild = path+"build/"
# pBatch = path+"batchOutput/gammaBare_may22_nominal/"
# pBatch = path+"batchOutput/gammaLP_refined_lowDensity_june11/"
# pBatch = path+"2batchOutput/gammaLP_jun12_128K/"

# pBatch = path+"2batchOutput/gammaBare_june17_145K/"
pBatch = path+"2batchOutput/gammaLP_june17_145K/"

# pBatch = path+"batchOutput/gammaMods/runB_shiny/"
# pBuild = path+"buildHighRef/"

## -- controls -- ###
# Import RunControl class (or you could define it here)
class RunControl:
  pass
rcGlobal = RunControl()

rcGlobal.verbose = False  # Simple: suppress G4 output, show only progress bar
rcGlobal.eventVerbose = 0 # 0 is silent, 1 is a little, 2 is a lot
rcGlobal.execName = "cf4Chamber"

## gamma run parameters
rcGlobal.lyList = np.round([2.25, 2.6], 2) # avoid weird float things // for shiny test
# rcGlobal.lyList = np.round([0.75, 1.0, 1.25, 1.5, 2], 2) # avoid weird float things
# rcGlobal.lyList = np.round([1.1], 2) # avoid weird float things

## fixed vis LY (from previous LP analysis).
rcGlobal.otherLY = 0
rcGlobal.onlyVis = True

rcGlobal.nbeam_end = 2.0e6  # gammas of maximum ly value
rcGlobal.coFactor = 4 # multiply Co57 nbeam by this factor to get Cs137 nbeam
rcGlobal.heightScale = 4 # multiply 6cm job by this
shiftHeight = 32.0
voltage = 4.3
skipG4command = False

roundingN = 10000
Func_ScaleN = lambda ly:  int(math.ceil(rcGlobal.nbeam_end * (0.5 + 0.5*(max(rcGlobal.lyList) + rcGlobal.otherLY) / ly)/ roundingN)) * roundingN  # scale nbeam with ly

## loops are
#1 ly loop
#2 source loop (base macro loop)
#3 source specific height loop
#4 LP or bare
#5 voltage loop (optional)
 
## experimental setup
rc1 = RunControl()
rc1.voltageList = np.round([voltage], 2) # avoid weird float things
rc1.baseMacro = "gammaCo57_base"
rc1.doLP = True
rc1.doBare = False
rc1Heights = [30.5, 28.5, 26.] # LP
# rc1Heights = [30.5, 28.5, 27.]
# rc1Heights = [28, 30.5, 33] # for Co Bare
rc1.heightList = [round(shiftHeight - h, 2) for h in rc1Heights] # avoid weird float things

rc2 = RunControl()
rc2.voltageList = np.round([voltage], 2) # avoid weird float things
rc2.baseMacro = "gammaCs137_base"
rc2.doLP = True
rc2.doBare = False
rc2Heights = [30.5, 28.5, 26.0] # for LP
# rc2Heights = [30.5, 28.5, 27.]
# rc2Heights = [26.5, 28, 30.5, 32] # for Cs Bare
rc2.heightList = [round(shiftHeight - h, 2) for h in rc2Heights] # avoid weird float things

rcList = [rc1, rc2]
# rcList = [rc2]

if not all([rc.doLP or rc.doBare for rc in rcList]):
  print("Error: At least one of doLP or doBare must be True in each of the RunControl instances.")
  sys.exit(1)

## -- Done Controls -- ##
def run_process_quiet(command):
  subprocess.run(command, shell = True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
  return command

def run_process_verbose(command):
  subprocess.run(command, shell = True)
  return command

if rcGlobal.verbose:
  run_process = run_process_verbose
else:
  run_process = run_process_quiet


if __name__ == "__main__":
  
  geantCommandList = [] # list of things that should work as long as in G4build directory

  ## getting the list of folders that already exist
  all_items = os.listdir(pBatch)
  folders = [int(item) for item in all_items if os.path.isdir(os.path.join(pBatch, item))]
  lastFolder = 0
  if len(folders) > 0:
    folders = sorted(folders, key=lambda x: x)
    lastFolder = max(folders) + 1

  # make new folders, loop on LY
  for iID, ly in enumerate(rcGlobal.lyList): ## ly loop
    
    folderID = iID + lastFolder
    # make a uniquely named directory (in lowest for loop)
    subprocess.run(f"cd {pBatch} ; mkdir {folderID}", shell = True)
    parDirID = str(folderID)
    
    for rc in rcList: ## gamma source loop    
      ## building specific macro!
      physPars = PhysicsParameters(macPath=rc.baseMacro + ".mac")
      detPars = DetectorParameters(macPath = rc.baseMacro + ".mac")
      runPars = RunParameters(macPath= rc.baseMacro + ".mac")
      
      ## run physics parameters
      # runPars.Nbeam = Func_ScaleN(ly)
      
      ## ly here is VUV ly
      totalLY = rcGlobal.otherLY + ly
      physPars.ly = totalLY # total ly (Ly here is VUV)
      localNbeam = Func_ScaleN(totalLY)
      
      if rc == rc1 and "co" in rc.baseMacro.lower(): # this is Co loop
        localNbeam = int(localNbeam * rcGlobal.coFactor) # scale up co nbeam
      
      print(f"Total LY for this run: {totalLY}, nbeam: {runPars.Nbeam}")
      if rcGlobal.onlyVis:
        physPars.vf = 1.0
      else:
        physPars.vf = np.round(rcGlobal.otherLY / totalLY, 2) # vf is set by the total ly and the LP ly (which is fixed by previous analysis)
      
      
      for voltage in rc.voltageList: ## voltage loop
        # detPars.voltage = voltage # set the detector parameters ## this is wrong, set voltage is implemented in RunParameters
        runPars.voltage = voltage
        
        for h in rc.heightList: ## height loop
          runPars.gammaHeight = h # set the run parameters
          hstr = str(h).replace(".", "d")
          vstr = str(voltage).replace(".", "d")
          
          if h > 4.5:
            runPars.Nbeam = int(localNbeam * rcGlobal.heightScale)
          else:
            runPars.Nbeam = localNbeam
        
          #### bare run
          if rc.doBare:
            runPars.lpFilter = False
            runPars.fname = pBatch + f"{parDirID}/" + rc.baseMacro + "_bare_" + vstr + "V" + "_h" + hstr
            # it works with savepath empty. all info is in the fname member
            macName = build_mac_from_items(physPar=physPars, detPar=detPars, runPar=runPars, 
                                fname = runPars.fname, savePath="", myVerbosity = rcGlobal.eventVerbose)
            print("fname to print:", runPars.fname) if skipG4command else None
            g4Command = f"cd {pBuild}; ./{rcGlobal.execName} {macName}"
            geantCommandList.append(g4Command)
          
          #### LP run
          if rc.doLP:
            runPars.lpFilter = True
            runPars.fname = pBatch + f"{parDirID}/" + rc.baseMacro + "_LP_" + vstr + "V" + "_h" + hstr
            macName = build_mac_from_items(physPar=physPars, detPar=detPars, runPar=runPars, 
                                fname = runPars.fname, savePath="", myVerbosity = rcGlobal.eventVerbose)
            print("fname to print:", runPars.fname) if skipG4command else None
            g4Command = f"cd {pBuild}; ./{rcGlobal.execName} {macName}"
            geantCommandList.append(g4Command)
  ## done loops on physics pars and building command list
  
  print("\n\n")
  [print(cmd) for cmd in geantCommandList]
  
  if skipG4command:
    print("Skipping G4 command execution as per configuration.")
    sys.exit(0)
  
  #### below is purely comp sci stuff ####
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