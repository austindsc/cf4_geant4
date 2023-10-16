import os
import numpy as np
import uproot

from ROOT import TFile

# returns a list of file names in a directory (without suffixes)
def get_all_root_files(directory: "path to directory"):
  
  # Get all files in the directory
  files = os.listdir(directory)
  
  # Filter for .root files
  root_files = [directory+f for f in files if f.endswith('.root')]
  
  return root_files

def get_all_mac_files(directory: "path to directory"):
  
  # Get all files in the directory
  files = os.listdir(directory)
#   print(files)
  
  # Filter for .root files
  mac_files = [directory+f for f in files if f.endswith('.mac')]
  
  return mac_files

# open using uproot, returns ttree and file
def openMyROOT_2(sfile: str, treeName = "simOutput", verbose = 1):
    if os.path.exists(sfile+".root") and os.path.exists(sfile+".mac"):
        f = uproot.open(sfile+".root")
        if verbose:
            print("loaded in:", sfile+".root")
        return f, f[treeName]
    elif not os.path.exists(sfile+".root"):
        if verbose:
            print("does not exist:", sfile+".root")
        exit(1)
    elif not os.path.exists(sfile+".mac"):
        if verbose:
            print("does not exist:", sfile+".mac")
        exit(1)
    return None, None


# using pyroot, returns file f and ttree
def openMyROOT(sfile: str, treename = "simOutput"):
    if os.path.exists(sfile+".root") and os.path.exists(sfile+".mac"):
        f = TFile(sfile+".root", "READ")
        print("loaded in:", sfile+".root")
        return f, f.Get(treename)
    elif not os.path.exists(sfile+".root"):
        print("does not exist:", sfile+".root")
        return None, None
    elif not os.path.exists(sfile+".mac"):
        print("does not exist:", sfile+".mac")
        return None, None
    return None, None