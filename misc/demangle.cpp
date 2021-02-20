// $Id: demangle.cpp,v 1.3 2020-11-29 12:05:00-08 - - $

// Demangle a typeid(X).name() string

#include <cstdlib>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

using namespace std;

#include <cxxabi.h>
template <typename type>
string demangle (const type &object) {
   const char *const name = typeid (object).name();
   int status = 0;
   using free_decl = void (*) (void*);
   unique_ptr<char,free_decl> result {
      abi::__cxa_demangle (name, nullptr, nullptr, &status),
      std::free,
   };
   return status == 0 ? result.get() : name; 
}

class foo { }; 
class bar: foo { }; 
class baz: bar { }; 
template <typename T> class tmpl { T x; };

template <typename type>
void print_demangled (const string &str) {
   type obj {};
   cout << str << " => " << sizeof obj << endl;
   cout << "   mangled:   " << typeid(obj).name() << endl;
   cout << "   demangled: " << demangle (obj) << endl;
}

template <typename type>
void print_demangled (const string &str, const type& obj) {
   print_demangled<type> (str);
   cout << "   value: " << obj << endl;
}


#define DEMANGLE_T(X) print_demangled<X> (#X)
#define DEMANGLE_V(X) print_demangled (#X, X)
int main() {
   using map_string_int = map<string,int>;
   DEMANGLE_T (bool);
   DEMANGLE_T (char);
   DEMANGLE_T (signed char);
   DEMANGLE_T (unsigned char);
   DEMANGLE_T (short);
   DEMANGLE_T (signed short);
   DEMANGLE_T (unsigned short);
   DEMANGLE_T (int);
   DEMANGLE_T (signed int);
   DEMANGLE_T (unsigned int);
   DEMANGLE_T (long);
   DEMANGLE_T (signed long);
   DEMANGLE_T (unsigned long);
   DEMANGLE_T (long long);
   DEMANGLE_T (float);
   DEMANGLE_T (double);
   DEMANGLE_T (long double);
   DEMANGLE_T (size_t);
   cout << "\f" << endl;
   DEMANGLE_T (foo);
   DEMANGLE_T (bar);
   DEMANGLE_T (baz);
   DEMANGLE_T (tmpl<int>);
   DEMANGLE_T (vector<string>);
   DEMANGLE_T (vector<int>);
   DEMANGLE_T (list<vector<long>>);
   DEMANGLE_T (map_string_int);
   DEMANGLE_V (__FILE__);
   DEMANGLE_V (__LINE__);
   DEMANGLE_V (__DATE__);
   DEMANGLE_V (__TIME__);
   DEMANGLE_V (__func__);
   DEMANGLE_V (__PRETTY_FUNCTION__);
   return 0;
}

//TEST// alias grind="valgrind --leak-check=full --show-reachable=yes"
//TEST// grind --log-file=demangle.log demangle >demangle.out 2>&1
//TEST// mkpspdf demangle.ps demangle.cpp* demangle.out demangle.log

