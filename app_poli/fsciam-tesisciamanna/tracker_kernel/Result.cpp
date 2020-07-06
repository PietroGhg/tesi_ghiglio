#include "Result.h"

Result::Result(std::vector<std::pair<int,int>> _pairs, std::vector<int> _unPairedObjects, std::vector<int> _unPairedTracks):
pairs(_pairs),unPairedTracks(_unPairedTracks),unPairedObjects(_unPairedObjects)
{}
std::vector<std::pair<int,int>> Result::getPairs()
{
  return pairs;
}
std::vector<int> Result::getUnPairedObjects()
{
  return unPairedObjects;
}
std::vector<int> Result::getUnPairedTracks()
{
  return unPairedTracks;
}
