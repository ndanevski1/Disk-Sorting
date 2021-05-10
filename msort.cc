#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>

#include "parse_schema.h"
// #include "json/json.h"
#include <jsoncpp/json/json.h>


using namespace std;

int main(int argc, const char* argv[]) {
  // if (argc < 7) {
  //   cout << "ERROR: invalid input parameters!" << endl;
  //   cout << "Please enter <schema_file> <input_file> <output_file> <mem_capacity> <k> <sorting_attributes>" << endl;
  //   exit(1);
  // }
  // string schema_file(argv[1]);

  // // Parse the schema JSON file
  // Json::Value schema;
  // Json::Reader json_reader;
  // // Support for std::string argument is added in C++11
  // // so you don't have to use .c_str() if you are on that.
  // ifstream schema_file_istream(schema_file.c_str(), ifstream::binary);
  // bool successful = json_reader.parse(schema_file_istream, schema, false);
  // if (!successful) {
  //   cout << "ERROR: " << json_reader.getFormatedErrorMessages() << endl;
  //   exit(1);
  // }

  // // Print out the schema
  // string attr_name;
  // int attr_len;
  // for (int i = 0; i < schema.size(); ++i) {
  //   attr_name = schema[i].get("name", "UTF-8" ).asString();
  //   attr_len = schema[i].get("length", "UTF-8").asInt();
  //   cout << "{name : " << attr_name << ", length : " << attr_len << "}" << endl;
  // }

  // Do the sort
  // Your implementation

  

  string filename_in = "csv10000.csv";
  string filename_runs = "csv_runs10000.csv";
  string filename_out = "csv_out10000.csv";
  FILE *in_fp = fopen(filename_in.c_str(), "r");
  FILE *fp_runs = fopen(filename_runs.c_str(), "w+");
  FILE *out_fp = fopen(filename_out.c_str(), "w+");
  int k = 10;
  int mem_capacity = 30000;
  int mem_capacity_for_use = 8*mem_capacity/10; // = 80% = 24000

  vector<string> sort_attrs_name = {"start_year", "student_number"};
  Schema schema = parse_schema("schema_example.json", sort_attrs_name);

  int run_length = mem_capacity_for_use; // = 24000  
  mk_runs(in_fp, fp_runs, run_length, schema);

  int tuple_len = 29;
  int file_size = 290000;
  // why exactly do we need k? Also asked on piazza.
  int number_of_runs = (file_size + run_length -1) / run_length; // = 13
  int tuples_in_run = run_length / tuple_len; // = 827
  int total_tuples = 10000;

  vector<RunIterator *> iterators;
  for(int i = 0; i < number_of_runs; i++) {
    iterators.push_back(new RunIterator(fp_runs, tuple_len*i*tuples_in_run, run_length, run_length/number_of_runs, &schema));
  }

  long buf_size = 1000;
  char* buf = new char[buf_size];
  
  merge_runs(iterators, out_fp, 0, buf, buf_size);
  delete[] buf;
  return 0;
}
