@echo off

rem number of workers/slaves
set arg1=%1 
set /a arg = %arg1%+1

rem number of executions
set arg2=%2 

for /l %%x in (1,1,%arg2%) do (
	mpiexec -n %arg% "C:\Users\Beni\MBA-USP-DS\00.Projeto\01.mpi\source\repos\MatrixMultiMPI\Debug\MatrixMultiMPI.exe"
)