#!/usr/local/bin/python3

import subprocess
import sys

from datetime import date

# def run_process(command):
# 	subprocess.run(command, shell = True)

path = "/Users/austindestcroix/Desktop/school_holders/projects/sbc/geant4_cf4/"
pBatch = path+"Batch/alphas_v1_noQuenching/"
pBuild = path+"build_myB1/"

VERBOSITY = 0

stock_verbosities = """
/vis/verbose {0}
/tracking/verbose {0}
/event/verbose {0}
/run/verbose {0}

/process/em/verbose {0}
/process/had/deex/verbose {0}
/particle/verbose {0}
/tracking/verbose {0}
""".format(VERBOSITY)


if __name__ == "__main__":
    print("""------------------runIt------------------\n""")
    
    medium = "gCF4"
    # density = 0.002     #in g/cm3
    densityList = [0.002, 0.01, 0.02, 0.03, 0.04, 0.06]
    # to repeat from 0.0125 onwards
    height = 0.5        #in cm
    ly = 5000        #max value at 5 bar, room temp, per MeV
    Nrun = 10000
        
    # output file list
    outputList = open(pBatch+"fileList-"+str(date.today())+".txt", 'w')
    for rho in densityList:
        density = rho
        
        outputName = "h_"+str(height)+"-density_"+str(density)+"-LY_"+str(ly)
        
        # writing to output list
        outputList.write(outputName+".root \n")
        
        ## controlling strings
        s_verbosities = stock_verbosities.format(VERBOSITY)
        s_myVerbosity = ("/setDetector/myVerbosity %i" % 1)
        s_medium = "/setMaterial/detectionMedium %s" % medium
        s_mRho = "/setMaterial/mediumDensity %f g/cm3" % rho
        s_ly = "/setMaterial/mediumLY %i MeVLY" % ly
        s_height = "/setDetector/alphaHeight %f" % height
        
        # each macro file
        macro = open(pBatch+outputName+".mac", 'w')
        macro.write(s_verbosities + "\n") # zero for verbosity
        macro.write(s_medium + "\n")
        macro.write(s_mRho + "\n")
        macro.write(s_ly + "\n")
        
        macro.write(s_myVerbosity + "\n")
        macro.write(s_height + "\n\n")
        
        macro.write("/run/initialize" + "\n")
        macro.write("/analysis/setFileName %s \n" % (pBatch+outputName+".root"))
        macro.write("/run/beamOn %i" % Nrun + "\n\n")

        macro.close()
        
        # 1. must run geant4 from its build directory (or change how it finds data files)
        # 2. must get root file and macro saved to correct place
        # 3. add all output file names to list --> add to tchain in one line in analysis
            
        g4Command = "cd %s ; ./exampleB1 %s" % (pBuild, pBatch + outputName+".mac")
        # print(g4Command)
        print("Running", outputName)
        subprocess.run(g4Command, shell = True)
    
    outputList.close()
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    print("\n\nDONE!")