*
* $Id: mntiny.F,v 1.1 2008/10/27 20:58:48 jfb Exp $
*
* $Log: mntiny.F,v $
* Revision 1.1  2008/10/27 20:58:48  jfb
* commit minuit
*
* Revision 1.1  2007/01/04 15:44:42  joe
* Populating...
*
* Revision 1.1.1.1  2000/06/08 11:19:20  andras
* import of MINUIT from CERNlib 2000
*
* Revision 1.1.1.1  1996/03/07 14:31:32  mclareni
* Minuit
*
*
#include "QatDataModeling/pilot.h"
      SUBROUTINE MNTINY(EPSP1,EPSBAK)
#include "QatDataModeling/d506dp.inc"
CC        Compares its argument with the value 1.0, and returns
CC        the value .TRUE. if they are equal.  To find EPSMAC
CC        safely by foiling the Fortran optimizer
CC
      PARAMETER (ONE=1.0)
      EPSBAK =  EPSP1  - ONE
      RETURN
      END
