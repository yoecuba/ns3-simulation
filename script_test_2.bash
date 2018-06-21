!/bin/bash


./waf --run "scratch/energy-model-example --pause=2 --nWifis=20 --energyEnhance="false" --fileName="test_120_disable/simulation0"" > test_120_disable/log3.0.out 2>&1

./waf --run "scratch/energy-model-example --pause=2 --nWifis=40 --energyEnhance="false" --fileName="test_120_disable/simulation1"" > test_120_disable/log3.1.out 2>&1

./waf --run "scratch/energy-model-example --pause=2 --nWifis=60 --energyEnhance="false" --fileName="test_120_disable/simulation2"" > test_120_disable/log3.2.out 2>&1

./waf --run "scratch/energy-model-example --pause=4 --nWifis=20 --energyEnhance="false" --fileName="test_120_disable/simulation3"" > test_120_disable/log3.3.out 2>&1

./waf --run "scratch/energy-model-example --pause=4 --nWifis=40 --energyEnhance="false" --fileName="test_120_disable/simulation4"" > test_120_disable/log3.4.out 2>&1

./waf --run "scratch/energy-model-example --pause=4 --nWifis=60 --energyEnhance="false" --fileName="test_120_disable/simulation5"" > test_120_disable/log3.5.out 2>&1

./waf --run "scratch/energy-model-example --pause=6 --nWifis=20 --energyEnhance="false" --fileName="test_120_disable/simulation6"" > test_120_disable/log3.6.out 2>&1

./waf --run "scratch/energy-model-example --pause=6 --nWifis=40 --energyEnhance="false" --fileName="test_120_disable/simulation7"" > test_120_disable/log3.7.out 2>&1

./waf --run "scratch/energy-model-example --pause=6 --nWifis=60 --energyEnhance="false" --fileName="test_120_disable/simulation8"" > test_120_disable/log3.8.out 2>&1

./waf --run "scratch/energy-model-example --pause=2 --nWifis=20 --energyEnhance="true" --fileName="test_120_enable/simulation0"" > test_120_enable/log4.0.out 2>&1

./waf --run "scratch/energy-model-example --pause=2 --nWifis=40 --energyEnhance="true" --fileName="test_120_enable/simulation1"" > test_120_enable/log4.1.out 2>&1

./waf --run "scratch/energy-model-example --pause=2 --nWifis=60 --energyEnhance="true" --fileName="test_120_enable/simulation2"" > test_120_enable/log4.2.out 2>&1

./waf --run "scratch/energy-model-example --pause=4 --nWifis=20 --energyEnhance="true" --fileName="test_120_enable/simulation3"" > test_120_enable/log4.3.out 2>&1

./waf --run "scratch/energy-model-example --pause=4 --nWifis=40 --energyEnhance="true" --fileName="test_120_enable/simulation4"" > test_120_enable/log4.4.out 2>&1

./waf --run "scratch/energy-model-example --pause=4 --nWifis=60 --energyEnhance="true" --fileName="test_120_enable/simulation5"" > test_120_enable/log4.5.out 2>&1

./waf --run "scratch/energy-model-example --pause=6 --nWifis=20 --energyEnhance="true" --fileName="test_120_enable/simulation6"" > test_120_enable/log4.6.out 2>&1

./waf --run "scratch/energy-model-example --pause=6 --nWifis=40 --energyEnhance="true" --fileName="test_120_enable/simulation7"" > test_120_enable/log4.7.out 2>&1

./waf --run "scratch/energy-model-example --pause=6 --nWifis=60 --energyEnhance="true" --fileName="test_120_enable/simulation8"" > test_120_enable/log4.8.out 2>&1







