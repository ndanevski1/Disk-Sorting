#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "leveldb/db.h"
#include "leveldb/options.h"
#include "library.h"
#include "parse_schema.h"

// #include "json/json.h"
#include <jsoncpp/json/json.h>


using namespace std;

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
string get_key_attribute(string tuple, int sort_attribute_index) {
	return parse_tuple(tuple)[sort_attribute_index];
}

int main(int argc, const char* argv[]) {

	if (argc != 4) {
		cout << "ERROR: invalid input parameters!" << endl;
		cout << "Please enter <schema_file> <input_file> <out_index>" << endl;
		exit(1);
	}
	string schema_file(argv[1]);
	string input_file(argv[2]);
	string out_index(argv[3]);

	leveldb::DB *db;
	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status status = leveldb::DB::Open(options, out_index, &db);
	if(!status.ok()){
		cerr << status.ToString() << endl;
		exit(-1);
	}
	vector<string> sort_attrs_name = {"start_year", "student_number"};
	Schema schema = parse_schema(schema_file, sort_attrs_name);

	ifstream input(input_file);
	ofstream output(out_index);

	long counter = 1;
	for(string tuple; getline(input, tuple);) {
		string key_s = get_key_attribute(tuple, schema.sort_attrs[0]) + "_" + to_string(counter);
		counter++;

		leveldb::Slice key = key_s;
		leveldb::Slice value = tuple;

		db->Put(leveldb::WriteOptions(), key, value); 
	}

	leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
	for (it->SeekToFirst(); it->Valid(); it->Next()) {
		// leveldb::Slice key = it->key();
		leveldb::Slice value = it->value();
		// std::string key_str = key.ToString();
		string val_str = value.ToString();
		// cout << key_str << ": "  << val_str << endl;
		output << val_str << endl;
		// cout << val_str << endl;
	}
	assert(it->status().ok());  // Check for any errors found during the scan
	delete it;

	output.close();
	return 0;
}