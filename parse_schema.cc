#include <iostream>
#include <fstream>
#include <cassert>

#include "library.h"
#include <jsoncpp/json/json.h>

type to_type(string s) {
    if(s == "string")
        return t_string;
    else if (s == "integer")
        return t_integer;
    else if (s == "float")
        return t_float;

    assert(false);
}

Schema parse_schema(string schema_file, vector<string> sort_attrs_name) {
    // Parse the schema JSON file
    Json::Value root;
    Json::Reader json_reader;
    // Support for std::string argument is added in C++11
    // so you don't have to use .c_str() if you are on that.
    ifstream schema_file_istream(schema_file.c_str(), ifstream::binary);
    bool successful = json_reader.parse(schema_file_istream, root, false);
    if (!successful) {
        cout << "ERROR: " << json_reader.getFormattedErrorMessages() << endl;
        exit(1);
    }

    vector<Attribute> attrs;
    vector<int> sort_attrs;

    for(auto attribute: root) {
        Attribute a(
            attribute["name"].asString(),
            (int) attribute["length"].asInt(),
            to_type(attribute["type"].asString()));
        attrs.push_back(a);
    }

    for(auto sort_attribute: sort_attrs_name) {
        long counter = 0;
        for(auto attribute: root) {
            if(attribute["name"].asString() == sort_attribute) {
                sort_attrs.push_back(counter);
                break;
            }
            counter++;
        }
    }
    Schema schema(attrs, sort_attrs);
    return schema;
}

// long main() {
//     vector<string> sort_attrs_name = {"account_name", "student_number"};

//     Schema schema = parse_schema("schema_example.json", sort_attrs_name);
//     cout << "Attributes: ";
//     for(auto attribute: schema.attrs)
//         cout << attribute.name << ", ";
//     cout << endl;
//     for(auto sort_attribute: schema.sort_attrs)
//         cout << sort_attribute << ", ";
//     cout << endl;
//     return 0;
// }