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

long Schema::get_serializing_length() {
  long len = this->attrs.size();
  for (auto attr: this->attrs) {
    len += attr.length;
  }
  return len;
}

void Record::print() {
  for(auto value: data)
    cout << value << " ";
  cout << endl;
}

long get_file_size(FILE *fp) {
  // return # of bytes
  // source: https://www.codegrepper.com/code-examples/c/how+to+check+the+size+of+a+file+in+linux+c

  fseek(fp, 0, SEEK_END); // seek to end of file
  auto size = ftell(fp); // get current file pointer
  fseek(fp, 0, SEEK_SET);
  return size;
}

Record char2record(char* tuple, Schema &schema) {
  vector<string> data(schema.attrs.size());
  long offset = 0;
  long index = 0;
  for(auto attr: schema.attrs) {
    string attr_value(tuple + offset, (long unsigned)attr.length);
    data[index] = attr_value;
    offset += attr.length + 1; // including the comma
    index++;
  }
  Record rec(&schema, data);
  return rec;
}

char* print_records(vector<Record> records, long tuple_len) {
  char* res = new char[records.size() * tuple_len + 1];

  long index = 0;
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
void print_record_to_buf(Record* rec, char* buf, long position) {
  long index = 0;
  for(auto value: rec->data) {
    strcpy(buf + position + index, value.c_str());
    index += value.size();
    buf[position + index] = ',';
    index++;
  }
  buf[position + index-1] = '\n';
  buf[position + index] = 0;
}

vector<Record> get_records(char* buffer, long tuple_len, Schema &schema, long number_of_records) {
  vector<Record> records(number_of_records);
  for(long i = 0; i < number_of_records; i++) {
    records[i] = char2record(buffer + i * tuple_len, schema);
  }
  return records;
}

// a comparator for records
bool compareRecords(const Record& r1, const Record& r2) {
  for(long sort_attr : r1.schema->sort_attrs){
    long c = r1.data[sort_attr].compare(r2.data[sort_attr]);
    if(c != 0){
      return c < 0;
    }
  }
  return 0;
}

void mk_runs(FILE *in_fp, FILE *out_fp, long run_length, Schema &schema)
{
  long tuple_len = schema.get_serializing_length();
  assert(run_length >= tuple_len);
  long tuples_in_run = run_length / tuple_len;
  long file_size = get_file_size(in_fp);

  vector<Record> records(tuples_in_run, Record());

  long tuples_left = file_size / tuple_len;
  char* tuple = new char[tuple_len + 1];
  while(tuples_left > 0) {
    for(long i = 0; i < min(tuples_left, tuples_in_run); i++) {
      long getline_arg = tuple_len + 1;

      getline(&tuple, (size_t*)(&getline_arg), in_fp);
      // cout << getline_arg << ' ' << strlen(tuple) << endl;
      records[i] = char2record(tuple, schema);
    }
    sort(records.begin(), records.begin() + min(tuples_left, tuples_in_run), compareRecords);
    char* records_for_disk = print_records(records, tuple_len);
    fwrite(records_for_disk, 1, min(tuples_left, tuples_in_run) * tuple_len, out_fp);
    delete[] records_for_disk;

    tuples_left -= tuples_in_run;
  }
  delete[] tuple;
}

void merge_runs(vector<RunIterator *> &iterators, FILE *out_fp,
  long start_pos, char *buf, long buf_size)
{
  long num_runs = iterators.size();

  long tuple_len = iterators[0]->get_schema().get_serializing_length();
  vector<Record*> next_records(num_runs);

  long running_iterators = 0;
  for(long i = 0; i < num_runs; i++){
    if(iterators[i]->has_next()){
      next_records[i] = iterators[i]->next();
      running_iterators++;
    }
  }

  long buffer_position = 0;
  long buffer_record_count = 0;

  while(running_iterators > 0){
    Record *next_rec = nullptr;
    long iterator_index = -1;
    for(long i = 0; i < num_runs; i++) {
      //meaning there is no next in the current run
      if(next_records[i] == nullptr) {
        continue;
      }
      if(iterator_index == -1 or compareRecords(*next_records[i], *next_records[iterator_index])) {
        next_rec = next_records[i];
        iterator_index = i;
      }
    }
    assert(iterator_index != -1);

    // cout << "Printing a sorted record: " ;
    // next_rec->print();
    print_record_to_buf(next_rec, buf, buffer_position);
    buffer_position += tuple_len;
    buffer_record_count++;

    if(buffer_position + tuple_len > buf_size) {
      // write buf to disk and set it to a new char* []
      fwrite(buf, 1, buffer_position, out_fp);
      buffer_position = 0;
      // cout << "tuples_left = " << tuples_left << endl;
    }
    if(iterators[iterator_index]->has_next()) {
      next_records[iterator_index] = iterators[iterator_index]->next();
    } else {
      next_records[iterator_index] = nullptr;
      running_iterators--;
    }
  }

  // write the last part of the buffer to disk
  // cout << "buf = " << endl << buf;
  fwrite(buf, 1, buffer_position, out_fp);
}