"""
Run this script to fix corrupted runs. This script will search for .root files in the
batchOutput directory and check if they contain the "simOutput" tree. If not, the file
is added to a list of corrupted runs. The script then generates a list of G4 commands
to run over the corrupted runs. The G4 commands are executed in parallel using the 
multiprocessing library.
"""

import os
import glob
import subprocess
import uproot
import sys
import subprocess # to call mkdir and build .mac files we need
from multiprocessing import Pool
from tqdm import tqdm

TESTING = True
VERBOSERUN = True
EXECNAME = "cf4Chamber"
PATH = "/Users/austindestcroix/Desktop/school_holders/projects/sbc/g4_cf4_updated/"
PATHSAVE = PATH+"batchOutput_feb9_lowLY/"
PBUILD = PATH+"build/"

def parseStringMyWay(sin, first = "cf4", second="geant4_cf4"):
  ind = sin.find(first)
  newOut = sin[ind+len(first):]
  ind = newOut.find(second)
  return newOut[ind+len(second):]

def run_process(command, verbose = VERBOSERUN):
  if verbose == False:
    subprocess.run(command, shell = True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)
  else:
    subprocess.run(command, shell = True)
  return parseStringMyWay(command)


if __name__ == "__main__":
  # Define the directory to search for .root files
  root_directory = PATHSAVE
  
  # Get a list of all .root files (recursively)
  root_files = glob.glob(os.path.join(root_directory, "**/*.root"), recursive=True)

  failList = []
  for fname in root_files:
    file = uproot.open(fname)
    if "simOutput" not in file:
      failList.append(fname)

  geantCommandList = []
  for fname in failList:
    macName = fname.replace(".root", ".mac")
    g4Command = f"rm {fname}; cd {PBUILD}; ./{EXECNAME} {macName}"
    geantCommandList.append(g4Command)


  print("\n")
  print("failure list:")
  [print(fname) for fname in failList]
  print("command list")
  [print(command) for command in geantCommandList]
  
  if TESTING == True:
    print("Testing mode, printing failures:")  
    exit(0)

  #### executing G4 commands ####
  nProc = 4
  with Pool(nProc) as pool:   # Create a pool of worker processes
      progress_bar = tqdm(total=len(failList))       # Create a tqdm progress bar

      # Use pool.imap() to execute the function in parallel and update progress bar
      for result in pool.imap(run_process, geantCommandList):
          print(f"finished: {result}") # put something here to write about what physics run was done
          progress_bar.update(1)
      progress_bar.close()

  print("\n\n\n\n\n\n")
  print("----------------------------------------------------------------")
  print("-------- Done running G4 over paremeter space --------")
  print("----------------------------------------------------------------")