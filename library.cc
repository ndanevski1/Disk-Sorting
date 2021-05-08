#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <algorithm>
#include "library.h"
// used for file size
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int Schema::get_schema_length() {
  int len = 0;
  for (auto attr: this->attrs) {
    len += attr.length;
  }
  return len;
}

int get_file_size(FILE *fp) {
  // return # of bytes
  // source: https://www.codegrepper.com/code-examples/c/how+to+check+the+size+of+a+file+in+linux+c
  
  fseek(fp, 0, SEEK_END); // seek to end of file
  auto size = ftell(fp); // get current file pointer
  fseek(fp, 0, SEEK_SET); 
  return size;
}

Record char2record(char* tuple, Schema &schema) {
  vector<string> data(schema.attrs.size());
  int offset = 0;
  int index = 0;
  for(auto attr: schema.attrs) {
    string attr_value(tuple + offset, (long unsigned)attr.length);
    data[index] = attr_value;
    offset += attr.length + 1; // including the comma
    index++;
  }
  Record rec(&schema, data);
  return rec;
} 

char* print_records(vector<Record> records, int tuple_len) {
  char* res = new char[records.size() * tuple_len + 1];

  int index = 0;
  for(Record rec: records) {
    for(auto value: rec.data) {
      strcpy(res + index, value.c_str());
      index += value.size();
      res[index] = ',';
      index++;
    }
    res[index-1] = '\n';
  }
  res[index] = 0; 
  return res;
}

vector<Record> get_records(char* buffer, int tuple_len, Schema &schema, int number_of_records) {
  vector<Record> records(number_of_records);
  for(int i = 0; i < number_of_records; i++) {
    records[i] = char2record(buffer + i * tuple_len, schema);
  }
  return records;
}

// a comparator for records
bool compareRecords(Record r1, Record r2) {
  int sorting_attr = r1.schema->sort_attrs[0];
  return (r1.data[sorting_attr] < r2.data[sorting_attr]);
}

void mk_runs(FILE *in_fp, FILE *out_fp, long run_length, Schema &schema)
{
  int tuple_len = schema.get_schema_length() + schema.attrs.size();
  assert(run_length >= tuple_len);
  int tuples_in_run = run_length / tuple_len;
  int file_size = get_file_size(in_fp);

  vector<Record> records(tuples_in_run, Record());

  int tuples_left = file_size / tuple_len;
  char* tuple = new char[tuple_len];
  while(tuples_left > 0) {
    for(int i = 0; i < min(tuples_left, tuples_in_run); i++) {
      int getline_arg = tuple_len - 1;

      getline(&tuple, (size_t*)(&getline_arg), in_fp);

      records[i] = char2record(tuple, schema);

       
    }
    sort(records.begin(), records.begin() + min(tuples_left, tuples_in_run), compareRecords);
    char* records_for_disk = print_records(records, tuple_len);
    fwrite(records_for_disk, 1, min(tuples_left, tuples_in_run) * tuple_len, out_fp);
    delete records_for_disk;
    
    tuples_left -= tuples_in_run;
  }
  delete tuple;
}

void merge_runs(RunIterator* iterators[], int num_runs, FILE *out_fp,
                long start_pos, char *buf, long buf_size)
{
  // Your implementation
}