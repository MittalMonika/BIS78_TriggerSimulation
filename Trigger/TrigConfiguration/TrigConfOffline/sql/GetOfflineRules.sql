

-- Obtain the differences in parameters for same components
SELECT DISTINCT HLT_SETUP.HST_ID, HLT_SETUP.HST_NAME, x.HCP_ID, x.HCP_NAME, x.HCP_ALIAS, xx.HPA_ID, xx.HPA_NAME, xx.HPA_VALUE
FROM HLT_COMPONENT x
  JOIN HLT_ST_TO_CP ON (HLT_ST_TO_CP.HST2CP_COMPONENT_ID = x.HCP_ID)
   JOIN HLT_SETUP ON (HLT_ST_TO_CP.HST2CP_SETUP_ID = HLT_SETUP.HST_ID)
     JOIN HLT_CP_TO_PA ON (HCP_ID = HLT_CP_TO_PA.HCP2PA_COMPONENT_ID)
        JOIN HLT_PARAMETER xx ON (xx.HPA_ID = HLT_CP_TO_PA.HCP2PA_PARAMETER_ID)
     WHERE ((HLT_SETUP.HST_ID IN (3,4))
       AND (x.HCP_NAME, x.HCP_ALIAS) IN (SELECT DISTINCT y.HCP_NAME, y.HCP_ALIAS
         FROM HLT_COMPONENT y
  	   JOIN HLT_ST_TO_CP ON (HLT_ST_TO_CP.HST2CP_COMPONENT_ID = y.HCP_ID)
   	     JOIN HLT_SETUP ON (HLT_ST_TO_CP.HST2CP_SETUP_ID = HLT_SETUP.HST_ID)
               JOIN HLT_CP_TO_PA ON (y.HCP_ID = HLT_CP_TO_PA.HCP2PA_COMPONENT_ID)
                 JOIN HLT_PARAMETER ON (HLT_PARAMETER.HPA_ID = HLT_CP_TO_PA.HCP2PA_PARAMETER_ID)
               WHERE (HLT_SETUP.HST_ID IN (5,6))
             )
       AND (x.HCP_ID) NOT IN (SELECT DISTINCT z.HCP_ID
         FROM HLT_COMPONENT z
  	   JOIN HLT_ST_TO_CP ON (HLT_ST_TO_CP.HST2CP_COMPONENT_ID = z.HCP_ID)
   	     JOIN HLT_SETUP ON (HLT_ST_TO_CP.HST2CP_SETUP_ID = HLT_SETUP.HST_ID)
               JOIN HLT_CP_TO_PA ON (z.HCP_ID = HLT_CP_TO_PA.HCP2PA_COMPONENT_ID)
                 JOIN HLT_PARAMETER ON (HLT_PARAMETER.HPA_ID = HLT_CP_TO_PA.HCP2PA_PARAMETER_ID)
               WHERE (HLT_SETUP.HST_ID IN (5,6))
             )
       AND NOT ((x.HCP_NAME, x.HCP_ALIAS, xx.HPA_ID) IN (SELECT DISTINCT y.HCP_NAME, y.HCP_ALIAS, yy.HPA_ID
         FROM HLT_COMPONENT y
  	   JOIN HLT_ST_TO_CP ON (HLT_ST_TO_CP.HST2CP_COMPONENT_ID = y.HCP_ID)
   	     JOIN HLT_SETUP ON (HLT_ST_TO_CP.HST2CP_SETUP_ID = HLT_SETUP.HST_ID)
               JOIN HLT_CP_TO_PA ON (y.HCP_ID = HLT_CP_TO_PA.HCP2PA_COMPONENT_ID)
                 JOIN HLT_PARAMETER yy ON (yy.HPA_ID = HLT_CP_TO_PA.HCP2PA_PARAMETER_ID)
               WHERE (HLT_SETUP.HST_ID IN (5,6))
         )
       ))
       OR
     ((HLT_SETUP.HST_ID IN (5,6))
       AND (x.HCP_NAME, x.HCP_ALIAS) IN (SELECT DISTINCT y.HCP_NAME, y.HCP_ALIAS
         FROM HLT_COMPONENT y
  	   JOIN HLT_ST_TO_CP ON (HLT_ST_TO_CP.HST2CP_COMPONENT_ID = y.HCP_ID)
   	     JOIN HLT_SETUP ON (HLT_ST_TO_CP.HST2CP_SETUP_ID = HLT_SETUP.HST_ID)
               JOIN HLT_CP_TO_PA ON (y.HCP_ID = HLT_CP_TO_PA.HCP2PA_COMPONENT_ID)
                 JOIN HLT_PARAMETER ON (HLT_PARAMETER.HPA_ID = HLT_CP_TO_PA.HCP2PA_PARAMETER_ID)
               WHERE (HLT_SETUP.HST_ID IN (3,4))
             )
       AND (x.HCP_ID) NOT IN (SELECT DISTINCT z.HCP_ID
         FROM HLT_COMPONENT z
  	   JOIN HLT_ST_TO_CP ON (HLT_ST_TO_CP.HST2CP_COMPONENT_ID = z.HCP_ID)
   	     JOIN HLT_SETUP ON (HLT_ST_TO_CP.HST2CP_SETUP_ID = HLT_SETUP.HST_ID)
               JOIN HLT_CP_TO_PA ON (z.HCP_ID = HLT_CP_TO_PA.HCP2PA_COMPONENT_ID)
                 JOIN HLT_PARAMETER ON (HLT_PARAMETER.HPA_ID = HLT_CP_TO_PA.HCP2PA_PARAMETER_ID)
               WHERE (HLT_SETUP.HST_ID IN (3,4))
             )
       AND NOT ((x.HCP_NAME, x.HCP_ALIAS, xx.HPA_ID) IN (SELECT DISTINCT y.HCP_NAME, y.HCP_ALIAS, yy.HPA_ID
         FROM HLT_COMPONENT y
  	   JOIN HLT_ST_TO_CP ON (HLT_ST_TO_CP.HST2CP_COMPONENT_ID = y.HCP_ID)
   	     JOIN HLT_SETUP ON (HLT_ST_TO_CP.HST2CP_SETUP_ID = HLT_SETUP.HST_ID)
               JOIN HLT_CP_TO_PA ON (y.HCP_ID = HLT_CP_TO_PA.HCP2PA_COMPONENT_ID)
                 JOIN HLT_PARAMETER yy ON (yy.HPA_ID = HLT_CP_TO_PA.HCP2PA_PARAMETER_ID)
               WHERE (HLT_SETUP.HST_ID IN (3,4))
         )
       ))
