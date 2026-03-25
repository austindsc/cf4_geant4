"""
Simple rerun script for invalid ROOT files.

Loops through all .root files in the folder below,
checks if they have a 'simOutput' tree, deletes invalid ones, and reruns.
"""

import os
import sys
import glob
import subprocess
from multiprocessing import Pool
from tqdm import tqdm

import uproot

# -- Configuration --
path = os.getenv("CF4G4")
pBuild = path + "build/"
execName = "cf4Chamber"
verbose = False

# SET THIS TO THE FOLDER TO CHECK
folder = path + "batchOutput/gammaLP_Jan26_moreStats/"


def check_root_valid(rootPath):
    """Check if a ROOT file has the 'simOutput' tree."""
    try:
        f = uproot.open(rootPath)
        tree = f["simOutput"]
        return True
    except:
        return False


def run_process(cmd):
    """Run a G4 command."""
    if verbose:
        subprocess.run(cmd, shell=True)
    else:
        subprocess.run(cmd, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return cmd


def main():
    
    # Find all .root files recursively
    rootFiles = glob.glob(os.path.join(folder, "**/*.root"), recursive=True)
    print(f"Found {len(rootFiles)} .root files in {folder}")
    
    # Check each file for validity
    invalidFiles = []
    print("Checking files for 'simOutput' tree...")
    for rf in tqdm(rootFiles, desc="Checking"):
        if not check_root_valid(rf):
            invalidFiles.append(rf)
    
    print(f"\nFound {len(invalidFiles)} invalid files out of {len(rootFiles)}")
    
    if len(invalidFiles) == 0:
        print("All files are valid! Nothing to rerun.")
        sys.exit(0)
        
    # Show invalid files
    print("\nInvalid files:")
    for f in invalidFiles:
        print(f"  {f}")
    # return
    
    # Build command list
    geantCommandList = []
    for rootPath in invalidFiles:
        macPath = rootPath.replace(".root", ".mac")
        if not os.path.exists(macPath):
            print(f"  Warning: macro not found for {rootPath}, skipping")
            continue
        g4Command = f"cd {pBuild}; ./{execName} {macPath}"
        geantCommandList.append((rootPath, g4Command))
    
    if len(geantCommandList) == 0:
        print("\nNo macros found to rerun.")
        sys.exit(0)
    
    print(f"\nWill delete {len(geantCommandList)} invalid .root files and rerun...")
    response = input("Proceed? [y/N]: ")
    if response.lower() != 'y':
        print("Aborted.")
        sys.exit(0)
    
    # Delete invalid root files
    print("Deleting invalid .root files...")
    for rootPath, _ in geantCommandList:
        os.remove(rootPath)
        print(f"  Deleted: {rootPath}")
    
    # Run commands
    commands_only = [cmd for _, cmd in geantCommandList]
    nProc = 4
    nReal = len(commands_only)
    dummy_commands = [f"echo 'Starting worker {i+1}'" for i in range(nProc + 1)]
    all_commands = dummy_commands + commands_only
    
    try:
        with Pool(nProc) as pool:
            progress_bar = tqdm(total=nReal, desc="Rerunning simulations")
            
            for result in pool.imap(run_process, all_commands):
                if ".mac" in result:
                    progress_bar.update(1)
                    fname = result.split("/")[-1]
                    progress_bar.set_postfix_str(f"Completed: {fname}")
            
            progress_bar.close()
            
    except KeyboardInterrupt:
        print("\n⚠️  Interrupted by user (Ctrl+C)")
        pool.terminate()
        pool.join()
        progress_bar.close()
        sys.exit(1)
    except Exception as e:
        print(f"\n❌ Error during execution: {e}")
        pool.terminate()
        pool.join()
        progress_bar.close()
        sys.exit(1)
    
    print("\n✅ Done rerunning invalid files!")


if __name__ == "__main__":
    main()
