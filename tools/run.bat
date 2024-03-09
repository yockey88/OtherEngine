@echo off
for /f "tokens=1,* delims= " %%a in ("%*") do set ALL_BUT_FIRST=%%b
.\bin\%1\%2\%ALL_BUT_FIRST%
