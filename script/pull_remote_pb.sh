#!/bin/bash

echo "Remote pull"
	ssh pbakhila@linux024.student.cs.uwaterloo.ca  \
	"
cd cs452/kernel;
hg pull;
"	
