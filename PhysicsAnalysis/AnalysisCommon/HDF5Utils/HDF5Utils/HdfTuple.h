// this is -*- C++ -*-
/*
Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HDF_TUPLE_HH
#define HDF_TUPLE_HH

/**
 * HDF5 Tuple Writer
 *
 * This is a tool to write N-dimensional arrays of compound data types
 * to HDF5 files.
 *
 * Skip down to the `WriterXd` and `VariableFillers` classes below to
 * see the stuff that you'll have to interact with.
 **/

#include "H5Cpp.h"

#include <functional>
#include <vector>
#include <memory>

namespace H5Utils {

  /// @brief Internal clssses and code
  /// @{

  namespace internal {

    /** @brief data_buffer_t
     *
     * This buffer element is used by the HDF5 library to store data
     * which is about to be written to disk.
     **/
    union data_buffer_t
    {
      int _int;
      long long _llong;
      unsigned int _uint;
      unsigned char _uchar;
      float _float;
      double _double;
      bool _bool;
    };

    /**
     * We have lots of code to get around HDF5's rather weak typing. These
     * templates are specialized in the cxx file
     **/

    /// templates to map primative types to HDF5 data types
    template <typename T> H5::DataType get_type();
    template<> H5::DataType get_type<int>();
    template<> H5::DataType get_type<long long>();
    template<> H5::DataType get_type<unsigned int>();
    template<> H5::DataType get_type<unsigned char>();
    template<> H5::DataType get_type<float>();
    template<> H5::DataType get_type<double>();
    template<> H5::DataType get_type<bool>();

    // check to make sure one of the above specializations is used
    template<typename T>
    H5::DataType get_type() {
      static_assert(sizeof(T) != sizeof(T), "you must override this class");
      return H5::DataType();
    }

    /// templates to cast the union into the desired type
    template <typename T>
    T& get_ref(data_buffer_t& buf);
    template<> int& get_ref<int>(data_buffer_t& buf);
    template<> long long& get_ref<long long>(data_buffer_t& buf);
    template<> unsigned int& get_ref<unsigned int>(data_buffer_t& buf);
    template<> unsigned char& get_ref<unsigned char>(data_buffer_t& buf);
    template<> float& get_ref<float>(data_buffer_t& buf);
    template<> double& get_ref<double>(data_buffer_t& buf);
    template<> bool& get_ref<bool>(data_buffer_t& buf);

    // check to make sure one of the above specializations is used
    template <typename T>
    T& get_ref(data_buffer_t& buf) {
      static_assert(sizeof(T) != sizeof(T), "you must override this class");
      return T();
    }

    /** @brief Buffer element harvester
     *
     * This is used to buld the unions we use to build the hdf5 memory
     * buffer. Together with the `get_type` templates it gives us a
     * strongly typed HDF5 writer.
     **/
    template <typename T>
    data_buffer_t get_buffer_from_func(const std::function<T()>& func) {
      data_buffer_t buffer;
      get_ref<T>(buffer) = func();
      return buffer;
    }

    /** @brief Variable fillers
     *
     * A variable filler instance encloses several things: a function
     * which harvests the output element, and the name and type of the
     * input. In most cases you probably don't have to work with this
     * directly, and can work with the `VariableFillers` container
     * below.
     **/
    class IVariableFiller
    {
    public:
      virtual ~IVariableFiller() {}
      virtual internal::data_buffer_t get_buffer() const = 0;
      virtual H5::DataType get_type() const = 0;
      virtual std::string name() const = 0;
    };

    /// implementation for variable filler
    template <typename T>
    class VariableFiller: public IVariableFiller
    {
    public:
      VariableFiller(const std::string&, const std::function<T()>&);
      internal::data_buffer_t get_buffer() const;
      H5::DataType get_type() const;
      std::string name() const;
    private:
      std::function<T()> m_getter;
      std::string m_name;
    };
    template <typename T>
    VariableFiller<T>::VariableFiller(const std::string& name,
                                      const std::function<T()>& func):
      m_getter(func),
      m_name(name)
    {
    }
    template <typename T>
    internal::data_buffer_t VariableFiller<T>::get_buffer() const {
      return internal::get_buffer_from_func<T>(m_getter);
    }
    template <typename T>
    H5::DataType VariableFiller<T>::get_type() const {
      return internal::get_type<T>();
    }
    template <typename T>
    std::string VariableFiller<T>::name() const {
      return m_name;
    }

  }
  /// @}



  /** @brief Variable filler arrays
   *
   * This is what you actually interact with.
   *
   * The elements added to this container specify one compound element
   * in the output HDF5 file. You need to give each variable a name
   * and a function that fills the variable. Note that these functions
   * have no inputs, but they can close over whatever buffers you want
   * to read from.
   *
   * For examples, see `copyRootTree.cxx`
   **/
  /// @{
  class VariableFillers:
    public std::vector<std::shared_ptr<internal::IVariableFiller> >
  {
  public:
    /** @brief Variable add method
     *
     * This should be the only method you need in this class. The
     * function should provide one output element, and will be called
     * with no arguments at least once each time the output is filled.
     **/
    template <typename T>
    void add(const std::string& name, const std::function<T()>&);
  };

  template <typename T>
  void VariableFillers::add(const std::string& name,
                            const std::function<T()>& fun) {
    using internal::VariableFiller;
    this->push_back(std::make_shared<VariableFiller<T> >(name, fun));
  }
  /// @}


  /** @brief WriterXd
   *
   * Writer class
   *
   * This is the other thing you interact with.
   *
   * You'll have to specify the H5::Group to write the dataset to, the
   * name of the new dataset, and the extent of the dataset.
   *
   * To fill, use the `fill_while_incrementing` function, which will
   * iterate over all possible values of `indices` within the dataset
   * dimensions. The variable filler is called once for each index
   * combination.
   **/

  class WriterXd {
  public:
    WriterXd(H5::Group& group, const std::string& name,
             VariableFillers fillers,
             std::vector<hsize_t> dataset_dimensions,
             hsize_t chunk_size = 2048);
    WriterXd(const WriterXd&) = delete;
    WriterXd& operator=(WriterXd&) = delete;
    ~WriterXd();
    void fillWhileIncrementing(
      std::vector<size_t>& indices = WriterXd::NONE);
    void flush();
  private:
    static std::vector<size_t> NONE;
    hsize_t buffer_size() const;
    H5::CompType m_type;
    std::vector<hsize_t> m_max_length;
    std::vector<hsize_t> m_dim_stride;
    hsize_t m_batch_size;
    hsize_t m_offset;
    std::vector<internal::data_buffer_t> m_buffer;
    VariableFillers m_fillers;
    H5::DataSet m_ds;
  };

}
#endif
