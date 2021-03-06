/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLXMLFILECATALOG_H
#define POOLXMLFILECATALOG_H

#include <iosfwd>
#include <string>
#include <vector>
#include <map>


#include "xercesc/dom/DOMDocument.hpp"
#include "xercesc/dom/DOMNode.hpp"
#include <xercesc/parsers/XercesDOMParser.hpp>

namespace pool {


  class XMLDOMErrorMessanger;
  class XMLQueryHandle;
 
 /**
    \class PoolXMLFileCatalog
     
    This class constitutes the core of the 
    XML based FileCatalog API for POOL. It uses the DOM model and 
    the external XercesC library for parsing. 

   \author Maria Girone  

    \note Glossary

    \arg Physical File Name (PFName) - Name referring to physical location of a file
    \arg Logical  File Name (LFNname) - Name referring to the full set of replicas for a file
    \arg File Identifier (FileID) - Unique identifier if a file. Will be generated by the Guid mechanism.  
    \arg File status (prereg) - Flag indicating if it is a registered or pre-registered pfn 
    \arg Job status  (status) - Flag indicating if the application registering files ended correctly (1) or not (0)

  */




  class PoolXMLFileCatalog {

  public:
    
    enum FileNameType { PFNAME, LFNAME };
    enum PFNStatus {NOTEXISTS,REGISTERED};

    struct poolFNs{
      poolFNs(){}
      poolFNs(const std::string& ifname, const std::string& iguid, 
	      const std::string& itype):
	fname(ifname),guid(iguid),filetype(itype){}
      poolFNs(const std::string& ifname, const std::string& iguid):
	fname(ifname),guid(iguid),filetype(""){}
      std::string fname;
      std::string guid;
      std::string filetype;
    };
  
    typedef std::pair<std::string,std::string> poolMeta;
    typedef std::map<std::string,std::string> poolAttr;
  public:
    /** Create a catalog file, initialization of XercesC.
        @param url the URL of the catalog.
    */
    PoolXMLFileCatalog(const std::string& url);

    /** Destructor, 
        set job status from 0 to 1 on new registered file, 
        and commit them on the catalog once again.
        Finally terminate XercesC.   
    */
    virtual ~PoolXMLFileCatalog();
    
    /** Parse the DOM tree of the XML catalog
     */
    void StartXMLFileCatalog();
    

    /** Dump all PFNames of the catalog and their attributes associate to the FileID
	@param FileID the unique file identifier
	@return std::vector<poolFNs>
    */
    std::vector<poolFNs> getAllpfns(const std::string& FileID);

    /** Dump all PFNames of the catalog and their attributes associate to the FileID
	@param query std::string
	@return std::vector<poolFNs>
    */
    std::vector<poolFNs> getAllpfnsbyq(const std::string& query,
				       unsigned int init=0, 
				       unsigned int ntot=0);
    std::vector<poolFNs> getAlllfnsbyq(const std::string& query,
				       unsigned int init=0, 
				       unsigned int ntot=0);
    std::vector<std::vector<poolMeta> > 
      getAllMetaDatabyq(const std::string& query,
			unsigned int init=0, 
			unsigned int ntot=0);

    std::vector<std::string > getAllGuidbyq(const std::string& query,
					    unsigned int init, 
					    unsigned int ntot);
      
    /** Dump all LFNames of the catalog associate to the FileID
	@param FileID std::string the unique file identifier
	@return std::vector<poolFNs>
    */
    std::vector<poolFNs> getAlllfns(const std::string& FileID);

    /** Dump all MetaData of the catalog 
	@param query std::string 
        @return std::vector<poolFNs>
    */
    std::vector<poolMeta> getAllMetaData(const std::string& FileID);

    /** Get the first available PFName or LFName associate with the FileID
        @param fntype FileNameType
        @param FileID std::string
	@return std::string
    */
    std::string getFile_FROM_FileId(const FileNameType &fntype, 
				    const std::string & FileID);

    /** Get the FileID associated to the PFName or LFName
	@param  fntype FileNameType
	@param  FName  std::string
	@return std::string
    */
    std::string getFileID_FROM_File(const FileNameType &fntype, 
				    const std::string & FName);
    
    /// Deleters  
    /** Delete PFName or LFName  from the DOM tree of the catalog associated to the FileiD
        @param fntype FileNameType
        @param FName  srtd::string
    */
    void delFile(const FileNameType &fntype, const std::string & FName);
    /** Delete FileID DOM Node from the catalog 
        @param FileID  srtd::string
    */
    void delFileID(const std::string & FileID);
    
    /// Manipulators
    /** Generate the FileID by the Guid mechanism
	@return std::string
    */
    std::string generateFileID();

