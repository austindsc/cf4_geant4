import sys
import subprocess # to call mkdir and build .mac files we need
from multiprocessing import Pool
from tqdm import tqdm

from helpers.G4_runClasses import *
from helpers.G4_macHelpers import *
from helpers.FileGetters import *

path = "/Users/austindestcroix/Desktop/school_holders/projects/sbc/geant4_cf4/"
# pBatch = path+"batchOutput/"
pBatch = path+"batchOutput/"
pBuild = path+"build/"
# VERBOSE = False
global VERBOSE # unneeded but cute
VERBOSE = False


# why not just put the parameter ID in the mac file as a comment!?
# don't need unique directories, but i like that for organization
# will make another code that builds the 'master' csv to track parameter space and assign ID

# how can I use this to easily load in the data?

# should improve the wrapping or ownership of the 'build_from_mac_items" function

def run_process(command, verbose = VERBOSE):
  if verbose == False:
    subprocess.run(command, shell = True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)
  else:
    subprocess.run(command, shell = True)
  # mac = somehow build macro file from info
  # or pass a 2d array that holds physPar/runPar and the g4 command
  # or change this command to be more 'complex', taking in the mac object, and
  # building the G4 command here. Then I can have this return whatever info I need
  # to have a nice printed output
  return 69

if __name__ == "__main__":

  baseMacro = "gammaCo57_base.mac"
  # baseMacro = "gammaCs137_base.mac"

  ## build the base objects to run off
  physPars = PhysicsParameters(macPath=baseMacro)

  # print(physPars.ly)
  # exit(0)
  # print(physPars.to_Mac_string())
  detPars = DetectorParameters(macPath = baseMacro)
  # print(detPars.to_Mac_string())
  runPars = RunParameters(macPath=baseMacro)
  # print(runPars.to_Mac_string())

  # arranging runs and parameter space
  # heights = {-0.5, 1.5, 4}
  # heights = {0.0, 1.5, 4.0, 5.5}
  heights = {1.5}
  # lys = np.arange(1100, 13000, 1000)
  # lys = [11000, 12000, 13000]
  NBEAM = 15000000
  geantCommandList = [] # list of things that should work as long as in G4build directory

  ## getting the list of folders that already exist
  all_items = os.listdir(pBatch)
  folders = [int(item) for item in all_items if os.path.isdir(os.path.join(pBatch, item))]
  folders = sorted(folders, key=lambda x: x)
  # lastFolder = max(folders)
  # folders = folders[:10]
  # folders = [0, 1, 2, 3] # overwrite and only fill a few folders
  # print(folders[4:])
  # exit(0)

  for f in folders: # this loops on LY right now
    print(f)
    macs = get_all_mac_files(pBatch+str(f)+ "/")
    # print("folder %s" % f)
    # print(macs[0])
    physPars = PhysicsParameters(macPath = macs[0])
    print(physPars.ly)
    
    # make a uniquely named directory
    # subprocess.run(f"cd {pBatch} ; mkdir {folderID}", shell = True)
    # parDirID = str(folderID)
    # control parameter ID here
    
    for h in heights:

      # set the run parameters
      runPars.gammaHeight = h

      # this is the important line for file naming and location
      hstr = str(h).replace(".", "d")
      runPars.fname = pBatch + f"{f}/" + "Co57_ly_"+str(int(float(physPars.ly)))+"_h_"+hstr

      runPars.Nbeam = NBEAM
      print(runPars.fname)
      # it works with savepath empty. all info is in the runPars.fname member
      # runPars.fname also has no suffix
      macName = build_mac_from_items(physPar=physPars, detPar=detPars, runPar=runPars, 
                            fname = runPars.fname, savePath="")

      g4Command = f"cd {pBuild}; ./exampleB1 {macName}"
      geantCommandList.append(g4Command)

  #     # print(g4Command)
  #     # subprocess.run(g4Command, shell=True)
  
  # # done paramter space (nested) forloops
  # # done building G4 command list
  
  #### executing G4 commands ####
  nProc = 4
  # Create a pool of worker processes
  with Pool(nProc) as pool:
      # Create a tqdm progress bar
      progress_bar = tqdm(total=len(geantCommandList))

      # Use pool.imap() to execute the function in parallel
      # Update the progress bar after each task completion
      for result in pool.imap(run_process, geantCommandList):
          # print("") # put something here to write about what physics run was done
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