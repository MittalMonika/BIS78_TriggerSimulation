#include <string>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include "QatDataAnalysis/OptParse.h"
#include "QatDataAnalysis/HistogramManager.h"
#include "QatDataAnalysis/IOLoader.h"
#include "QatDataAnalysis/IODriver.h"


inline std::string getDriver(std::string driverName) {
  // Rules:
  if (driverName!="") {
    return driverName;
  }
  else {
    char *dn=getenv("QAT_IO_DRIVER");
    if (dn) {
      return std::string(dn);
    }
    else {
      char *cmt=getenv("CMTPATH");
      if (cmt) {
	return "RootDriver";
      }
      else {
	return "HDF5Driver";
      }
    }
  }
}



class HIOZeroToOne::Clockwork {
public:
  IOLoader        loader;
  const IODriver *driver;
};

HIOZeroToOne::HIOZeroToOne(const std::string & driver):output(NULL), verbose(false), m_c(new Clockwork) {
  m_c->driver = m_c->loader.ioDriver(getDriver(driver));
  if (!m_c->driver) throw std::runtime_error("Warning, could not open a driver. Possible installation problem");
}

HIOZeroToOne::~HIOZeroToOne() {
  if (output) m_c->driver->write(output);
  if (output) m_c->driver->close(output);
  delete m_c;
}

void HIOZeroToOne::optParse(int & argc, char ** & argv) {
  for (int i=1; i<argc;i++) {
    if (std::string(argv[i])=="-o") {
      i++;
      if (i<argc) {
	if (output) {
	  throw std::runtime_error("HIOZeroToOne two output files specified" );
	}
	else {
	  output    = m_c->driver->newManager (argv[i]);
	  std::copy (argv+i+1, argv+argc, argv+i-1);
	  argc -=2;
	  i    -=2;
	}
      }
      else {
	throw std::runtime_error("Cannot parse output file" );
      }
    }
    else if (std::string(argv[i])=="-v") {
      verbose=true;
      std::copy(argv+i+1, argv+argc, argv+i);
      argc -= 1;
      i    -= 1;
    }
  
  }
  if (!output) throw std::runtime_error("HIOZeroToOne options parse error" );
}


class HIOOneToOne::Clockwork {
public:
  IOLoader        loader;
  const IODriver *driver;
};

HIOOneToOne::HIOOneToOne(const std::string & driver):input(NULL),output(NULL), verbose(false), m_c(new Clockwork) {
  m_c->driver = m_c->loader.ioDriver(getDriver(driver));
  if (!m_c->driver) throw std::runtime_error("Warning, could not open a driver. Possible installation problem");
}

HIOOneToOne::~HIOOneToOne() {
  if (output) m_c->driver->write(output);
  if (output) m_c->driver->close(output);
  if (input) m_c->driver->close(input);
  delete m_c;
}

void HIOOneToOne::optParse(int & argc, char ** & argv) {
  for (int i=1; i<argc;i++) {
    if (std::string(argv[i])=="-o") {
      i++;
      if (i<argc) {
	if (output) {
	  throw std::runtime_error("HIOOneToOne two output files specified" );
	}
	else {
	  output    = m_c->driver->newManager (argv[i]);
	  std::copy (argv+i+1, argv+argc, argv+i-1);
	  argc -=2;
	  i    -=2;
	}
      }
      else {
	throw std::runtime_error("Cannot parse output file" );
      }
    }
    else if (std::string(argv[i])=="-i") {
      i++;
      if (i<argc) {
	if (input) {
	  throw std::runtime_error("HIOOneToOne two input files specified" );
	}
	else {
	  input    = m_c->driver->openManager (argv[i]);
	  std::copy (argv+i+1, argv+argc, argv+i-1);
	  argc -=2;
	  i    -=2;
	}
      }
      else {
	throw std::runtime_error("Cannot parse input file" );
      }
    }
    else if (std::string(argv[i])=="-v") {
      verbose=true;
      std::copy(argv+i+1, argv+argc, argv+i);
      argc -= 1;
      i    -= 1;
    }
  
  }
  if (!output) throw std::runtime_error("HIOOneToOne options parse error" );
  if (!input)  throw std::runtime_error("HIOOneToOne no input file" );
}

class HIOOneToZero::Clockwork {
public:
  IOLoader        loader;
  const IODriver *driver;
};

HIOOneToZero::HIOOneToZero(const std::string & driver):input(NULL), verbose(false), m_c(new Clockwork) {
  m_c->driver = m_c->loader.ioDriver(getDriver(driver));
  if (!m_c->driver) throw std::runtime_error("Warning, could not open a root driver. Possible installation problem");
}

HIOOneToZero::~HIOOneToZero() {
  if (input) m_c->driver->close(input);
  delete m_c;
}

void HIOOneToZero::optParse(int & argc, char ** & argv) {
  for (int i=1; i<argc;i++) {
    if (std::string(argv[i])=="-i") {
      i++;
      if (i<argc) {
	if (input) {
	  throw std::runtime_error("HIOOneToZero two input files specified" );
	}
	else {
	  input    = m_c->driver->openManager (argv[i]);
	  std::copy (argv+i+1, argv+argc, argv+i-1);
	  argc -=2;
	  i    -=2;
	}
      }
      else {
	throw std::runtime_error("Cannot parse input file" );
      }
    }
    else if (std::string(argv[i])=="-v") {
      verbose=true;
      std::copy(argv+i+1, argv+argc, argv+i);
      argc -= 1;
      i    -= 1;
    }
  
  }
  if (!input)  throw std::runtime_error("HIOOneToZero no input file" );
}


class HIONToOne::Clockwork {
public:
  IOLoader        loader;
  const IODriver *driver;
};

