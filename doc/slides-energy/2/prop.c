inline void propagateLocations(RepGraph& G){
  //Topological sort the graph
  std::deque<Vertex> Sorted;
  topological_sort(G, std::front_inserter(Sorted));

  //Following the topological sort, insert the locations
  //of the source in the target.
  for(auto V : Sorted){
    for(auto E : make_iterator_range(out_edges(V,G)))
      for(auto& Loc : G[V].Locations)
	G[target(E,G)].Locations.insert(Loc);
  }
}
