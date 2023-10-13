//
// Created by Carlos on 10/5/23.
//

#ifndef STRING_H
#define STRING_H

#include "valrep.h"
#include "value.h"

class Value;
class Location;

class String : public ValRep {
  private:
    std::string m_string;

  public:
    String(std::string a_string);
    virtual ~String();

    Value substr(int start, int end, Location loc) const;
    Value strcat(Value b_string) const;
    int strlen() const {
      return m_string.length();
    };
    std::string get_actual_string();
};


#endif //STRING_H
