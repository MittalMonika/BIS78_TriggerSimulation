
C*********************************************************************

C...FHSETPARA
C...Dummy function, to be removed when FEYNHIGGS is to be linked.

      SUBROUTINE FHSETPARA(IER,SCF,DMT,DMB,DMW,DMZ,DTANB,DMA,DMH,DM3L, 
     &     DM3E,DM3Q,DM3U,DM3D,DM2L,DM2E,DM2Q,DM2U, DM2D,DM1L,DM1E,DM1Q, 
     &     DM1U,DM1D,DMU,ATAU,AT,AB,AMU,AC,AS,AE,AU,AD,
     &     DM1,DM2,DM3,RLT,RLB,QTAU,QT,QB)
      IMPLICIT DOUBLE PRECISION(A-H, O-Z)
      IMPLICIT INTEGER(I-N)

C...Commonblocks.
      COMMON/PYDAT1/MSTU(200),PARU(200),MSTJ(200),PARJ(200)
      SAVE /PYDAT1/
 
C...Stop program if this routine is ever called.
      WRITE(MSTU(11),5000)
      IF(PYR(0).LT.10D0) STOP
 
C...Format for error printout.
 5000 FORMAT(1X,'Error: you did not link FEYNHIGGS correctly.'/
     &1X,'Dummy routine FHSETPARA in PYTHIA file called instead.'/
     &1X,'Execution stopped!')
      RETURN
      END
