#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <future>
#include <vector>

#include <NTL/RR.h>
using namespace NTL;

//
// Global variables for arguments and configuration
//
int PRECISION = 10, THREADS = 8;
bool QUIET_MODE = false;
bool SHOW_STAT = false;
std::string OUTPUT_PATH = "";


//
// NTL NUMBERS
//
RR R_ZERO = RR(0.0);
RR R_ONE = RR(1.0);
RR R_TWO = RR(2.0);
RR R_FOUR = RR(4.0);
RR R_1123 = RR(1123.0);
RR R_21460 = RR(21460.0);
RR R_882 = RR(882.0);

//
// functions
//
void computePI();
bool parseArgs(int argc, char** argv);
RR power(RR base, RR exp);
RR fact(RR n);
RR computeMultiplier(RR n);
RR computeMultiplicand(RR n);
RR compute(int i, int from, int to);

int main(int argc, char** argv)
{
  parseArgs(argc, argv);

  // -o OUTPUT_PATH argument
  std::ofstream out(OUTPUT_PATH);
  std::streambuf *coutbuf = nullptr;
  if(out.good()) {
    // redirect output
    coutbuf = std::cout.rdbuf();
    std::cout.rdbuf(out.rdbuf());
  }

  // set ntl precisions
  RR::SetPrecision(200);
  RR::SetOutputPrecision(200);
  // std::cout.precision(999);

  computePI();

  // return std cout buffer back
  if(out.good())
    std::cout.rdbuf(coutbuf);

  // std::cout << "here " << compute(111, 0, 100)<< std::endl;
  return 0;
}

void computePI()
{
  int members_per_thread = PRECISION / THREADS;
  int outsiders_count = PRECISION % THREADS;

  // vector of threads (futures)
  std::vector<std::future<RR>> futures;
  RR result = R_ZERO;

  // start timer
  auto start = std::chrono::system_clock::now();

  int i = 0, from = 0, additional = 0;
  for ( ; i < THREADS; i++) {
    additional = i + 1 == THREADS ? outsiders_count : 0; // if last cycle - add outsiders.
    futures.push_back(
        std::async(
          std::launch::async, compute, i,
          from, from + members_per_thread + additional)
        );

    if(!QUIET_MODE)
      std::cout << "Thread-" << i << " started." << std::endl;

    from += members_per_thread + additional;
  }


  // ok, now lets wait for results from each thread
  // and sum them into one variable/
  for (auto& f : futures) {
    result = result + f.get();
  }

  // std::cout << "res " << result << std::endl;
  result = result * (R_ONE/R_882);
  result = R_FOUR / result;
  auto end = std::chrono::system_clock::now();

  std::chrono::duration<long double> elapsed_seconds = end - start;

  if(!QUIET_MODE) {
    std::cout << "Threads used in current run: " << THREADS << std::endl;
  }

  std::cout << std::fixed << result << std::endl;

  if(SHOW_STAT) {
    std::cout << "in " << elapsed_seconds.count() << " seconds / using " << THREADS << " thread(s)" <<  std::endl;
  }
}

int nextArgumentToInt(char** argv, int i)
{
  try {
    return strtol(argv[i + 1], nullptr, 10);
  } catch(const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return -1;
  };
}
bool parseArgs(int argc, char** argv)
{
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if(arg == "-p" || arg == "-precision") {
      // get precision
      PRECISION = nextArgumentToInt(argv, i);
      if(PRECISION != -1) i++;
    }
    else if (arg == "-t" || arg == "-tasks") {
      // tasks
      THREADS = nextArgumentToInt(argv, i);
      if(THREADS != -1) i++;
    }
    else if(arg == "-q") {
      QUIET_MODE = true;
    }
    else if(arg == "-stat") {
      SHOW_STAT = true;
    }
    else if(arg == "-o") {
      OUTPUT_PATH = argv[i + 1];
      i++;
    }
  }
}


// power function for RR
RR power(RR base, RR exp)
{
  RR result = R_ONE;
  RR i = R_ZERO;
  for(; i < exp; i = i + R_ONE) {
    // std::cout << "yup ";
    result = result * base;
  }

  // std::cout << base << " to " << exp << " is " << result << std::endl;
  return result;
}

// factorial for RR
RR fact(RR n)
{
  return (n == R_ONE || n == R_ZERO) ? R_ONE : fact(n - R_ONE) * n;
}

// ramanujan multiplier
RR computeMultiplier(RR n)
{
  return (power(-R_ONE, n) * fact(R_FOUR*n)) / power( power(R_FOUR, n) * fact(n) , R_FOUR);
}

// ramanujan multiplicand
RR computeMultiplicand(RR n)
{
  return (R_1123 + (R_21460 * n)) / power(R_882, R_TWO*n);
}

// compute Ramanujan serie from .. to
RR compute(int i, int from, int to)
{
  RR result = R_ZERO;
  RR tmpProduct;
  int j = from;
  for ( ; j < to; j++ ) {
    RR multiplier = computeMultiplier(RR(j));
    RR multiplicand = computeMultiplicand(RR(j));
    // std::cout << num<< " and denom: " << denom << std::endl;
    // std::cout << i;
    result += multiplier * multiplicand;
  }

  if(!QUIET_MODE)
    std::cout << "Thread-" << i << " calculated." << std::endl;

  return result;
}
