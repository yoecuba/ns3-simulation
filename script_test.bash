#!/bin/bash

./waf --run "scratch/energy-model-example --pause=2 --nWifis=20 --CSVfileName="/testing/simulation0""

./waf --run "scratch/energy-model-example --pause=2 --nWifis=40 --CSVfileName="/testing/simulation1""

./waf --run "scratch/energy-model-example --pause=2 --nWifis=60 --CSVfileName="/testing/simulation2""

./waf --run "scratch/energy-model-example --pause=4 --nWifis=20 --CSVfileName="/testing/simulation3""

./waf --run "scratch/energy-model-example --pause=4 --nWifis=40 --CSVfileName="/testing/simulation4""

./waf --run "scratch/energy-model-example --pause=4 --nWifis=60 --CSVfileName="/testing/simulation5""

./waf --run "scratch/energy-model-example --pause=6 --nWifis=20 --CSVfileName="/testing/simulation6""

./waf --run "scratch/energy-model-example --pause=6 --nWifis=40 --CSVfileName="/testing/simulation7""

./waf --run "scratch/energy-model-example --pause=6 --nWifis=60 --CSVfileName="/testing/simulation8""


#4 con 1
#1 con 3
#6 con 4
#3 con 6


 
