#include <boost/array.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>

namespace anl
{

struct mapped_1_type {};
struct mapped_pair_type {};
struct mapped_tuple_type {};

struct tuple_index_nonzero {};
struct tuple_index_zero {};

template <typename T2>
struct size_mapped {
  typedef mapped_1_type type;
  static const int value = 1;
};

template <typename T2a, typename T2b>
struct size_mapped<std::pair<T2a, T2b> > {
  typedef mapped_pair_type type;
  static const int value = 2;
};

#ifndef TUPLE_MAX_ARG
#define TUPLE_MAX_ARG 10
#endif

#define SIZE_MAPPED_TUPLE_DEFINITION(z, n, unused) \
  template < BOOST_PP_ENUM_PARAMS(n, typename T) >                      \
  struct size_mapped<boost::tuple< BOOST_PP_ENUM_PARAMS(n, T) > >{      \
    typedef boost::tuple< BOOST_PP_ENUM_PARAMS(n, T) > tuple_type;      \
    typedef mapped_tuple_type type;                                     \
    static const int value = boost::tuples::length<tuple_type>::value;  \
  };

BOOST_PP_REPEAT_FROM_TO(1, TUPLE_MAX_ARG, SIZE_MAPPED_TUPLE_DEFINITION, ~)

// template <typename T2a>
// struct size_mapped<boost::tuple<T2a> >{
//   typedef boost::tuple<T2a> tuple_type;
//   typedef mapped_tuple_type type;
//   static const int value = boost::tuples::length<tuple_type>::value;
// };

template <int i>
struct tuple_index {
  typedef tuple_index_nonzero type;
  static const int value = i;
};

template <>
struct tuple_index<0> {
  typedef tuple_index_zero type;
  static const int value = 0;
};


template <typename T2>
class ModuleParameter<std::map<std::string, T2> > : public VModuleParameter
{
  typedef typename std::pair<std::string, T2> value_type;
  typedef typename std::map<std::string, T2> map_type;
  typedef typename std::string key_type;
  typedef T2 mapped_type;
  typedef typename map_type::iterator iter_type;
  typedef typename size_mapped<mapped_type>::type value_catagory;
 
  static const int MappedSize = size_mapped<mapped_type>::value;
  static const int MappedIndex = MappedSize-1;
  
public:
  ModuleParameter(map_type* ptr, const std::string& name,
                  const std::string& key_name,
                  const std::string& key_default)
    : VModuleParameter(name), _ptr(ptr),
      _key_name(key_name), _key_tmp(key_default),
      _first_input(true)
  {
    set_default_string(key_default);
  }
  
  ModuleParameter(map_type* ptr, const std::string& name,
                  const std::string& key_name,
                  const std::string& key_default,
                  const std::vector<ModParam>& values_info)
    : VModuleParameter(name), _ptr(ptr),
      _key_name(key_name), _key_tmp(key_default), _value_info(values_info),
      _first_input(true)
  {
    set_default_string(key_default);
  }

  std::string type_name() const { return "map"; }

  bool first_input()
  {
    if (_first_input) {
      _first_input = false;
      return true;
    }
    return false;
  }

  virtual bool ask()
  {
    typedef typename tuple_index<MappedIndex>::type tuple_index_type;

    std::cout << "Define table of " << name() << ":" << std::endl;
    ModuleParameter<std::string> tmpKeyParam(&_key_tmp, _key_name);
    tmpKeyParam.set_question(name()+" (OK for exit)");
    
    _ptr->clear();
    if (first_input()) { set_default_map_value(); }

    while (1) {
      tmpKeyParam.ask();
      if (_key_tmp=="ok" || _key_tmp=="OK") break;
      
      set_mapped<MappedIndex>(&_default_value, value_catagory(),
                              tuple_index_type());
      
      const std::size_t NumValue = _value_info.size();
      for (std::size_t i=0; i<NumValue; ++i) {
        if (value_enable(i, value_catagory())) {
          _value_info[i]->ask();
        }
      }
      
      mapped_type tmpValue;
      get_mapped<MappedIndex>(&tmpValue, value_catagory(),
                              tuple_index_type());
      _ptr->insert(std::make_pair(_key_tmp, tmpValue));
      _key_tmp = "OK";
    }
    return true;
  }
  
  void output(std::ostream& os) const
  {
    typedef typename tuple_index<MappedIndex>::type tuple_index_type;

    os << '\n';
    for (iter_type it=_ptr->begin(); it!=_ptr->end(); ++it) {
      os << "  " << _key_name << " : " << it->first << '\n';
      mapped_type tmpValue = it->second;
      set_mapped<MappedIndex>(&tmpValue, value_catagory(), tuple_index_type());
      output_mapped<MappedIndex>(os, value_catagory(), tuple_index_type());
    }
    os.flush();
  }
  
  void input(std::istream& is)
  {
    ;
  }
  
  std::string map_key_name() const { return _key_name; }
  void set_map_key(const std::string& key) { _key_tmp = key; }
  size_t num_map_value() const { return _value_info.size(); }
  boost::shared_ptr<VModuleParameter const> get_map_value(size_t i) const { return _value_info[i]; }
  void add_map_value(ModParam param) { _value_info.push_back(param); }

  void set_default_map_value()
  {
    typedef typename tuple_index<MappedIndex>::type tuple_index_type;
    get_mapped<MappedIndex>(&_default_value,
                            value_catagory(), tuple_index_type());
  }

