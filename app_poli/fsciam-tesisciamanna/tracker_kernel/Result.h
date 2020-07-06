#ifndef RESULT_H
#define RESULT_H
#include <vector>

class Result
{
  public:
    Result(std::vector<std::pair<int,int>> _pairs, std::vector<int> _unPairedObjects, std::vector<int> _unPairedTracks);
    std::vector<std::pair<int,int>> getPairs();
    std::vector<int> getUnPairedObjects();
    std::vector<int> getUnPairedTracks();
  private:
    std::vector<std::pair<int,int>> pairs;
    std::vector<int> unPairedObjects;
    std::vector<int> unPairedTracks;

};
#endif
