//Benchmark for the division optimization
//compile with -lbenchmark 
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <benchmark/benchmark.h>
using namespace std;

int div(int n){
  return n / 4;
}

int better_div(int n){
  return n >> 4;
}

vector<int> randVec(int l){
  vector<int> v(l);
  for(auto& el : v)
    el = rand() % l;
  return v;
}

//the actual benchmark, generates a random vector and sorts it
static void bench1(benchmark::State& state) {
  // Perform setup here
  auto v = randVec(state.range(0));
  for (auto _ : state) {
    // This code gets timed
    for(auto& el : v)
      el = div(el);
  }
}

static void bench2(benchmark::State& state) {
  // Perform setup here
  auto v = randVec(state.range(0));
  for (auto _ : state) {
    // This code gets timed
    for(auto& el : v)
      el = better_div(el);
  }
}
// Register the function as a benchmark
BENCHMARK(bench1)->Arg(100);
BENCHMARK(bench2)->Arg(100);
// Run the benchmark
BENCHMARK_MAIN();
