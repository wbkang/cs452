#!/bin/bash

echo "Remote push"
	ssh pbakhila@linux024.student.cs.uwaterloo.ca  \
	"
cd cs452/kernel;
hg pull;
"	
