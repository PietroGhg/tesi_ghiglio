#pragma once
#include <string>
#include <map>
#include <cmath> //floor, sqrt
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/Support/raw_ostream.h"
#include "FilesUtils.h"

class FunctionLocation {
private:
  std::string Name;
  unsigned Line;
  unsigned Column;
  std::string File;
public:
  FunctionLocation() : Line(0), Column(0), File(""){}
  FunctionLocation(llvm::DISubprogram* SubPr);
  unsigned getLine() const { return Line; }
  unsigned getColumn() const { return Column; }
  bool operator<(const FunctionLocation& Other) const;
  const std::string& getFile() const { return File; }
  friend raw_ostream& operator<<(raw_ostream& os, const FunctionLocation& sl);
};

class SourceLocation {
 private:
  unsigned Line;
  unsigned Column;
  std::string File;
  FunctionLocation FunLoc;
 public:
  SourceLocation() : Line(0), Column(0), File("") {}
  SourceLocation(unsigned Line, unsigned Column, const std::string& File);
  SourceLocation(llvm::DILocation* Loc);
  SourceLocation(llvm::DISubprogram* SubPr);
  SourceLocation(unsigned long id,
		 std::map<unsigned, std::string>& idFileMap);
  bool operator==(const SourceLocation& Other) const;
  bool operator<(const SourceLocation& Other) const;
  unsigned getLine() const { return Line; }
  unsigned getColumn() const { return Column; }
  unsigned long getSingle(std::map<std::string, unsigned>& fileIDMap);
  const std::string& getFile() const { return File; }
  const FunctionLocation& getFunLoc() const { return FunLoc; }
  std::string toString() const ;
  friend raw_ostream& operator<<(raw_ostream& os, const SourceLocation& sl);
};




/** Support class to convert from 
 * (line, col, fileid) to an unique id, and viceversa
 */
class Tri{
  using int_t = unsigned long;
private:
  int_t first;
  int_t second;
  int_t third;
public:
  static pair<int_t, int_t> oneToTwo(int_t one){
    double z = double(one);
    double w = floor( (sqrt(8.0*z + 1.0) - 1.0) / 2.0);
    double t = (w*w + w) / 2.0;
    double y = z - t;
    double x = w - y;
    return pair<int_t, int_t>(int_t(x), int_t(y));
  }
    
  static int_t twoToOne(int_t one, int_t two){
    double x = double(one);
    double y = double(two);
    return int_t((1.0/2.0)*(x+y)*(x+y+1) + y);
  }
  
  Tri(int_t first, int_t second , int_t third) :
    first(first), second(second), third(third) {}
  
  Tri(int_t one){
    pair<int_t, int_t> x_yz = oneToTwo(one);
    auto y_z = oneToTwo(x_yz.second);
    first = x_yz.first;
    second = y_z.first;
    third = y_z.second;
  }
  
  int_t getFirst() { return first; }
  int_t getSecond() { return second; }
  int_t getThird() { return third; }
  int_t getSingle() { return twoToOne(first, twoToOne(second, third)); }
};
