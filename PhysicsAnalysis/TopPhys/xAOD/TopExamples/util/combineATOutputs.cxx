/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include <TROOT.h>
#include <TFile.h>
#include <TList.h>
#include <TKey.h>
#include <TString.h>
#include <TObject.h>
#include <TTree.h>

#include <string>
#include <iostream>
#include <algorithm>

// for debugging
// #define XXX std::cout << "I am here: " << __FILE__ << ":" << __LINE__ << std::endl;

// this function is adapted from René Brun's $ROOTSYS/tutorials/io/copyFiles.C macro
void CopyDirToNewDir(TDirectory *source, TDirectory * outFile, TString prefix="") {
    //copy all objects and subdirs of directory source as a subdir of the current directory
//     source->ls();
    outFile->cd();
    //loop on all entries of this directory
    TKey *key;
    TIter nextkey(source->GetListOfKeys());
    while ((key = (TKey*)nextkey())) {
        const char *classname = key->GetClassName();
        TClass *cl = gROOT->GetClass(classname);
        if (!cl) continue;
        if (cl->InheritsFrom(TDirectory::Class())) {
            source->cd(key->GetName());
            TDirectory *subdir = gDirectory;
            TString name = prefix;
            if (prefix != "") name.Append("_");
            name+=subdir->GetName();
            TDirectory *adir = outFile->mkdir(name);
            outFile->cd();
            CopyDirToNewDir(subdir,adir); // no prefix for sub-directories
            outFile->cd();
        } else if (cl->InheritsFrom(TTree::Class())) {
            TTree *T = (TTree*)source->Get(key->GetName());
            outFile->cd();
            TTree *newT = T->CloneTree(-1,"fast");
            TString name = prefix;
            if (prefix != "") name.Append("_");
            name+=newT->GetName();
            newT->SetName(name);
            newT->Write();
        } else {
            source->cd();
            TObject *obj = key->ReadObj();
            outFile->cd();
            TString name = prefix;
            if (prefix != "") name.Append("_");
            name+=obj->GetName();
            obj->Write(name);
            delete obj;
        }
    }
}

int main(int argc, char** argv) {
  
    if (argc != 3) {
        std::cout<<"Usage:"<<std::endl;
        std::cout<<"       combineATOuputs outputFile inputFile1[:prefix1][,inputFile2[:prefix2][,inputFile3[:prefix3]]]"<<std::endl;
        std::cout<<"Examples:"<<std::endl;
        std::cout<<"       combineATOuputs out_combined.root out_EMTopo.root:EMTopo,out_EMPflow.root:EMPflow"<<std::endl;
        std::cout<<"       combineATOuputs out_combined.root out_EMTopo.root,out_EMPflow.root:EMPflow"<<std::endl;
        exit(1);
    }
    
    // processing the input files string
    std::string outString(argv[2]);
    std::vector<std::string> tempStringVector;
    
    // looping on the input files string and split it
    std::string::size_type start = 0, end = 0;
    while ((end = outString.find(",", start)) != std::string::npos) {
        std::string token = outString.substr(start, end - start);
        if(token.size()) tempStringVector.push_back(token);
        start = end + 1;
    }
    tempStringVector.push_back(outString.substr(start, end - start));// the last element
    
    // separate in intput files and prefixes
    std::vector<std::string> inFileNames;
    std::vector<std::string> prefixes;
    for (auto s: tempStringVector) {
        std::vector<std::string> tmp;
        std::string::size_type st = 0, ed = 0;
        while ((ed = s.find(":", st)) != std::string::npos) {
            std::string tk = s.substr(st, ed - st);
            if(tk.size()) tmp.push_back(tk);
            st = ed + 1;
        }
        tmp.push_back(s.substr(st, ed - st));// the last element
        if (tmp.size()>2) {
            std::cout<<"Error: can't interpret "<<s<<std::endl;
            exit(1);
        }
        std::string tmp_fileName = tmp[0];
        std::string tmp_prefix = tmp.size()==2?tmp[1]:""; // if no prefix is provided, empty prefix is assumed
        
        // checking if the file or prefix is not already used
        if (std::find(inFileNames.begin(), inFileNames.end(), tmp_fileName) != inFileNames.end()) {
            std::cout<<"Error: can't use twice the same input file "<<tmp_fileName<<std::endl;
            exit(1);
        }
        if (std::find(prefixes.begin(), prefixes.end(), tmp_prefix) != prefixes.end()) {
            std::cout<<"Error: can't use twice the same prefix "<<tmp_prefix<<std::endl;
            exit(1);
        }
        
        // now adding the input files and prefixed in the vectors
        inFileNames.push_back(tmp_fileName);
        prefixes.push_back(tmp_prefix);
    }
    
    // the output file
    TFile * fout = new TFile(argv[1],"create");
    
    // now looping on the input files
    for (unsigned int i = 0; i<inFileNames.size(); i++) {
        TFile * f = new TFile(inFileNames[i].c_str(),"read");
        std::cout<<"Copy content of file "<<inFileNames[i]<<" in file "<<argv[1]<<" with prefix "<<prefixes[i]<<std::endl;
        CopyDirToNewDir(f, fout, TString(prefixes[i]));
        f->Close();
    }
    fout->Close();

    
    return 0;
}
