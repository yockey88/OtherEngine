/**
 * \file reflection/type_database.cpp
 **/
#include "reflection/type_database.hpp"

namespace dotother {
namespace echo {

  TypeDatabase* TypeDatabase::instance = nullptr;  
  
  TypeDatabase& TypeDatabase::Instance() {
    if (instance == nullptr) {
      instance = new TypeDatabase;
    }
    return *instance;
  }
      
  void TypeDatabase::CloseDatabase() {
    if (instance != nullptr) {
      delete instance;
      instance = nullptr;
    }
  }

} // namespace echo
} // namespace dotother
