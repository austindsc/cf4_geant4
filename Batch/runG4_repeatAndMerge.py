import sys
import os
import subprocess # to call mkdir and build .mac files we need
from multiprocessing import Pool
from tqdm import tqdm

import ROOT

from helpers.G4_runClasses import *
from helpers.G4_macHelpers import *
from helpers.FileGetters import *

path = "/Users/austindestcroix/Desktop/school_holders/projects/sbc/geant4_cf4/"
# pBatch = path+"batchOutput/"
pBatch = path+"batchOutput2_oct16/"
pBuild = path+"build/"

global VERBOSE # unneeded but cute
VERBOSE = False

# should improve the wrapping or ownership of the 'build_from_mac_items" function
def parseStringMyWay(sin, first = "exampleB1", second="geant4_cf4"):
  ind = sin.find("exampleB1 ")
  newOut = sin[ind+len("exampleB1"):]
  ind = newOut.find("geant4_cf4")
  return newOut[ind+len("geant4_cf4"):]

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
  return parseStringMyWay(command)

# def deleteV2files(macName):


# def deleteFile(fname, ending):
#     # Check if the file name ends with the specified ending
#     if fname.endswith(ending):
#         try:
#             # Use subprocess to execute the bash command to delete the file
#             subprocess.run(["rm", fname])
#             print(f"File {fname} deleted.")
#         except subprocess.CalledProcessError as e:
#             print(f"Error deleting {fname}: {e}")
#     else:
#         print(f"File {fname} does not end with '{ending}'.")

def getLastFolder(f1):
  folder = os.path.dirname(f1)
  folder_name = os.path.basename(folder)
  return str(folder_name)

def mergeTTrees(macName, macName2):
  fname1 = macName.rstrip(".mac")
  fname2 = macName2.rstrip(".mac")
  # basePath = os.path.dirname(macName)
  outname = fname1 + "_MERGED"+".root"

  # Open the input files
  file1 = ROOT.TFile(fname1 + ".root", "READ")
  file2 = ROOT.TFile(fname2 + ".root", "READ")
  output_file = ROOT.TFile(outname, "RECREATE")

  # Get the TTrees from the input files
  tree1 = file1.Get("simOutput")
  tree2 = file2.Get("simOutput")
  tlist = ROOT.TList()
  tlist.Add(tree1)
  tlist.Add(tree2)
  merged_tree = ROOT.TTree.MergeTrees(tlist)

  # Create an output file
  output_file.cd()
  merged_tree.Write()
  output_file.Close()

  # # Clone the TTrees into the output file
  # merged_tree1 = tree1.CloneTree()
  # merged_tree2 = tree2.CloneTree()

  # Write the merged TTrees to the output file
  # merged_tree2.Write()

  # Close the input and output files
  file1.Close()
  file2.Close()
  return outname


argsToRun = ["/Users/austindestcroix/Desktop/school_holders/projects/sbc/geant4_cf4/batchOutput2_oct16/0/Co57_ly_2000_h_1d5.mac", 
"/Users/austindestcroix/Desktop/school_holders/projects/sbc/geant4_cf4/batchOutput2_oct16/0/Co57_ly_2000_h_4d0.mac", 
"/Users/austindestcroix/Desktop/school_holders/projects/sbc/geant4_cf4/batchOutput2_oct16/1/Co57_ly_2500_h_1d5.mac", 
"/Users/austindestcroix/Desktop/school_holders/projects/sbc/geant4_cf4/batchOutput2_oct16/1/Co57_ly_2500_h_4d0.mac", 
"/Users/austindestcroix/Desktop/school_holders/projects/sbc/geant4_cf4/batchOutput2_oct16/2/Co57_ly_3000_h_1d5.mac", 
"/Users/austindestcroix/Desktop/school_holders/projects/sbc/geant4_cf4/batchOutput2_oct16/2/Co57_ly_3000_h_4d0.mac", 
"/Users/austindestcroix/Desktop/school_holders/projects/sbc/geant4_cf4/batchOutput2_oct16/3/Co57_ly_3500_h_1d5.mac", 
"/Users/austindestcroix/Desktop/school_holders/projects/sbc/geant4_cf4/batchOutput2_oct16/4/Co57_ly_4000_h_1d5.mac", 
"/Users/austindestcroix/Desktop/school_holders/projects/sbc/geant4_cf4/batchOutput2_oct16/4/Co57_ly_4000_h_4d0.mac", 
"/Users/austindestcroix/Desktop/school_holders/projects/sbc/geant4_cf4/batchOutput2_oct16/5/Co57_ly_4500_h_1d5.mac", 
"/Users/austindestcroix/Desktop/school_holders/projects/sbc/geant4_cf4/batchOutput2_oct16/6/Co57_ly_5000_h_1d5.mac"]

# argsToRun = argsToRun[0:2]
fnames = []
geantCommandList = []
scaleNBEAM = 2

if __name__ == "__main__":

  for command in argsToRun:
    runPars = RunParameters(macPath=command)
    fname1 = runPars.fname # same as command with rstrip(".mac")
    fname2 = runPars.fname+ "_V2"
    fnames.append([fname1, fname2])

    runPars.fname = fname2
    runPars.Nbeam = int(runPars.Nbeam * scaleNBEAM)

    physPars = PhysicsParameters(macPath=command)
    detPars = DetectorParameters(macPath=command)

    # print(runPars.fname)
    # print(runPars.to_Mac_string())

    # generate the new mac file
    macName = build_mac_from_items(physPar=physPars, detPar=detPars, runPar=runPars, fname = runPars.fname, savePath="")

    # # it works with savepath empty. all info is in the runPars.fname member
    # # runPars.fname also has no suffix
    g4Command = f"cd {pBuild}; ./exampleB1 {macName}"
    geantCommandList.append(g4Command)
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

  ## do the merging
  for iCommand, (f1, f2) in enumerate(fnames):
    # print(iCommand, f1, f2)
    mergedName = mergeTTrees(f1, f2) # now have a new file with a bad name
    # exit(0)

    # need to delete f1, delete f2, rename file to f1 name
    pFolder = pBatch + getLastFolder(f1+".mac")
    command = f"cd {pFolder}; mv {f1}.root {f1}.rootFIRST; mv {f2}.root {f2}.rootSECOND"
    # print(command)
    subprocess.run(command, shell = True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)
    
    runPars1 = RunParameters(macPath=f1+".mac")
    detPars1 = DetectorParameters(macPath=f1+".mac")
    physPars1 = PhysicsParameters(macPath=f1+".mac")

    runPars2 = RunParameters(macPath=f2+".mac")

    # modifying the mac file to keep info consistent
    runPars1.Nbeam += runPars2.Nbeam
    runPars1.fname = mergedName.rstrip(".root")
    print(runPars1.fname)
    build_mac_from_items(physPar=physPars1, detPar=detPars1, runPar=runPars1, fname = runPars1.fname, savePath="")
    
    command = f"cd {pFolder}; mv {f1}.mac {f1}.macFIRST; mv {f2}.mac {f2}.macSECOND"
    # print(command)
    subprocess.run(command, shell = True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)


    # change Nbeam of the original mac file
    # delete mac file 2

  print("\n")
  print("----------------------------------------------------------------")
  print("-------- Done running G4 over paremeter space --------")
  print("----------------------------------------------------------------")
   