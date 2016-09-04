@echo off

set pwd=%cd%
set base=%~dp0
call :fix_link bin\shaders noise
call :fix_link lib glm
cd %pwd%
exit /B

:fix_link
	cd %base%
	set /p dest=<%base%%1\%2
	cd %base%%1
	del %2
	mklink /J %2 %dest:/=\%