HIONToOne::HIONToOne(const std::string & driver):output(NULL), verbose(false), m_c(new Clockwork) {
  m_c->driver = m_c->loader.ioDriver(getDriver(driver));
  if (!m_c->driver) throw std::runtime_error("Warning, could not open a root driver. Possible installation problem");
}

HIONToOne::~HIONToOne() {
  if (output) m_c->driver->write(output);
  if (output) m_c->driver->close(output);
  for (unsigned int i=0;i<input.size();i++) m_c->driver->close(input[i]);
  delete m_c;
}

void HIONToOne::optParse(int & argc, char ** & argv) {
  for (int i=1; i<argc;i++) {
    if (std::string(argv[i])=="-o") {
      i++;
      if (i<argc) {
	if (output) {
	  throw std::runtime_error("HIONToOne two output files specified" );
	}
	else {
	  output    = m_c->driver->newManager (argv[i]);
	  std::copy (argv+i+1, argv+argc, argv+i-1);
	  argc -=2;
	  i    -=2;
	}
      }
      else {
	throw std::runtime_error("Cannot parse output file" );
      }
    }
    else if (std::string(argv[i])=="-v") {
      verbose=true;
      std::copy(argv+i+1, argv+argc, argv+i);
      argc -= 1;
      i    -= 1;
    }
    else {
      if (std::string(argv[i]).find('=')==std::string(argv[i]).npos) {
	const HistogramManager *manager=m_c->driver->openManager(argv[i]);
	if (manager) input.push_back(manager);
	std::copy(argv+i+1, argv+argc, argv+i);
	argc -= 1;
	i    -= 1;
      }
    }
  }
  if (!input.size()) throw std::runtime_error("HIONToOne no input file" );
  if (!output) throw std::runtime_error("HIONToOne options parse error" );
}

class HIONToZero::Clockwork {
public:
  IOLoader        loader;
  const IODriver *driver;
};

HIONToZero::HIONToZero(const std::string & driver):verbose(false), m_c(new Clockwork) {
  m_c->driver = m_c->loader.ioDriver(getDriver(driver));
  if (!m_c->driver) throw std::runtime_error("Warning, could not open a root driver. Possible installation problem");
}

HIONToZero::~HIONToZero() {
  for (unsigned int i=0;i<input.size();i++) m_c->driver->close(input[i]);
  delete m_c;
}

void HIONToZero::optParse(int & argc, char ** & argv) {
  for (int i=1; i<argc;i++) {
    if (std::string(argv[i])=="-v") {
      verbose=true;
      std::copy(argv+i+1, argv+argc, argv+i);
      argc -= 1;
      i    -= 1;
    }
    else {
      if (std::string(argv[i]).find('=')==std::string(argv[i]).npos) {
	const HistogramManager *manager=m_c->driver->openManager(argv[i]);
	if (manager) input.push_back(manager);
	std::copy(argv+i+1, argv+argc, argv+i);
	argc -= 1;
	i    -= 1;
      }
    }
  }
  if (!input.size()) throw std::runtime_error("HIONToZero no input file" );
}


class NumericInput::Clockwork {
public:



  struct InputData {
    std::string name;
    std::string doc;
    double value;
  };


  std::vector<InputData> inputData;
  bool modParameter (const std::string & modifier);

};

bool NumericInput::Clockwork::modParameter (const std::string & modifier) {
  // Parse modifier
  size_t pos = modifier.find('=');
  if (pos==modifier.npos) {
    return false;
  }
  else {
    std::string variable = modifier.substr(0,pos);
    std::string value    = modifier.substr(pos+1,modifier.npos);
    std::istringstream   valueStream(value);
    double val;
    valueStream >> val;
    if (!valueStream) return false;
    for (unsigned int i=0;i<inputData.size();i++) {
      std::string trimmedVar= (inputData[i].name.substr(0,inputData[i].name.find(' ')));
      if (trimmedVar==variable) {
	inputData[i].value=val;
	return true;
      }
    }
    
    return false;
  }
}


NumericInput::NumericInput() :m_c(new Clockwork()) {
}

NumericInput::~NumericInput() {
  delete m_c;
}


std::string NumericInput::usage() const {
  std::ostringstream stream;
  stream << "Parameter usage" << std::endl;
  for (unsigned int i = 0; i< m_c->inputData.size();i++) {
    stream << m_c->inputData[i].name << " " << m_c->inputData[i].doc << '\t' << m_c->inputData[i].value << std::endl;
  }  
  return stream.str();

}
void NumericInput::optParse(int & argc, char ** & argv) {
  for (int i=1; i<argc;i++) {
    std::string thisArg(argv[i]);
    if (thisArg.find("=")!=thisArg.npos) {
      if (!m_c->modParameter(thisArg)) {
	      throw std::runtime_error("Cannot modify parameter "+ thisArg);
      } else {
	      std::copy(argv+i+1, argv+argc, argv+i);
	      argc -= 1;
	      i    -= 1;
      }
    }
  }
}


void NumericInput::declare(const std::string & name, const std::string & doc, double val){
  Clockwork::InputData data;
  data.name=name;
  data.doc =doc;
  data.value=val;
  m_c->inputData.push_back(data);
}

double NumericInput::getByName  (const std::string & name) const {
  for (unsigned int i=0;i<m_c->inputData.size();i++) {
    std::string trimmedVar= (m_c->inputData[i].name.substr(0,m_c->inputData[i].name.find(' ')));
    if (trimmedVar==name) {
      return m_c->inputData[i].value;
    }
  }
  //something wrong if you reached here
  throw std::runtime_error("Cannot parse parameter " + name);
}

