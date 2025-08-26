#ifndef NDDEMJSON
#define NDDEMJSON
#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <cctype>
#include <cstdlib>

namespace nddem {

// Some previews
struct JsonValue;
using json = struct JsonValue ; 
class JsonObject ; 
using JsonArray  = std::vector<JsonValue>;

// =============================================================================
class JsonObject {
public:
  JsonValue & operator[] (std::string &s) ;
  JsonValue & at(std::string &s) ;
  
  // subclasses of iterators and data
  struct dat ; 
  struct iterator; 
  
  iterator begin() ;
  iterator end() ;
  bool exist (std::string key) ; 
  
  
  std::map<std::string, JsonValue> data;
} ; 

// =============================================================================
struct JsonValue 
{
    enum class Type { Null, Bool, Number, String, Array, Object } type;

    double number_value = 0.0;
    bool bool_value = false;
    std::string string_value;
    JsonArray array_value;
    JsonObject object_value;

    JsonValue() : type(Type::Null) {}
    JsonValue(double n) : type(Type::Number), number_value(n) {}
    JsonValue(bool b) : type(Type::Bool), bool_value(b) {}
    JsonValue(std::string s) : type(Type::String), string_value(std::move(s)) {}
    JsonValue(JsonArray a) : type(Type::Array), array_value(std::move(a)) {}
    JsonValue(JsonObject o) : type(Type::Object), object_value(std::move(o)) {}
    bool is_number () { return type==Type::Number ; }
    bool is_string () { return type==Type::String ; }
    bool is_array  () { return type==Type::Array ; }
    bool is_object () { return type==Type::Object ;}
    
    JsonValue & operator[] (std::string key) ;
    template <typename T> T get() {return ;}
    
    auto begin() 
    {
      if (type!=Type::Array) 
        throw std::runtime_error(std::string("Cannot iterate a non array"));
      return array_value.begin() ; 
    }
    auto end() 
    {
      if (type!=Type::Array)
        throw std::runtime_error(std::string("Cannot iterate a non array")); 
      return array_value.end() ;
    }
    size_t size() 
    {
      if (type!=Type::Array)
        throw std::runtime_error(std::string("Cannot size a non array")); 
      return array_value.size() ;
    }
    JsonValue & operator[] (size_t id)
    {
      if (type!=Type::Array)
        throw std::runtime_error(std::string("Cannot slice a non array")); 
      return array_value[id] ;
    }
    //JsonObject::iterator find(std::string key) ;
    bool exist (std::string key) ; 
    
    JsonObject & items() 
    {
      if (type!=Type::Object)
      {
        throw std::runtime_error(std::string("Cannot get map from a non object"));
      }
      return object_value ;
    }
    
    // Subclass JsonParser
    class JsonParser 
    {
      std::string text;
      size_t pos = 0;

      public:
        JsonValue parse(std::string txt) {text = txt ; return parse() ; }
        JsonValue parse() ;

      private:
        void skip_whitespace() { while (pos < text.size() && std::isspace(text[pos])) ++pos; }
        char peek() const { return pos < text.size() ? text[pos] : '\0'; }
        char get() { return pos < text.size() ? text[pos++] : '\0'; }
        void expect(char expected) { if (get() != expected) throw std::runtime_error(std::string("Expected '") + expected + "'");}
        JsonValue parse_value() ; 
        JsonValue parse_null() ;
        JsonValue parse_true() ; 
        JsonValue parse_false() ; 
        JsonValue parse_number() ;
        JsonValue parse_string() ;
        JsonValue parse_array() ;
        JsonValue parse_object() ;
  };
    
  static JsonValue parse(std::string &val) {JsonValue res ; JsonParser parser ; res = parser.parse(val) ; return res ; }
};
// end JsonValue

// =============================================================================
//----------------------- JsonObject impl ---------------------------------------
struct JsonObject::dat
{
public:
  dat(std::pair<std::string, JsonValue> p) {k = p.first ; v = p.second ; }
  std::string key() {return k; }
  JsonValue & value() {return v ; }
private: 
  std::string k ;
  JsonValue v ; 
} ;

struct JsonObject::iterator
{
  iterator(std::map<std::string,JsonValue>::iterator a) {it = a ; } 
  bool operator!=(iterator & o) {return it !=o.it; }
  iterator & operator++() {it ++ ; return *this ;}
  dat operator* () { return JsonObject::dat({it->first , it->second }) ; }
  
