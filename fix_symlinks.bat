@echo off

set pwd=%cd%
set base=%~dp0
call :fix_link bin\shaders noise
call :fix_link lib glm
call :fix_link lib zstr
cd %pwd%
exit /B

:fix_link
	cd %base%
	set /p dest=<%1\%2
	cd %1
	del %2
	mklink /J %2 %dest:/=\%
