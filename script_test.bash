#!/bin/bash

./waf --run "scratch/manet-routing-compare --pause=2 --nSinks=60 --CSVfileName="test2""



./waf --run "scratch/manet-routing-compare --pause=4 --nSinks=60 --CSVfileName="test5""



./waf --run "scratch/manet-routing-compare --pause=6 --nSinks=60 --CSVfileName="test8""


./waf --run "scratch/manet-routing-compare --pause=2 --nSinks=20 --CSVfileName="test0""

./waf --run "scratch/manet-routing-compare --pause=2 --nSinks=40 --CSVfileName="test1""

./waf --run "scratch/manet-routing-compare --pause=4 --nSinks=20 --CSVfileName="test3""

./waf --run "scratch/manet-routing-compare --pause=4 --nSinks=40 --CSVfileName="test4""

./waf --run "scratch/manet-routing-compare --pause=6 --nSinks=20 --CSVfileName="test6""

./waf --run "scratch/manet-routing-compare --pause=6 --nSinks=40 --CSVfileName="test7""