  private:
    std::map<std::string,JsonValue>::iterator it ; 
} ; 
  
JsonValue & JsonObject::operator[] (std::string &s) {return data[s] ; }
JsonValue & JsonObject::at(std::string &s) {return data.at(s) ; }
JsonObject::iterator JsonObject::begin() {return JsonObject::iterator({data.begin()}) ; }
JsonObject::iterator JsonObject::end() {return JsonObject::iterator({data.end()}) ; }
//JsonObject::iterator JsonObject::find(std::string key) { return JsonObject::iterator({data.find(key)}) ; }  
bool JsonObject::exist(std::string key) {auto res = data.find(key) ; if (res==data.end()) return false ; else return true ; } 

//------------------------- JsonValue impl -------------------------------------
JsonValue & JsonValue::operator[] (std::string key) 
{
if (type != Type::Object) throw std::runtime_error(std::string("Trying to access key in a non object"));
  return object_value.at(key) ; 
}

template<> JsonValue JsonValue::get<JsonValue>() {if (type!=Type::Object) std::runtime_error(std::string("Incorrect type number")); return object_value ; } 
template<> bool JsonValue::get<bool>() {if (type!=Type::Bool) std::runtime_error(std::string("Incorrect type number")); return bool_value ; } 
template<> int JsonValue::get<int>() {if (type!=Type::Number) std::runtime_error(std::string("Incorrect type number")); return number_value ; } 
template<> double JsonValue::get<double>() {if (type!=Type::Number) std::runtime_error(std::string("Incorrect type number")); return number_value ; } 
template<> std::string JsonValue::get<std::string>() {if (type!=Type::String) std::runtime_error(std::string("Incorrect type string")); return string_value ;} 
template<> std::vector<double> JsonValue::get<std::vector<double>>()
{
  if (type!=Type::Array) std::runtime_error(std::string("Incorrect type array"));
  std::vector<double> res ; 
  res.resize(array_value.size()) ; 
  for (size_t i = 0 ; i<array_value.size() ; i++)
  {
    res[i] = array_value[i].get<double>() ; 
  }
  return res ;   
}
template<> std::vector<int> JsonValue::get<std::vector<int>>()
{
  if (type!=Type::Array) std::runtime_error(std::string("Incorrect type array"));
  std::vector<int> res ; 
  res.resize(array_value.size()) ; 
  for (size_t i = 0 ; i<array_value.size() ; i++)
  {
    res[i] = array_value[i].get<int>() ; 
  }
  return res ;   
}
template<> std::vector<bool> JsonValue::get<std::vector<bool>>()
{
  if (type!=Type::Array) std::runtime_error(std::string("Incorrect type array"));
  std::vector<bool> res ; 
  res.resize(array_value.size()) ; 
  for (size_t i = 0 ; i<array_value.size() ; i++)
  {
    res[i] = array_value[i].get<bool>() ; 
  }
  return res ;   
}
template<> std::vector<std::string> JsonValue::get<std::vector<std::string>>()
{
  if (type!=Type::Array) std::runtime_error(std::string("Incorrect type array"));
  std::vector<std::string> res ; 
  res.resize(array_value.size()) ; 
  for (size_t i = 0 ; i<array_value.size() ; i++)
  {
    res[i] = array_value[i].get<std::string>() ; 
  }
  return res ;   
}
template<> std::vector<std::vector<double>> JsonValue::get<std::vector<std::vector<double>>>()
{
  if (type!=Type::Array) std::runtime_error(std::string("Incorrect type array"));
  std::vector<std::vector<double>> res ; 
  res.resize(array_value.size()) ; 
  for (size_t i = 0 ; i<array_value.size() ; i++)
  {
    res[i] = array_value[i].get<std::vector<double>>() ; 
  }
  return res ;   
}
template<> std::map<std::string,std::string> JsonValue::get<std::map<std::string,std::string>>()
{
  if (type!=Type::Object) std::runtime_error(std::string("Incorrect type object for mapping"));
  std::map<std::string,std::string> res ; 
  for (auto [key, value] : object_value.data)
    res[key] = value.get<std::string>() ; 
  
  return res ;   
}

/*JsonObject::iterator JsonValue::find(std::string key)
{
  if (type != Type::Object)
    throw std::runtime_error(std::string("Cannot find in a non object"));         
  return object_value.find(key) ; 
}*/
bool JsonValue::exist (std::string key)
{
  if (type != Type::Object)
    throw std::runtime_error(std::string("Cannot check existence in a non object"));         
  return object_value.exist(key) ; 
}

std::istream& operator>>(std::istream& in, JsonValue& value) {
    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    value=value.parse(content);
    return in ; 
}

std::ostream& operator<< (std::ostream& out, const JsonValue &j)
{
  switch(j.type)
  {
    case JsonValue::Type::Null: out << "Null" ; break ; 
    case JsonValue::Type::Bool: out << (j.bool_value?"True":"False") ; break ;
    case JsonValue::Type::Number: out << j.number_value ; break ;
    case JsonValue::Type::String: out << j.string_value ; break ; 
    case JsonValue::Type::Array: 
      out << "[" ; 
      for (auto & v: j.array_value)
        out << v << "," ;
      out << "]" ; 
      break ; 
    case JsonValue::Type::Object:
      out << "{ " ; 
      for (auto& [key, value] : j.object_value.data)
        out << "\"" << key << "\": " << value << ", \n" ;
      out << "}" ; 
      break ;       
  }
  return out ; 
}

//====================JsonParser impl ==========================================
JsonValue JsonValue::JsonParser::parse() 
{
  skip_whitespace();
  JsonValue result = parse_value();
  skip_whitespace();
  if (pos != text.size())
      throw std::runtime_error("Unexpected trailing characters");
  return result;
}
//---------------------------------------------
JsonValue JsonValue::JsonParser::parse_value() {
    skip_whitespace();
    char c = peek();
    if (c == '"') return parse_string();
    if (c == '-' || std::isdigit(c)) return parse_number();
    if (c == 't') return parse_true();
    if (c == 'f') return parse_false();
    if (c == 'n') return parse_null();
    if (c == '[') return parse_array();
    if (c == '{') return parse_object();
    throw std::runtime_error("Unexpected character in JSON input");
}
//---------------------------------------------
JsonValue JsonValue::JsonParser::parse_null() {
    expect('n');
    expect('u'); expect('l'); expect('l');
    return JsonValue();
}
//---------------------------------------------
JsonValue JsonValue::JsonParser::parse_true() {
    expect('t');
    expect('r'); expect('u'); expect('e');
    return JsonValue(true);
}
//---------------------------------------------
JsonValue JsonValue::JsonParser::parse_false() {
    expect('f');
    expect('a'); expect('l'); expect('s'); expect('e');
    return JsonValue(false);
}
//---------------------------------------------
JsonValue JsonValue::JsonParser::parse_number() {
    size_t start = pos;
    if (peek() == '-') ++pos;
    while (std::isdigit(peek())) ++pos;
    if (peek() == '.') {
        ++pos;
        while (std::isdigit(peek())) ++pos;
    }
    if (peek() == 'e' || peek() == 'E') {
        ++pos;
        if (peek() == '+' || peek() == '-') ++pos;
        while (std::isdigit(peek())) ++pos;
    }
    double value = std::stod(text.substr(start, pos - start));
    return JsonValue(value);
}
//---------------------------------------------
JsonValue JsonValue::JsonParser::parse_string() {
    expect('"');
    std::string result;
    while (peek() != '"') {
        char c = get();
        if (c == '\0') throw std::runtime_error("Unterminated string");
        result += c;
    }
    expect('"');
    return JsonValue(result);
}
//---------------------------------------------
JsonValue JsonValue::JsonParser::parse_array() {
    expect('[');
    skip_whitespace();
    JsonArray arr;
    if (peek() == ']') {
        get();
        return JsonValue(arr);
    }

    while (true) {
        skip_whitespace();
        arr.push_back(parse_value());
        skip_whitespace();
        if (peek() == ',') {
            get();
        } else if (peek() == ']') {
            get();
            break;
        } else {
            throw std::runtime_error("Expected ',' or ']'");
        }
    }

    return JsonValue(arr);
}
//---------------------------------------------
JsonValue JsonValue::JsonParser::parse_object() {
    expect('{');
    skip_whitespace();
    JsonObject obj;
    if (peek() == '}') {
        get();
        return JsonValue(obj);
    }

    while (true) {
        skip_whitespace();
        if (peek() != '"') throw std::runtime_error("Expected object key string");
        std::string key = parse_string().string_value;
        skip_whitespace();
        expect(':');
        skip_whitespace();
        obj[key] = parse_value();
        skip_whitespace();
        if (peek() == ',') {
            get();
        } else if (peek() == '}') {
            get();
            break;
        } else {
            throw std::runtime_error("Expected ',' or '}'");
        }
    }

    return JsonValue(obj);
}

//--------------------------------------------------
} // END namespace
#endif
