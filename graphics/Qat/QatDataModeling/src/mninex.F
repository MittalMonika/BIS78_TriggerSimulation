*
* $Id: mninex.F,v 1.1 2008/10/27 20:58:48 jfb Exp $
*
* $Log: mninex.F,v $
* Revision 1.1  2008/10/27 20:58:48  jfb
* commit minuit
*
* Revision 1.1  2007/01/04 15:44:42  joe
* Populating...
*
* Revision 1.1.1.1  2000/06/08 11:19:20  andras
* import of MINUIT from CERNlib 2000
*
* Revision 1.1.1.1  1996/03/07 14:31:30  mclareni
* Minuit
*
*
#include "QatDataModeling/pilot.h"
      SUBROUTINE MNINEX(PINT)
#include "QatDataModeling/d506dp.inc"
CC        Transforms from internal coordinates (PINT) to external
CC        parameters (U).   The minimizing routines which work in
CC        internal coordinates call this routine before calling FCN.
#include "QatDataModeling/d506cm.inc"
      DIMENSION PINT(*)
      DO 100 J= 1, NPAR
      I = NEXOFI(J)
      IF (NVARL(I) .EQ. 1) THEN
         U(I) = PINT(J)
      ELSE
         U(I) = ALIM(I) + 0.5*(SIN(PINT(J)) +1.0) * (BLIM(I)-ALIM(I))
      ENDIF
  100 CONTINUE
      RETURN
      END
