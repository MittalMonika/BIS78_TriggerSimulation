#!/usr/bin/env python

# Test the reflex dictionary usage in PyROOT to
# instantiate and configure the overlap removal tool(s).

import sys
sys.argv.append('-b')

# Setup libs
from ROOT import gROOT
gROOT.Macro('$ROOTCOREDIR/scripts/load_packages.C')

# Import using the PyROOT bindings and reflex dictionary
from ROOT import ORUtils, OverlapRemovalTool

# Define some testing helper functions

def test_sc(statuscode):
    """Test a StatusCode and throw if failure"""
    if statuscode.isFailure():
        raise Exception('StatusCode failure!')

def test_tool_init(tool_type, verbose=False):
    """Performs simple construction and default initialization of a tool"""
    if verbose:
        print 'Initializing tool', tool_type.__name__
    tool = tool_type(tool_type.__name__)
    test_sc(tool.initialize())
    if verbose:
        print '  => Success'
    return tool

# Configure the legacy tool
orToolLegacy = OverlapRemovalTool('OverlapRemovalToolLegacy')

test_sc( orToolLegacy.setProperty('OverlapLabel', 'overlaps') )
test_sc( orToolLegacy.setProperty(bool)('LinkOverlapObjects', True) )
# Initialize the tool
test_sc( orToolLegacy.initialize() )

# Configure the new master tool
orTool = ORUtils.OverlapRemovalTool('OverlapRemovalTool')

# Try initialization some overlap tools
verbose = False
test_tool_init(ORUtils.DeltaROverlapTool, verbose=verbose)
test_tool_init(ORUtils.ObjLinkOverlapTool, verbose=verbose)
test_tool_init(ORUtils.EleEleOverlapTool, verbose=verbose)
test_tool_init(ORUtils.EleJetOverlapTool, verbose=verbose)
test_tool_init(ORUtils.EleMuSharedTrkOverlapTool, verbose=verbose)
test_tool_init(ORUtils.MuJetOverlapTool, verbose=verbose)
test_tool_init(ORUtils.TauJetOverlapTool, verbose=verbose)
test_tool_init(ORUtils.TauLooseEleOverlapTool, verbose=verbose)
test_tool_init(ORUtils.TauLooseMuOverlapTool, verbose=verbose)
test_tool_init(ORUtils.TauAntiTauJetOverlapTool, verbose=verbose)

# Try creating some helper objects
bjetHelper = ORUtils.BJetHelper('isBJet')
DeltaRMatcher = ORUtils.DeltaRMatcher(0.4)
slidingMatcher = ORUtils.SlidingDeltaRMatcher(0.04, 10.)
ghostMatcher = ORUtils.MuJetGhostDRMatcher(0.4)
decHelper = ORUtils.OverlapDecorationHelper('selected', 'overlaps')
linkHelper = ORUtils.OverlapLinkHelper('overlapLink')
