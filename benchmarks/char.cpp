//Benchmark for the char optimization
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
  char c1 = 100;
  char c2 = 101;
  for (auto _ : state) {
    // This code gets timed
    benchmark::DoNotOptimize(c1 = 3*c1 + c2);
  }
}

static void bench2(benchmark::State& state) {
  // Perform setup here
  int i1 = 100;
  int i2 = 101;
  for (auto _ : state) {
    // This code gets timed
    benchmark::DoNotOptimize(i1 = 3*i1 + i2);
  }
}
// Register the function as a benchmark
BENCHMARK(bench1);
BENCHMARK(bench2);
// Run the benchmark
BENCHMARK_MAIN();