  void enable_map_value(int type, const std::vector<int>& enables)
  {
    boost::array<int, MappedSize> enableArray;
    for (int i=0; i<MappedSize; ++i) {
      enableArray[i] = 0;
    }
    for (std::size_t k=0; k<enables.size(); ++k) {
      int ena = enables[k];
      if (ena < MappedSize) {
        enableArray[ena] = 1;
      }
    }
    _value_enable[type] = enableArray;
  }
  
  bool value_enable(int i, mapped_1_type) const { return true; }
  bool value_enable(int i, mapped_pair_type) const { return true; }
  bool value_enable(int i, mapped_tuple_type) const
  {
    if (i>0) {
      int type = 0;
      _value_info[0]->get(&type);
      typename std::map<int, boost::array<int, MappedSize> >::const_iterator it
        = _value_enable.find(type);
      if (it!=_value_enable.end() && (it->second)[i]==0) {
        return false;
      }
    }
    return true;
  }

  void set_map_value(const std::string& name, int val)
  { set_map_value_impl(name, val); }

  void set_map_value(const std::string& name, double val)
  { set_map_value_impl(name, val); }

  void set_map_value(const std::string& name, const std::string& val)
  { set_map_value_impl(name, val); }

  void insert_map()
  {
    typedef typename tuple_index<MappedIndex>::type tuple_index_type;
    mapped_type tmpValue;
    get_mapped<MappedIndex>(&tmpValue, value_catagory(), tuple_index_type());
    _ptr->insert(std::make_pair(_key_tmp, tmpValue));
    
    set_mapped<MappedIndex>(&_default_value, value_catagory(),
                            tuple_index_type());
  }
  
private:
  template<int i>
  void set_mapped(mapped_type* pval, mapped_1_type, tuple_index_zero) const
  {
    _value_info[0]->set(pval);
  }

  template<int i>
  void set_mapped(mapped_type* pval, mapped_pair_type, tuple_index_nonzero) const
  {
    _value_info[0]->set(&(pval->first));
    _value_info[1]->set(&(pval->second));
  }
  
  template<int i>
  void set_mapped(mapped_type* pval, mapped_tuple_type, tuple_index_nonzero) const
  {
    typedef typename tuple_index<i-1>::type next_index_type;
    set_mapped<i-1>(pval, mapped_tuple_type(), next_index_type());
    _value_info[i]->set(&(boost::tuples::get<i>(*pval)));
  }
  
  template<int i>
  void set_mapped(mapped_type* pval, mapped_tuple_type, tuple_index_zero) const
  {
    _value_info[i]->set(&(boost::tuples::get<i>(*pval)));
  }
  
  template<int i>
  void get_mapped(mapped_type* pval, mapped_1_type, tuple_index_zero) const
  {
    _value_info[0]->get(pval);
  }

  template<int i>
  void get_mapped(mapped_type* pval, mapped_pair_type, tuple_index_nonzero) const
  {
    _value_info[0]->get(&(pval->first));
    _value_info[1]->get(&(pval->second));
  }
  
  template<int i>
  void get_mapped(mapped_type* pval, mapped_tuple_type, tuple_index_nonzero) const
  {
    typedef typename tuple_index<i-1>::type next_index_type;
    get_mapped<i-1>(pval, mapped_tuple_type(), next_index_type());
    _value_info[i]->get(&(boost::tuples::get<i>(*pval)));
  }

  template<int i>
  void get_mapped(mapped_type* pval, mapped_tuple_type, tuple_index_zero) const
  {
    _value_info[i]->get(&(boost::tuples::get<i>(*pval)));
  }

  template<int i>
  void output_mapped(std::ostream& os, mapped_1_type, tuple_index_zero) const
  {
    os << "    "; _value_info[0]->print(os); os << '\n';
  } 

  template<int i>
  void output_mapped(std::ostream& os, mapped_pair_type, tuple_index_nonzero) const
  {
    os << "    "; _value_info[0]->print(os); os << '\n';
    os << "    "; _value_info[1]->print(os); os << '\n';
  } 

  template<int i>
  void output_mapped(std::ostream& os, mapped_tuple_type,
                     tuple_index_nonzero) const
  {
    typedef typename tuple_index<i-1>::type next_index_type;
    output_mapped<i-1>(os, mapped_tuple_type(), next_index_type());
    if (value_enable(i, mapped_tuple_type())) {
      os << "    "; _value_info[i]->print(os); os << '\n';
    }
  }
  
  template<int i>
  void output_mapped(std::ostream& os, mapped_tuple_type,
                     tuple_index_zero) const
  {
    if (value_enable(i, mapped_tuple_type())) {
      os << "    "; _value_info[i]->print(os); os << '\n';
    }
  }

  template <typename ValueT>
  void set_map_value_impl(const std::string& name, ValueT val)
  {
    typedef std::vector<ModParam>::iterator IterT;

    if (first_input()) { set_default_map_value(); }

    for (IterT it=_value_info.begin(); it!=_value_info.end(); ++it) {
      if ((*it)->name()==name) {
        (*it)->set_value(val);
        return;
      }
    }

    BOOST_THROW_EXCEPTION( ANLException() <<
                           ANLErrInfo(std::string("Parameter is not found: ")
                                      +name) );
  }

private:
  map_type* _ptr;
  std::string _key_name;
  std::string _key_tmp;
  mapped_type _default_value;
  std::vector<ModParam> _value_info;
  std::map<int, boost::array<int, MappedSize> > _value_enable;
  // std::map<int, boost::array<std::string, MappedSize> > _value_question;
  bool _first_input;
};

}
