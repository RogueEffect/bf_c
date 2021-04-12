@echo off
set SDIR=src
set CC=gcc
set FILES=bf.c
set FLAGS=-g -Wall -std=c99
set EXE=out\bf.exe

echo Building %EXE%...
(cd %SDIR% && %CC% %FLAGS% %FILES% -o ..\%EXE% && echo Build complete) || echo Build failed
cd ..