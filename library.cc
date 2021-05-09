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

void Record::print() {
  for(auto value: data)
    cout << value << " ";
  cout << endl;
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
void print_record_to_buf(Record* rec, char* buf, int position) {
  int index = 0;
  for(auto value: rec->data) {
    strcpy(buf + position + index, value.c_str());
    index += value.size();
    buf[position + index] = ',';
    index++;
  }
  buf[position + index-1] = '\n';
  buf[position + index] = 0; 
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
  char* tuple = new char[tuple_len + 1];
  while(tuples_left > 0) {
    for(int i = 0; i < min(tuples_left, tuples_in_run); i++) {
      int getline_arg = tuple_len + 1;

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


void merge_runs(vector<RunIterator> &iterators, int num_runs, FILE *out_fp,
  long start_pos, char *buf, long buf_size, int total_tuples)
{
  assert(iterators.size() == (unsigned int) num_runs);

  vector<int> indices(num_runs, 0);

  int tuples_left = total_tuples;
  int tuple_len = iterators[0].get_schema().get_schema_length() + iterators[0].get_schema().attrs.size();
  vector<Record*> next_records(num_runs, nullptr);

  int starting_record_to_write = 0;
  int buffer_position = 0;
  
  for(int j = 0; j < total_tuples; j++) {
    Record* next_rec = nullptr;
    int iterator_index = -1;
    for(int i = 0; i < num_runs; i++) {
      // there is no way to check if iterators[i].next() == nullptr, so I just check
      // if it has no data saved inside (i.e. it is equal ot Record())
      if (iterators[i].next()->data.size() == 0 && iterators[i].has_next()) {
        // at this point .next() is updated.
        next_records[i] = iterators[i].next();
      }
      //meaning there is no next in the current run
      if(next_records[i] == nullptr) {
        continue;
      }  
      if(next_rec == nullptr) {
        next_rec = next_records[i];
        iterator_index = i;
      }
      else if (compareRecords(*next_records[i], *next_rec)) {
        next_rec = next_records[i];
        iterator_index = i;
      }
    }
    if(iterator_index != -1) {
      // cout << "Printing a sorted record: " ;
      // next_rec->print();
      print_record_to_buf(next_rec, buf, buffer_position);
      buffer_position += tuple_len;
      
      int current_number_of_tuples = j - starting_record_to_write + 1; // we start from zero
      if(current_number_of_tuples * tuple_len <= buf_size &&
          (current_number_of_tuples + 1) * tuple_len > buf_size) {
            // write buf to disk and set it to a new char* []
            fseek(out_fp, start_pos, SEEK_SET);
            fwrite(buf, 1, current_number_of_tuples * tuple_len, out_fp);
            start_pos += current_number_of_tuples * tuple_len;
            tuples_left -= current_number_of_tuples;
            starting_record_to_write = j + 1;
            delete[] buf;
            buffer_position = 0;
            buf = new char[buf_size];
            // cout << "tuples_left = " << tuples_left << endl;
      }
      if(iterators[iterator_index].has_next())
        next_records[iterator_index] = iterators[iterator_index].next();
      else
        next_records[iterator_index] = nullptr;
    }
  }

  // write the last part of the buffer to disk
  // cout << "buf = " << endl << buf;
  fseek (out_fp, start_pos, SEEK_SET);
  fwrite(buf, 1, tuples_left * tuple_len, out_fp);
}