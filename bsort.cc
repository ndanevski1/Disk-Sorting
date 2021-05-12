#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <chrono>

#include "leveldb/db.h"
#include "leveldb/options.h"
#include "library.h"
#include "parse_schema.h"

// #include "json/json.h"
#include <jsoncpp/json/json.h>


using namespace std;
using namespace std::chrono;

vector<string> parse_tuple(string tuple){
	stringstream ss(tuple);
	vector<string> values;

	string attribute;
	while(getline(ss, attribute, ',')){
		values.push_back(attribute);
	}
	ss >> attribute;
	values.push_back(attribute);
	return values;
}

int main(int argc, const char* argv[]) {

	if (argc < 5) {
		cout << "ERROR: invalid input parameters!" << endl;
		cout << "Please enter <schema_file> <input_file> <out_index> <sorting_attributes>" << endl;
		exit(1);
	}
	auto start_time = high_resolution_clock::now();

	string schema_file(argv[1]);
	string input_file(argv[2]);
	string out_index(argv[3]);

	leveldb::DB *db;
	leveldb::Options options;
	options.create_if_missing = true;
	options.error_if_exists = true;
	leveldb::Status status = leveldb::DB::Open(options, ("./" + out_index + "db"), &db);
	if(!status.ok()){
		cerr << status.ToString() << endl;
		exit(-1);
	}
	vector<string> sort_attrs_name;
    for(long i = 4; i < argc; i++){
        string sort_attr(argv[i]);
        sort_attrs_name.push_back(sort_attr);
    }
	Schema schema = parse_schema(schema_file, sort_attrs_name);

	ifstream input(input_file);
	ofstream output;
	output.open(out_index);

	long counter = 1;
	for(string tuple; getline(input, tuple);) {
		string key_s = "";
		vector<string> attributes = parse_tuple(tuple);
		for(long i = 0; i < schema.sort_attrs.size(); i++){
			key_s += attributes[schema.sort_attrs[i]];
			key_s += "_";
		}
		key_s += to_string(counter);
		counter++;

		leveldb::Slice key = key_s;
		leveldb::Slice value = tuple;

		db->Put(leveldb::WriteOptions(), key, value);
	}

	leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
	for (it->SeekToFirst(); it->Valid(); it->Next()) {
		leveldb::Slice value = it->value();
		string val_str = value.ToString();
		output << val_str << endl;
	}
	assert(it->status().ok());  // Check for any errors found during the scan
	delete it;

	output.close();

	auto end_time = high_resolution_clock::now();
 	auto duration = duration_cast<seconds>(end_time - start_time);
  	std::cerr << "BSORT TIME: " << duration.count() << " seconds." << std::endl;
	return 0;
}