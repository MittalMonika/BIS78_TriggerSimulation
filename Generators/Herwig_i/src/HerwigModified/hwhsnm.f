CDECK  ID>, HWHSNM. 
*CMZ :-        -20/09/95  15.28.53  by  Mike Seymour
*-- Author :    Mike Seymour
C-----------------------------------------------------------------------
*-- Author :    Mike Seymour, Mark Heyes, Brian Cox, Jeff Forshaw 18.12.97
* Upgraded to remove asymptotic approximation
C-----------------------------------------------------------------------
*__ Author :    Graham Jones, James Monk 15.04.2008
* Allow switch between original asymptotic version and upgraded version for ATLAS
* Switch depends on omega0 - omega0 > 0 runs the original (and uses that omega0 value)
*                            omega0 < 0 runs the upgraded version
C-----------------------------------------------------------------------
      FUNCTION HWHSNM(ID1,ID2,S,T)
C     MATRIX ELEMENT SQUARED FOR COLOUR-SINGLET PARTON-PARTON SCATTERING
C     INCLUDES SPIN AND COLOUR AVERAGES AND SUMS.
C     FOR PHOTON EXCHANGE, INTERFERENCE WITH U-CHANNEL CONTRIBUTION IS
C     INCLUDED FOR IDENTICAL QUARKS AND LIKEWISE S-CHANNEL CONTRIBUTION
C     FOR IDENTICAL QUARK-ANTIQUARK PAIRS.
C-----------------------------------------------------------------------
      INCLUDE 'HERWIG65.INC'
      DOUBLE PRECISION HWHSNM,HWUAEM,HWUALF,S,T,ASQ,AINU,AINS,Y,SOLD,
     $ TOLD,QQ(13,13),ZETA3
      INTEGER ID1,ID2
      LOGICAL PHOTON
      LOGICAL PRVSN
C---ZETA3=RIEMANN ZETA FUNCTION(3)
      PARAMETER (ZETA3=1.202056903159594D0)
C---paramtric fit to the function used in the Bartels/Forshaw/et al. calculation
      DOUBLE PRECISION para,parb,parc,pard,pare !,alphas
      PARAMETER (para=7.6155D0)
      PARAMETER (parb=-20.259D0)
      PARAMETER (parc=41.534D0)
      PARAMETER (pard=-2.6150D0)
      PARAMETER (pare=8.6744D0)
      DOUBLE PRECISION Z

      SAVE ASQ,AINU,AINS,SOLD,TOLD,QQ
      DATA ASQ,AINU,AINS,SOLD,TOLD,QQ/5*0,169*-1/
      SAVE PRVSN
      DATA PRVSN/.TRUE./
C---PHOTON=.TRUE. FOR PHOTON EXCHANGE, .FALSE. FOR MUELLER-TANG
      PHOTON=MOD(IPROC,100).GE.50
      IF (OMEGA0.lt.0.) THEN
      
c---This is the upgraded version:      

C---QQ CACHES THE KINEMATIC-INDEPENDENT FACTORS, TO MAKE IT RUN FASTER
C  (BEARING IN MIND THAT THIS ROUTINE IS CALLED 169 TIMES PER EVENT)
        IF (QQ(ID1,ID2).LT.0) THEN
          IF(PRVSN) THEN 
            WRITE(*,*)
     $      "        USING UPGRADED HWHSNM FOR COLOUR SINGLET EXCHANGE" 
            PRVSN = .FALSE.
          ENDIF

          IF (PHOTON) THEN
            IF (ID1.EQ.13.OR.ID2.EQ.13) THEN
              QQ(ID1,ID2)=0
            ELSE
              QQ(ID1,ID2)=(QFCH(MOD(ID1-1,6)+1)*QFCH(MOD(ID2-1,6)+1))**2
     $            *(4*PIFAC)**2
            ENDIF
          ELSE
            IF (ID1.EQ.13.AND.ID2.EQ.13) THEN
              QQ(ID1,ID2)=CAFAC**4
            ELSEIF (ID1.EQ.13.OR.ID2.EQ.13) THEN
              QQ(ID1,ID2)=(CAFAC*CFFAC)**2
            ELSE
              QQ(ID1,ID2)=CFFAC**4
            ENDIF
            QQ(ID1,ID2)=QQ(ID1,ID2)*16*PIFAC
          ENDIF
        ENDIF
