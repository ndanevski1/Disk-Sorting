#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdio>
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

int get_file_size(FILE *in_fp) {
  // return # of bytes
  // source: https://www.codegrepper.com/code-examples/c/how+to+check+the+size+of+a+file+in+linux+c
  auto fd = fileno(in_fp); 
  struct stat buf;
  fstat(fd, &buf);
  int size = buf.st_size;
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

// a comparator for records
bool compareRecords(Record r1, Record r2) {
  int sorting_attr = r1.schema->sort_attrs[0];
  return (r1.data[sorting_attr] < r2.data[sorting_attr]);
}

void mk_runs(FILE *in_fp, FILE *out_fp, long run_length, Schema &schema)
{
  // TODO: check if -1 needed for the \n character: 0x0a
  int tuple_len = schema.get_schema_length() + schema.attrs.size();
  int tuples_in_run = run_length / tuple_len;
  int file_size = get_file_size(in_fp);

  int number_of_runs = (file_size / tuple_len + tuples_in_run - 1) / tuples_in_run;

  vector<Record> records(tuples_in_run, Record());

  int tuples_left = file_size / tuple_len;
  while(tuples_left > 0) {
    for(int i = 0; i < min(tuples_left, tuples_in_run); i++) {
      char* tuple = new char[tuple_len];

      getline(&tuple, (size_t*)(&tuple_len), in_fp);

      records[i] = char2record(tuple, schema);
    }
    sort(records.begin(), records.begin() + min(tuples_left, tuples_in_run), compareRecords);
    char* records_for_disk = print_records(records, tuple_len);
    fwrite(records_for_disk, 1, min(tuples_left, tuples_in_run) * tuple_len, out_fp);
    delete(records_for_disk);
    
    tuples_left -= tuples_in_run;
  }
}

void merge_runs(RunIterator* iterators[], int num_runs, FILE *out_fp,
                long start_pos, char *buf, long buf_size)
{
  // Your implementation
}