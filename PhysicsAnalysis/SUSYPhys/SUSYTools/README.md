
================
SUSYTools
================

**Current Main Developers**: stefano.zambito@cern.ch, zach.marshall@cern.ch

------------------------------------

.. contents:: Table of contents


------------------------------------

------------------------------------
Recommended tags
------------------------------------

**Rel 21.0 Samples :**   Base,21.2.0 + newest SUSYTools

**Rel 20.7 Samples :**   Base,2.4.33 + SUSYTools-00-08-66

**Rel 20.1 Samples :**   Base,2.3.8 + SUSYTools-00-07-58


------------------------------------

------------------------
AnalysisBase / AthAnalysisBase Setup
------------------------

Because these two releases are becoming more similar, most of the instructions are the same.  Set up the latest recommended AnalysisBase release::

   setupATLAS
   asetup AnalysisBase,21.2.0

Or the latest AthAnalysis release::

   setupATLAS
   asetup AthAnalysis,21.2.0

For working with code, a sparse checkout is pretty straightforward::

   git atlas init-workdir https://:@gitlab.cern.ch:8443/atlas/athena.git

And then the version of SUSYTools in the release can be checked out via::

   cd athena
   git checkout 21.2.0
   git atlas addpkg SUSYTools

or the trunk version::

   cd athena
   git checkout master
   git atlas addpkg SUSYTools

Then, to compile::

   cd ..
   mkdir build
   cd build
   cmake ../athena/Projects/WorkDir
   gmake
   source x86_64-slc6-gcc62-opt/setup.sh

Additional packages needed on top of the recommended release are documented in ``doc/packages.txt``; for now you can add those to your work area via git atlas addpkg.  Afterwards, be sure to recompile everything::
   
   cmake ../athena/Projects/WorkDir
   gmake
   source x86_64-slc6-gcc62-opt/setup.sh

and you are ready to go!

## Still to add ##

Convenience script for getting packages we recommend -- at the moment, this is not something that is a part of a git workflow.

Testing
--------------

To run unit tests, simply go to your build area and type::

   gmake test

To test locally in an AnalysisBase release, get your favourite benchmark sample (e.g. mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.merge.DAOD_SUSY1.e3698_s2608_s2183_r7725_r7676_p2596/), and run::

   SUSYToolsTester <myAOD.pool.root> 100 isData=0 isAtlfast=0 Debug=0 NoSyst=0 2>&1 | tee log

The standard ``SUSYToolsTester`` code is meant to test the configuration of all the relevant CP tools and apply a minimal analysis flow. It includes the recalibration and correction of all physics objects, the recalculation of the MissingET, the extraction of SFs and other specific functionalities that are enabled based on the DxAOD stream. All systematic variations available in the central registry are tested by default, and a cutflow is displayed for each of them. This can be disabled by setting ``NoSyst=1``.

To test locally in an AthAnalysis release, run the test job options as::

   cd ..
   athena.py SUSYTools/minimalExampleJobOptions_mc.py
   
which is the athena-friendly equivalent of the ``SUSYToolsTester`` code above for running on MC.  You can also change "mc" to "data" or "atlfast" to run on data or fast simulation if you would prefer.

## Still to add ##

Unit tests for specific packages

------------------------------------

--------------------------------
Retrieving Cross sections
--------------------------------
All cross-sections are stored in text files in the directories like ``data/mc15_13TeV/``.
The ``Backgrounds.txt`` file contains all cross-sections per background sample (characterized by its dataset (DS) id).
Each signal grid has a corresponding text file with the cross-sections per grid point and per process (characterized by its dataset id and process id (finalState)).

A parser is provided with ``SUSYTools`` to access it::

   root [0] gSystem->Load("../build/x86_64-slc6-gcc49-opt/lib/libSUSYToolsLib.so")
   root [1] #include "SUSYTools/SUSYCrossSection.h"
   root [2] SUSY::CrossSectionDB myDB("SUSYTools/data/mc15_13TeV/");
   root [3] myDB.xsectTimesEff(410000)
   (const float)1.37378997802734375e+02
   root [4] myDB.xsectTimesEff(410000,0)
   (const float)1.37378997802734375e+02
   root [5] myDB.xsectTimesEff(164949, 111)
   (const float)8.08409880846738815e-04
   root [6] myDB.xsectTimesEff(164949, 1000022, 1000022)
   (const float)8.08409880846738815e-04


------------------------------------