C---THE KINEMATIC-DEPENDENT PART IS ALSO CACHED
        IF (S.NE.SOLD.OR.T.NE.TOLD) THEN
          IF (PHOTON) THEN
            AINS=HWUAEM(T)**2
            ASQ=2*(S**2+(S+T)**2)/T**2*AINS
            AINU=(-2)*S/T*AINS
            AINS=2*AINS-AINU
          ELSE
            Y=LOG(S/(-T))
            Z=(1.5/PIFAC)*asfixd*Y
            ASQ=1.D0/(4.D0*PIFAC)*asfixd**4
     &            *(para+parb*Z+parc*Z**2+exp(pard+pare*Z))
     &            *(S/T)**2

            AINU=0
            AINS=0
          ENDIF
        ENDIF
C---THE FINAL ANSWER IS JUST THEIR PRODUCT
        IF (ID1.EQ.ID2) THEN
          HWHSNM=QQ(ID1,ID2)*(ASQ+AINU)
        ELSEIF (ABS(ID1-ID2).EQ.6) THEN
          HWHSNM=QQ(ID1,ID2)*(ASQ+AINS)
        ELSE
          HWHSNM=QQ(ID1,ID2)*ASQ
        ENDIF
      ELSE
c---This is the original version:   
        IF (QQ(ID1,ID2).LT.ZERO) THEN
        
          IF(PRVSN) THEN 
            WRITE(*,*)
     $      "        USING ORIGINAL HWHSNM FOR COLOUR SINGLET EXCHANGE" 
            PRVSN=.FALSE.
          ENDIF
        
          IF (PHOTON) THEN
            IF (ID1.EQ.13.OR.ID2.EQ.13) THEN
              QQ(ID1,ID2)=0
            ELSE
              QQ(ID1,ID2)=(QFCH(MOD(ID1-1,6)+1)*QFCH(MOD(ID2-1,6)+1))**2
     $            *(4*PIFAC)**2
            ENDIF
          ELSE
            IF (ID1.EQ.13.AND.ID2.EQ.13) THEN
              QQ(ID1,ID2)=CAFAC**4
            ELSEIF (ID1.EQ.13.OR.ID2.EQ.13) THEN
              QQ(ID1,ID2)=(CAFAC*CFFAC)**2
            ELSE
              QQ(ID1,ID2)=CFFAC**4
            ENDIF
            QQ(ID1,ID2)=QQ(ID1,ID2)*
     $          PIFAC**3/(4*(3.5*ASFIXD*CAFAC*ZETA3)**3)
     $          *(16*PIFAC)
          ENDIF
        ENDIF
C---THE KINEMATIC-DEPENDENT PART IS ALSO CACHED
        IF (S.NE.SOLD.OR.T.NE.TOLD) THEN
          IF (PHOTON) THEN
            AINS=HWUAEM(T)**2
            ASQ=2*(S**2+(S+T)**2)/T**2*AINS
            AINU=(-4)*S/T*AINS/NCOLO
            AINS=4*AINS/NCOLO-AINU
          ELSE
            Y=LOG(S/(-T))+ONE
            ASQ=HWUALF(1,EMSCA)**4*(S/T)**2*EXP(2*OMEGA0*Y)/Y**3
            AINU=0
            AINS=0
          ENDIF
        ENDIF
C---THE FINAL ANSWER IS JUST THEIR PRODUCT
        IF (ID1.EQ.ID2) THEN
          HWHSNM=QQ(ID1,ID2)*(ASQ+AINU)
        ELSEIF (ABS(ID1-ID2).EQ.6) THEN
          HWHSNM=QQ(ID1,ID2)*(ASQ+AINS)
        ELSE
          HWHSNM=QQ(ID1,ID2)*ASQ
        ENDIF
      ENDIF
      END
