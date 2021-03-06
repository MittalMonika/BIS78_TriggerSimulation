*CMZ :          02/08/2001  11.42.47  by  Maria Smizanska
*-- Author :    Maria Smizanska   22/09/96
      FUNCTION charm(I)
*                                                                      *
*----------------------------------------------------------------------*
*                                                                      *
* Name : charm                                                        *
*        (module)                                                      *
*                                                                      *
* Description :   Is it charm hadron ? Selects only hadrons with      *
*                 anti_c quark (D-, antiBaryons Lambdac-)              *
*                 because  exclusive signal channels are forced        *
*                 for these hadrons                                    *
* Arguments   :  I particle line # in LUJETS                           *
*                                                                      *
*                                                                      *
* Banks/Tables:                                                        *
*          LUJETS    used                                              *
*                                                                      *
*----------------------------------------------------------------------*
*                                                                      *
* Author : Maria Smizanska                    Date : 20/09/96          *
*                                                                      *
*                                                                      *
* Last modifications :                                                 *
*        [name]                               Date : [date]            *
*                                                                      *
* Keywords :                                                           *
*        [keywords]                                                    *
*----------------------------------------------------------------------*
*
c +CDE, TYPING.
*KEEP,DOUB.
C...Double precision and integer declarations.
      IMPLICIT DOUBLE PRECISION(A-H, O-Z)
      IMPLICIT INTEGER(I-N)
c      INTEGER PYK,PYCHGE,PYCOMP
*KEEP,LUJETS.
       COMMON/PYJETS/N,NPAD,K(4000,5),P(4000,5),V(4000,5)
       integer charm
*KEEP,FAMILY.
       name_self(I)=K(I,2)
       name_selv(I)=abs(K(I,2))
       name_father(I)=K(K(I,3),2)
       name_grandpa(I)=K(K(K(I,3),3),2)
       name_daughter1(I)=K(K(I,4),2)
       name_daughter2(I)=K(K(I,5),2)

       link_self(I)=I
       link_father(I)=K(I,3)
       link_grandpa(I)=K(K(I,3),3)
       link_daughter1(I)=K(I,4)
       link_daughter2(I)=K(I,5)
*KEND.
C ----------- end CDE --------------------------------------------------
      charm=0
c      if((name_self(I).ge.400.and.name_self(I).lt.500).or.
c     +   (name_self(I).gt.4000.and.name_self(I).le.5000))
c     +charm=1

      if(name_self(I).eq.-411.or.name_self(I).eq.-421.or.
     +   name_self(I).eq.-431.or.
     +   name_self(I).eq.-4122.or.name_self(I).eq.-4132.or.
     +   name_self(I).eq.-4232.or.name_self(I).eq.-4332)
     +charm=1


      return
*
      END          ! charm
