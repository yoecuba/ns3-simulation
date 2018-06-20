#!/bin/bash


./waf --run "scratch/energy-model-example --pause=2 --nWifis=20 --energyEnhance="true" --fileName="test_1/simulation0"" > test_1/log3.0.out 2>&1

./waf --run "scratch/energy-model-example --pause=2 --nWifis=40 --energyEnhance="true" --fileName="test_1/simulation1"" > test_1/log3.1.out 2>&1

./waf --run "scratch/energy-model-example --pause=2 --nWifis=60 --energyEnhance="true" --fileName="test_1/simulation2"" > test_1/log3.2.out 2>&1

./waf --run "scratch/energy-model-example --pause=4 --nWifis=20 --energyEnhance="true" --fileName="test_1/simulation3"" > test_1/log3.3.out 2>&1

./waf --run "scratch/energy-model-example --pause=4 --nWifis=40 --energyEnhance="true" --fileName="test_1/simulation4"" > test_1/log3.4.out 2>&1

./waf --run "scratch/energy-model-example --pause=4 --nWifis=60 --energyEnhance="true" --fileName="test_1/simulation5"" > test_1/log3.5.out 2>&1

./waf --run "scratch/energy-model-example --pause=6 --nWifis=20 --energyEnhance="true" --fileName="test_1/simulation6"" > test_1/log3.6.out 2>&1

./waf --run "scratch/energy-model-example --pause=6 --nWifis=40 --energyEnhance="true" --fileName="test_1/simulation7"" > test_1/log3.7.out 2>&1

./waf --run "scratch/energy-model-example --pause=6 --nWifis=60 --energyEnhance="true" --fileName="test_1/simulation8"" > test_1/log3.8.out 2>&1

#./waf --run "scratch/energy-model-example --pause=2 --nWifis=20 --fileName="test/simulation0"" > test/log4.0.out 2>&1

#./waf --run "scratch/energy-model-example --pause=2 --nWifis=40 --fileName="test/simulation1"" > test/log4.1.out 2>&1

#./waf --run "scratch/energy-model-example --pause=2 --nWifis=60 --fileName="test/simulation2"" > test/log4.2.out 2>&1

#./waf --run "scratch/energy-model-example --pause=4 --nWifis=20 --fileName="test/simulation3"" > test/log4.3.out 2>&1

#./waf --run "scratch/energy-model-example --pause=4 --nWifis=40 --fileName="test/simulation4"" > test/log4.4.out 2>&1

#./waf --run "scratch/energy-model-example --pause=4 --nWifis=60 --fileName="test/simulation5"" > test/log4.5.out 2>&1

#./waf --run "scratch/energy-model-example --pause=6 --nWifis=20 --fileName="test/simulation6"" > test/log4.6.out 2>&1

#./waf --run "scratch/energy-model-example --pause=6 --nWifis=40 --fileName="test/simulation7"" > test/log4.7.out 2>&1

#./waf --run "scratch/energy-model-example --pause=6 --nWifis=60 --fileName="test/simulation8"" > test/log4.8.out 2>&1