ORDER BY HCP_NAME, HCP_ALIAS, xx.HPA_NAME, HLT_SETUP.HST_ID, xx.HPA_ID



-- Obtain the differences in parameters for components with the same alias
SELECT DISTINCT HLT_SETUP.HST_ID, HLT_SETUP.HST_NAME, x.HCP_ID, x.HCP_NAME, x.HCP_ALIAS, xx.HPA_ID, xx.HPA_NAME, xx.HPA_VALUE
FROM HLT_COMPONENT x
  JOIN HLT_ST_TO_CP ON (HLT_ST_TO_CP.HST2CP_COMPONENT_ID = x.HCP_ID)
   JOIN HLT_SETUP ON (HLT_ST_TO_CP.HST2CP_SETUP_ID = HLT_SETUP.HST_ID)
     JOIN HLT_CP_TO_PA ON (HCP_ID = HLT_CP_TO_PA.HCP2PA_COMPONENT_ID)
        JOIN HLT_PARAMETER xx ON (xx.HPA_ID = HLT_CP_TO_PA.HCP2PA_PARAMETER_ID)
     WHERE ((HLT_SETUP.HST_ID IN (3,4))
       AND (x.HCP_ALIAS) IN (SELECT DISTINCT y.HCP_ALIAS
         FROM HLT_COMPONENT y
  	   JOIN HLT_ST_TO_CP ON (HLT_ST_TO_CP.HST2CP_COMPONENT_ID = y.HCP_ID)
   	     JOIN HLT_SETUP ON (HLT_ST_TO_CP.HST2CP_SETUP_ID = HLT_SETUP.HST_ID)
               JOIN HLT_CP_TO_PA ON (y.HCP_ID = HLT_CP_TO_PA.HCP2PA_COMPONENT_ID)
                 JOIN HLT_PARAMETER ON (HLT_PARAMETER.HPA_ID = HLT_CP_TO_PA.HCP2PA_PARAMETER_ID)
               WHERE (HLT_SETUP.HST_ID IN (5,6))
             )
       AND (x.HCP_ID) NOT IN (SELECT DISTINCT z.HCP_ID
         FROM HLT_COMPONENT z
  	   JOIN HLT_ST_TO_CP ON (HLT_ST_TO_CP.HST2CP_COMPONENT_ID = z.HCP_ID)
   	     JOIN HLT_SETUP ON (HLT_ST_TO_CP.HST2CP_SETUP_ID = HLT_SETUP.HST_ID)
               JOIN HLT_CP_TO_PA ON (z.HCP_ID = HLT_CP_TO_PA.HCP2PA_COMPONENT_ID)
                 JOIN HLT_PARAMETER ON (HLT_PARAMETER.HPA_ID = HLT_CP_TO_PA.HCP2PA_PARAMETER_ID)
               WHERE (HLT_SETUP.HST_ID IN (5,6))
             )
       AND NOT ((x.HCP_ALIAS, xx.HPA_ID) IN (SELECT DISTINCT y.HCP_ALIAS, yy.HPA_ID
         FROM HLT_COMPONENT y
  	   JOIN HLT_ST_TO_CP ON (HLT_ST_TO_CP.HST2CP_COMPONENT_ID = y.HCP_ID)
   	     JOIN HLT_SETUP ON (HLT_ST_TO_CP.HST2CP_SETUP_ID = HLT_SETUP.HST_ID)
               JOIN HLT_CP_TO_PA ON (y.HCP_ID = HLT_CP_TO_PA.HCP2PA_COMPONENT_ID)
                 JOIN HLT_PARAMETER yy ON (yy.HPA_ID = HLT_CP_TO_PA.HCP2PA_PARAMETER_ID)
               WHERE (HLT_SETUP.HST_ID IN (5,6))
         )
       ))
       OR
     ((HLT_SETUP.HST_ID IN (5,6))
       AND (x.HCP_ALIAS) IN (SELECT DISTINCT y.HCP_ALIAS
         FROM HLT_COMPONENT y
  	   JOIN HLT_ST_TO_CP ON (HLT_ST_TO_CP.HST2CP_COMPONENT_ID = y.HCP_ID)
   	     JOIN HLT_SETUP ON (HLT_ST_TO_CP.HST2CP_SETUP_ID = HLT_SETUP.HST_ID)
               JOIN HLT_CP_TO_PA ON (y.HCP_ID = HLT_CP_TO_PA.HCP2PA_COMPONENT_ID)
                 JOIN HLT_PARAMETER ON (HLT_PARAMETER.HPA_ID = HLT_CP_TO_PA.HCP2PA_PARAMETER_ID)
               WHERE (HLT_SETUP.HST_ID IN (3,4))
             )
       AND (x.HCP_ID) NOT IN (SELECT DISTINCT z.HCP_ID
         FROM HLT_COMPONENT z
  	   JOIN HLT_ST_TO_CP ON (HLT_ST_TO_CP.HST2CP_COMPONENT_ID = z.HCP_ID)
   	     JOIN HLT_SETUP ON (HLT_ST_TO_CP.HST2CP_SETUP_ID = HLT_SETUP.HST_ID)
               JOIN HLT_CP_TO_PA ON (z.HCP_ID = HLT_CP_TO_PA.HCP2PA_COMPONENT_ID)
                 JOIN HLT_PARAMETER ON (HLT_PARAMETER.HPA_ID = HLT_CP_TO_PA.HCP2PA_PARAMETER_ID)
               WHERE (HLT_SETUP.HST_ID IN (3,4))
             )
       AND NOT ((x.HCP_ALIAS, xx.HPA_ID) IN (SELECT DISTINCT y.HCP_ALIAS, yy.HPA_ID
         FROM HLT_COMPONENT y
  	   JOIN HLT_ST_TO_CP ON (HLT_ST_TO_CP.HST2CP_COMPONENT_ID = y.HCP_ID)
   	     JOIN HLT_SETUP ON (HLT_ST_TO_CP.HST2CP_SETUP_ID = HLT_SETUP.HST_ID)
               JOIN HLT_CP_TO_PA ON (y.HCP_ID = HLT_CP_TO_PA.HCP2PA_COMPONENT_ID)
                 JOIN HLT_PARAMETER yy ON (yy.HPA_ID = HLT_CP_TO_PA.HCP2PA_PARAMETER_ID)
               WHERE (HLT_SETUP.HST_ID IN (3,4))
         )
       ))
