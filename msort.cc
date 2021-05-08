#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>

#include "parse_schema.h"
// #include "json/json.h"
#include <jsoncpp/json/json.h>


using namespace std;

void print_record(Record r) {
  for(auto value: r.data)
    cout << value;
  cout << endl;
}

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

  

  string filename_in = "csv.csv";
  string filename_out = "csvOut.csv";
  FILE *in_fp = fopen(filename_in.c_str(), "r");
  FILE *out_fp = fopen(filename_out.c_str(), "w+");

  vector<string> sort_attrs_name = {"start_year", "student_number"};
  Schema schema = parse_schema("schema_example.json", sort_attrs_name);
    
  mk_runs(in_fp, out_fp, 2000, schema);

  RunIterator r_iter(out_fp, 0, 2000, 250, &schema);
  while(r_iter.has_next()) {
    Record* rec = r_iter.next();
    print_record(*rec);
  }
  return 0;
}
