# DYHighMass
DY high mass differential cross section measurement tool

## How to install
```sh
# install analyzer
git clone https://github.com/kyHwangs/DYHighMass.git
cd DYHighMass
source envset.sh
mkdir install build
cd build
cmake ../ -DCMAKE_INSTALL_PREFIX=../install
make -j4 install

# install plotter
cd DYHighMass
source envset.sh

pip3 install --user uproot pickle uuid cmsstyle array # only need first time
chmod +x ./plotter/* # only need first time
```

## How to submit jobs
```sh
mkdir DYHighMass/Batch
cd Batch
generate_script --base <baseDir> --channel <channel>
cd <baseDir>
condor_submit condor_submit.sub
```

`<baseDir>`: base workspace name

`<channel>`: channel to analysis **(ALLOWED: MUMU, EMU, EE)**

## How to make plots
When the condor jobs are done, you should merge all of them and check the status of jobs.
```sh
hadd -j 15 output.root ./ROOT/*.root # taks ~ 10 min, it should be "output.root"!
(or you can submit jobs with `condor_submit hadd_submit.sub`)
job_summary --channel <channel>
```
When the jobs are done without any problem, no warning sign appears.
Then, excute below command,
```sh
plotter_MUMU.py --era <era> # allowed era: 2016_preVFP, 2016_postVFP, 2017 and 2018

or

plotter_EMU.py -- era <era>
plotter_EE.py --era <era>
```