    /** create a DOM Node for a PFName or LFName with all the attributes
        as grandchild of an existing FileID Node
	@param fntype FileNameType
        @param FileID std::string
        @param FName  std::string
    */
    void setFile_onFileID(const FileNameType& fntype, 
			  const std::string & FileID, 
			  const std::string & FName, 
			  const std::string & filetype);

    /** create a DOM Node for a FileID, if it does not exist,  
        and DOM Node of the PFName with all the attributes
        @param FileID std::string
        @param PFName std::string
	@param ftype  std::string
    */    
    void insertFile(const std::string& FileID, 
		    const std::string& PFName, 
		    const std::string& ftype);

    /** create a DOM Node for a FileID and DOM Node of the PFName 
	with all the attributes 
        @param FileID std::string
        @param PFName std::string
	@param ftype  std::string
    */        
    void registerFile(const std::string& FileID, 
		      const std::string& PFName, 
		      const std::string& ftype);

    /** set PFNRequest to REGISTERED for PFName , return the FileID
	@param PFName std::string
	@return std::string
    */
    std::string registerFile(const std::string& PFName);
    
      /// Store information on the XML File
    /** Print on catalog the DOM tree
    */
    void storeOn();
    
    /// Status 
    
    /** Return the status of a PFName
	@param PFName std::string
	@return PFNStatus
    */
    PFNStatus statusPFN(const std::string & PFName);

    /** Return the status of a LFName
	@param LFName std::string
	@return bool
    */
    bool existsLFN(const std::string & LFNname);

    /** Return the status of a FileID
	@param FileID std::string
	@return bool
    */
    bool existsFileID( const std::string & FileID); 

    /** Check if the catalog is read-only
	@return bool
    */
    bool isitReadOnly();

    /** Check if the catalog should be updated 
	@return bool
    */
    bool getUpdate();

    /** Set the catalog to be or not to be update
	@param isupdated bool
    */
    void setUpdate(bool isupdated);

    /** Re-start the catalog as it was at the beginning 
     */
    void getback();

    std::string getGuidMeta(const std::string& fid, 
			    const std::string& attr_name); 

    void setGuidMeta(const std::string& fid, 
		     const std::string& attr_name, 
		     const std::string& attr_value);

    void setGuidMeta(const std::string& attr_name, 
		     const std::string& attr_value);

    void dropGuidMetas();
    void dropGuidMetas(const std::string& attr_name);
    void dropMetas(const std::string& fid);

    poolAttr& getMeta();

    void setMeta(const std::vector<std::string>& name, 
		 const std::vector<std::string>& type);

    std::string getfiletype(const std::string& pfn);

  private:
    typedef xercesc::DOMNode DOMNode;
    typedef xercesc::DOMDocument DOMDocument;
    typedef xercesc::XercesDOMParser XercesDOMParser;
    DOMNode*   getNode(const std::string & nodename, 
		       const std::string & attr,
		       const std::string & attrname);

    DOMNode*   getChild(DOMNode* Parent, const std::string& ChildTagName);

    DOMNode*   getChildAttr(DOMNode* Parent, 
			    const std::string& ChildTagName, 
			    const std::string& ChildAttr);

    DOMNode*   getChildwithAttr(DOMNode* Parent, 
				const std::string &ChildTagName, 
				const std::string &attrib , 
				const std::string &attv);

    void        setAttr(DOMNode* Node, 
			const std::string & attrib , 
			const std::string & attvalue);

    std::string getAttr(DOMNode* Node, const std::string & attrib);

    void        selectType(FileNameType fntype,std::string & daughter,
			   std::string & grandaug);
    
    std::string getfile(bool backup);

    bool finding(XMLQueryHandle& qhandle, unsigned int i);

  private:
    bool read_only;
    bool update;
    DOMDocument* doc;
    XercesDOMParser *parser;
    XMLDOMErrorMessanger *errMessanger;
    unsigned int index_l;
    
  private:
    static const std::string MainNode;
    static const std::string METANode;
    static const std::string ParentNodeName;
    static const std::string DaughtPNodeName;
    static const std::string DaughtLNodeName;
    static const std::string GranDaPNodeName;
    static const std::string GranDaLNodeName;
    static const std::string FileAtt_type;
    static const std::string FileAtt_ID;
    static const std::string FileAtt_name;
    static const std::string FileAtt_fitype;
    
    static const std::string MetaNode;
    static const std::string FileAtt_attname;
    static const std::string FileAtt_attvalu;
    
  private:
    static int NumberOfIstances;
    std::string m_file;
    
  private:
    unsigned int nmeta;
    unsigned int imeta;
    bool fmeta;

  private:
    poolAttr  metaGuid;
    poolAttr  metadata;

  private:
    std::map<std::string, DOMNode *> mem_pfn;
    std::map<std::string, DOMNode *> mem_lfn;
    std::map<std::string, DOMNode *> mem_fid;
    typedef std::map<std::string, DOMNode *>::iterator  mem_iter;
    
  };

} /* ns pool */

#endif 
