-- PROJECT: ATLAS MDT Calibration DB
-- AUTHOR: Elisabetta.Vilucchi@lnf.infn.it, celio@fis.uniroma3.it
-- DATE: May 08
-- VERSION v1r1

--USAGE:
--sqlplus <USER_NAME>/<USER_PASSWORD> @mdt_tube_table.sql <USER_NAME> <SITE_NAME>(2 characters like MU,MI,RM...) <USER_PASSOWRD> <TABLESPCE_NAME>

--DROP TABLE &1.."MDT_TUBE";
CREATE TABLE &1.."MDT_TUBE" 
( "SEQID" NUMBER NOT NULL,
"TUBE_ID" NUMBER(10) NOT NULL ,
"HEAD_ID" NUMBER NOT NULL , 
"CHAMBER" varchar2(10) NOT NULL,
"SITE_NAME" VARCHAR2(2) DEFAULT UPPER('&2') ,
"INSERT_TIME" VARCHAR2(29),
"LOWRUN" NUMBER(8) NOT NULL , 
"UPRUN" NUMBER(8) NOT NULL , 
"RUNFLAG" NUMBER(3) DEFAULT 0 NOT NULL , 
"CALIBFLAG" NUMBER(3) DEFAULT 0 NOT NULL , 
"VALIDFLAG" NUMBER(3) DEFAULT 0 NOT NULL , 
"ENTRIES" BINARY_FLOAT DEFAULT 0 NOT NULL , 
"CHISQUARE_1" BINARY_FLOAT DEFAULT -1 NOT NULL ,
"CHISQUARE_2" BINARY_FLOAT DEFAULT -1 NOT NULL ,
"P0" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"P1" BINARY_FLOAT DEFAULT 0 NOT NULL , 
"P2" BINARY_FLOAT DEFAULT 0 NOT NULL , 
"P3" BINARY_FLOAT DEFAULT 0 NOT NULL , 
"P4" BINARY_FLOAT DEFAULT 0 NOT NULL , 
"P5" BINARY_FLOAT DEFAULT 0 NOT NULL , 
"P6" BINARY_FLOAT DEFAULT 0 NOT NULL , 
"P7" BINARY_FLOAT DEFAULT 0 NOT NULL , 
"P8" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"P9" BINARY_FLOAT DEFAULT 0 NOT NULL , 
"P0_ERR" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"P1_ERR" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"P2_ERR" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"P3_ERR" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"P4_ERR" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"P5_ERR" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"P6_ERR" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"P7_ERR" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"P8_ERR" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"P9_ERR" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"COV_1" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"COV_2" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"COV_3" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"COV_4" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"ADC_0" BINARY_FLOAT DEFAULT 0 NOT NULL , 
"ADC_1" BINARY_FLOAT DEFAULT 0 NOT NULL , 
"ADC_2" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"ADC_3" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"ADC_0_ERR" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"ADC_1_ERR" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"ADC_2_ERR" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"ADC_3_ERR" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"ADC_CHISQUARE" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"ADC_PEDESTAL" BINARY_FLOAT DEFAULT 0 NOT NULL ,
"ADC_PEDESTAL_WIDTH" BINARY_FLOAT DEFAULT 0 NOT NULL , 
"NHITS" NUMBER(6) DEFAULT 0 NOT NULL ,
"NHITS_ABOVE_ADC_CUT" NUMBER(6)DEFAULT 0 NOT NULL ,
"ALGO_FLAG" varchar2(12) DEFAULT 0 NOT NULL ,
"TUBE_GROUPING" varchar2(12) DEFAULT 0 NOT NULL ,
CONSTRAINT "HEAD_ID_SITE_REF" FOREIGN KEY ("HEAD_ID", "SITE_NAME") REFERENCES &1.."MDT_HEAD" ("HEAD_ID", "SITE_NAME") VALIDATE , 
CONSTRAINT "SEQID_SITE_PR" PRIMARY KEY ("SEQID", "SITE_NAME") VALIDATE ,
CHECK (uprun>=lowrun) VALIDATE ) 
TABLESPACE &4 PCTFREE 10 INITRANS 1 MAXTRANS 255 STORAGE ( INITIAL 64K BUFFER_POOL DEFAULT) LOGGING;
