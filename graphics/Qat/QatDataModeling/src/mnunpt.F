*
* $Id: mnunpt.F,v 1.1 2008/10/27 20:58:48 jfb Exp $
*
* $Log: mnunpt.F,v $
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
      LOGICAL FUNCTION MNUNPT(CFNAME)
C           is .TRUE. if CFNAME contains unprintable characters.
      CHARACTER CFNAME*(*)
      CHARACTER CPT*80, CP1*40,CP2*40
      PARAMETER (CP1=' ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklm')
      PARAMETER (CP2='nopqrstuvwxyz1234567890./;:[]$%*_!@#&+()')
      CPT=CP1//CP2
      MNUNPT = .FALSE.
      L = LEN(CFNAME)
      DO 100 I= 1, L
         DO 50 IC= 1, 80
         IF (CFNAME(I:I) .EQ. CPT(IC:IC))  GO TO 100
   50    CONTINUE
      MNUNPT = .TRUE.
      GO TO 150
  100 CONTINUE
  150 CONTINUE
      RETURN
      END
