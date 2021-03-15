  #!/usr/bin/env python
import sys
import os
import os.path
import shutil
############################################
# Directory Structure:
# Morpher Home:
#     -Morpher_DFG_Generator
#     -Morpher_CGRA_Mapper
#     -hycube_simulator
#     -Morpher_Scripts

# Build all three tools before running this script

def main():

  if not 'MORPHER_HOME' in os.environ:
    raise Exception('Set MORPHER_HOME directory as an environment variable (Ex: export MORPHER_HOME=/home/dmd/Workplace/Morphor/github_ecolab_repos)')

  MORPHER_HOME = os.getenv('MORPHER_HOME')
  DFG_GEN_HOME = MORPHER_HOME + '/Morpher_DFG_Generator'


##################################array_add############################################################################################################

  DFG_GEN_KERNEL = DFG_GEN_HOME + '/applications/array_add/'

  my_mkdir(DFG_GEN_KERNEL)

  print('\nRunning Morpher_DFG_Generator\n')
  os.chdir(DFG_GEN_KERNEL)

  print('\nGenerating DFG\n')
  os.system('./run_pass.sh array_add')
  os.system('dot -Tpdf array_add_INNERMOST_LN1_PartPredDFG.dot -o array_add_INNERMOST_LN1_PartPredDFG.pdf')

  MEM_TRACE = DFG_GEN_KERNEL + '/memtraces'

  my_mkdir(MEM_TRACE)

  print('\nGenerating Data Memory Content\n')
  os.system('./final')
  
##################################array_add############################################################################################################


##################################pedometer############################################################################################################

  DFG_GEN_KERNEL = DFG_GEN_HOME + '/applications/pedometer/'

  my_mkdir(DFG_GEN_KERNEL)

  print('\nRunning Morpher_DFG_Generator\n')
  os.chdir(DFG_GEN_KERNEL)

  print('\nGenerating DFG\n')
  os.system('./run_pass.sh pedometer')
  os.system('dot -Tpdf pedometer_INNERMOST_LN1_PartPredDFG.dot -o pedometer_INNERMOST_LN1_PartPredDFG.pdf')

  MEM_TRACE = DFG_GEN_KERNEL + '/memtraces'

  my_mkdir(MEM_TRACE)

  print('\nGenerating Data Memory Content\n')
  os.system('./final')

##################################pedometer############################################################################################################

##################################gemm############################################################################################################

  DFG_GEN_KERNEL = DFG_GEN_HOME + '/applications/gemm/'

  my_mkdir(DFG_GEN_KERNEL)

  print('\nRunning Morpher_DFG_Generator\n')
  os.chdir(DFG_GEN_KERNEL)

  print('\nGenerating DFG\n')
  os.system('./run_pass.sh gemm')
  os.system('dot -Tpdf gemm_INNERMOST_LN111_PartPredDFG.dot -o gemm_INNERMOST_LN111_PartPredDFG.pdf')

  MEM_TRACE = DFG_GEN_KERNEL + '/memtraces'

  my_mkdir(MEM_TRACE)

  print('\nGenerating Data Memory Content\n')
  os.system('./final')

##################################gemm############################################################################################################

##################################fft############################################################################################################

  DFG_GEN_KERNEL = DFG_GEN_HOME + '/applications/hycube_v3_design_app_test/fft/fft_no_unroll/'

  my_mkdir(DFG_GEN_KERNEL)

  print('\nRunning Morpher_DFG_Generator\n')
  os.chdir(DFG_GEN_KERNEL)

  print('\nGenerating DFG\n')
  os.system('./run_pass.sh fix_fft')
  os.system('dot -Tpdf fix_fft_INNERMOST_LN111_PartPredDFG.dot -o fix_fft_INNERMOST_LN111_PartPredDFG.pdf')

  MEM_TRACE = DFG_GEN_KERNEL + '/memtraces'

  my_mkdir(MEM_TRACE)

  print('\nGenerating Data Memory Content\n')
  os.system('./final')

##################################fft############################################################################################################

def my_mkdir(dir):
    try:
        os.mkdir(dir)
    except:
        pass

if __name__ == '__main__':
  main()