#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <chrono>

#include "parse_schema.h"
#include <jsoncpp/json/json.h>

using namespace std;
using namespace std::chrono;


int main(int argc, const char* argv[]) {
  if (argc < 7) {
    cout << "ERROR: invalid input parameters!" << endl;
    cout << "Please enter <schema_file> <input_file> <output_file> <mem_capacity> <k> <sorting_attributes>" << endl;
    exit(1);
  }
  auto start_time = high_resolution_clock::now();

  string schema_file(argv[1]);

  string filename_in = argv[2];
  vector<string> run_filenames = {"csvRuns1.csv", "csvRuns2.csv"};
  string filename_out = argv[3];
  FILE *in_fp = fopen(filename_in.c_str(), "r");
  vector<FILE *> run_fps = {fopen(run_filenames[0].c_str(), "w+"), fopen(run_filenames[1].c_str(), "w+")};
  FILE *out_fp = fopen(filename_out.c_str(), "w+");

  int mem_capacity = atoi(argv[4]);
  int k = atoi(argv[5]);
  int mem_capacity_for_use = 8*mem_capacity/10;

  vector<string> sort_attrs_name;
  for(int i = 6; i < argc; i++){
    sort_attrs_name.push_back(argv[i]);
  }
  Schema schema = parse_schema(schema_file, sort_attrs_name);

  int run_length = mem_capacity_for_use - mem_capacity_for_use % schema.get_serializing_length();
  int file_size = get_file_size(in_fp);

  FILE *curr_run_fp = run_fps[0];
  if(run_length >= file_size){
    curr_run_fp = out_fp;
  }

  mk_runs(in_fp, curr_run_fp, run_length, schema);

  fclose(in_fp);
  cout << "Did runs" << endl;

  while(run_length < file_size){
    int num_runs = (file_size + run_length - 1) / run_length;
    int buffer_per_run = mem_capacity_for_use / (k + 1);
    buffer_per_run -= buffer_per_run % schema.get_serializing_length();

    FILE *next_file;
    if(run_length * k >= file_size){
      next_file = out_fp;
    } else {
      next_file = run_fps[1];
    }
    fseek(next_file, 0, SEEK_SET);
    fseek(curr_run_fp, 0, SEEK_SET);

    for(int i = 0; i < num_runs; i += k){
      vector<RunIterator *> iterators;
      for(int j = 0; j < k; j++){
        if(run_length * (i + j) < file_size){
          iterators.push_back(new RunIterator(curr_run_fp,
            run_length * (i + j), run_length, buffer_per_run, &schema, file_size / schema.get_serializing_length()));
        }
      }

      char* buf = new char[buffer_per_run];
      merge_runs(iterators, next_file, i * run_length, buf, buffer_per_run);
      delete[] buf;

      for(int j = 0; j < iterators.size(); j++){
        delete iterators[j];
      }
    }

    run_length *= k;
    cout << "Sorted in blocks of " << run_length << endl;

    curr_run_fp = run_fps[1];
    std::swap(run_fps[0], run_fps[1]);
  }

  fclose(run_fps[0]);
  fclose(run_fps[1]);
  remove(run_filenames[0].c_str());
  remove(run_filenames[1].c_str());

  fclose(out_fp);

  auto end_time = high_resolution_clock::now();
  auto duration = duration_cast<seconds>(end_time - start_time);
  std::cerr << "MSORT TIME: " << duration.count() << " seconds." << std::endl;
  return 0;
}
