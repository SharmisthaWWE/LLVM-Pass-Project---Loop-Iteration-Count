# LLVM Pass Project - Loop Iteration Count
In LLVM different transform and analysis passes are implemented to perform different analysis and optimization tasks. However, this
passes requires some basic information about the program to perform an optimization. For example, the presence of loop in a program, 
number of loop iteration, number of opcode, etc. In this project, a new pass is designed and implemented to profile the number of 
loop iteration for each loop in a program. While calculating the number of loop iteration, the sub loops of each parent loop is 
also considered. Therefore, a complete set of information about the loops of a program can be obtained. Loop Iteration Count (LIC)
is a LLVM pass that profile the number of loops including subloop and loop trip count for all the loops in a program. Moreover, it
also profile the function name that conatins those loops.

In order to run the program, follow the steps below,
1) Install LLVM 5.0 with the corresponding clang support.
2) Create a folder named "LICPass" under llvm/projects/
3) Clone this project in llvm/projects/LICPass
4) Go to "build" directory
5) Generate Makefile using command "$ cmake -G "Unix Makefiles" ../llvm"
6) Then run "$ make"
7) Go to the directory that contains .bc file on which the pass will be applied
8) Run command "$ opt -load LICPass.so -LoopCount -disable-output file.bc"



# Disclaimer
The sourcecode does not come with any kind of warrenty or support, though you can contact me if you need.

# Copyright
Sharmistha Bardhan