ORDER BY HCP_NAME, HCP_ALIAS, xx.HPA_NAME, HLT_SETUP.HST_ID, xx.HPA_ID

-- Obtain Components not present in other setup
SELECT DISTINCT HLT_SETUP.HST_ID, HLT_SETUP.HST_NAME, HCP_ID, HCP_NAME, HCP_ALIAS
FROM HLT_COMPONENT x
  JOIN HLT_ST_TO_CP ON (HLT_ST_TO_CP.HST2CP_COMPONENT_ID = HCP_ID)
   JOIN HLT_SETUP ON (HLT_ST_TO_CP.HST2CP_SETUP_ID = HLT_SETUP.HST_ID)
     JOIN HLT_CP_TO_PA ON (HCP_ID = HLT_CP_TO_PA.HCP2PA_COMPONENT_ID)
        JOIN HLT_PARAMETER ON (HLT_PARAMETER.HPA_ID = HLT_CP_TO_PA.HCP2PA_PARAMETER_ID)
     WHERE (HLT_SETUP.HST_ID IN (3,4))
       AND (x.HCP_NAME, x.HCP_ALIAS) NOT IN (SELECT DISTINCT y.HCP_NAME, y.HCP_ALIAS
         FROM HLT_COMPONENT y
  	   JOIN HLT_ST_TO_CP ON (HLT_ST_TO_CP.HST2CP_COMPONENT_ID = y.HCP_ID)
   	     JOIN HLT_SETUP ON (HLT_ST_TO_CP.HST2CP_SETUP_ID = HLT_SETUP.HST_ID)
               JOIN HLT_CP_TO_PA ON (y.HCP_ID = HLT_CP_TO_PA.HCP2PA_COMPONENT_ID)
                 JOIN HLT_PARAMETER ON (HLT_PARAMETER.HPA_ID = HLT_CP_TO_PA.HCP2PA_PARAMETER_ID)
               WHERE (HLT_SETUP.HST_ID IN (5,6))
             )

