#
#  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
#

from AthenaMonitoring.AthenaMonitoringConf import GenericMonitoringTool as _GenericMonitoringTool

class GenericMonitoringTool(_GenericMonitoringTool):
    """Configurable of a GenericMonitoringTool"""

    def __init__(self, name, **kwargs):
        super(GenericMonitoringTool, self).__init__(name, **kwargs)


## Generate histogram definition string for the `GenericMonitoringTool.Histograms` property
#
#  For full details see the GenericMonitoringTool documentation.
#  @param varname  one (1D) or two (2D) variable names separated by comma
#  @param type     histogram type
#  @param path     top-level histrogram directory
#  @param title    Histogram title and optional axis title (same syntax as in TH constructor)
#  @param opt      Histrogram options (see GenericMonitoringTool)
#  @param labels   List of bin labels (for a 2D histogram, sequential list of x- and y-axis labels)
def defineHistogram(varname, type='TH1F', path='EXPERT',
                    title=None,
                    xbins=100, xmin=0, xmax=1,
                    ybins=None, ymin=None, ymax=None, zmin=None, zmax=None, opt='', labels=None):

    if title is None: title=varname
    coded = "%s, %s, %s, %s, %d, %f, %f" % (path, type, varname, title, xbins, xmin, xmax)
    if ybins is not None:
        coded += ",%d, %f, %f" % (ybins, ymin, ymax)
        if zmin is not None:
            coded += ", %f, %f" % (zmin, zmax)
    if ybins is None and ymin is not None:
        coded += ", %f, %f" % (ymin, ymax)

    if isinstance(labels, list) and len(labels)>0:
        coded += ',' + ':'.join(labels) + ':'

    # For backwards compatibility
    elif labels is not None:
        labels = labels.strip()   # remove spurious white-spaces
        if len(labels)>0:
            if labels[-1]!=':': labels += ':'  # C++ parser expects at least one ":"
            coded += ",%s " % labels
    coded += ", %s" % opt

    return coded