ORDER BY HLT_SETUP.HST_ID, HCP_NAME, HCP_ALIAS, HLT_PARAMETER.HPA_ID, HLT_PARAMETER.HPA_NAME

-- Obtain Identical components
SELECT DISTINCT HLT_SETUP.HST_ID, HLT_SETUP.HST_NAME, HCP_ID, HCP_NAME, HCP_ALIAS
FROM HLT_COMPONENT x
  JOIN HLT_ST_TO_CP ON (HLT_ST_TO_CP.HST2CP_COMPONENT_ID = HCP_ID)
   JOIN HLT_SETUP ON (HLT_ST_TO_CP.HST2CP_SETUP_ID = HLT_SETUP.HST_ID)
     JOIN HLT_CP_TO_PA ON (HCP_ID = HLT_CP_TO_PA.HCP2PA_COMPONENT_ID)
        JOIN HLT_PARAMETER ON (HLT_PARAMETER.HPA_ID = HLT_CP_TO_PA.HCP2PA_PARAMETER_ID)
     WHERE (HLT_SETUP.HST_ID IN (3,4))
       AND (x.HCP_NAME, x.HCP_ALIAS) IN (SELECT DISTINCT y.HCP_NAME, y.HCP_ALIAS
         FROM HLT_COMPONENT y
  	   JOIN HLT_ST_TO_CP ON (HLT_ST_TO_CP.HST2CP_COMPONENT_ID = y.HCP_ID)
   	     JOIN HLT_SETUP ON (HLT_ST_TO_CP.HST2CP_SETUP_ID = HLT_SETUP.HST_ID)
               JOIN HLT_CP_TO_PA ON (y.HCP_ID = HLT_CP_TO_PA.HCP2PA_COMPONENT_ID)
                 JOIN HLT_PARAMETER ON (HLT_PARAMETER.HPA_ID = HLT_CP_TO_PA.HCP2PA_PARAMETER_ID)
               WHERE (HLT_SETUP.HST_ID IN (5,6))
             )
       AND (x.HCP_ID) IN (SELECT DISTINCT z.HCP_ID
         FROM HLT_COMPONENT z
  	   JOIN HLT_ST_TO_CP ON (HLT_ST_TO_CP.HST2CP_COMPONENT_ID = z.HCP_ID)
   	     JOIN HLT_SETUP ON (HLT_ST_TO_CP.HST2CP_SETUP_ID = HLT_SETUP.HST_ID)
               JOIN HLT_CP_TO_PA ON (z.HCP_ID = HLT_CP_TO_PA.HCP2PA_COMPONENT_ID)
                 JOIN HLT_PARAMETER ON (HLT_PARAMETER.HPA_ID = HLT_CP_TO_PA.HCP2PA_PARAMETER_ID)
               WHERE (HLT_SETUP.HST_ID IN (3,4))
             )

ORDER BY HCP_NAME, HCP_ALIAS, HLT_SETUP.HST_ID, HLT_PARAMETER.HPA_ID, HLT_PARAMETER.HPA_NAME


