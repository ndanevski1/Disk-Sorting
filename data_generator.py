import json
import random
import string
from scipy.stats import truncnorm

def get_truncated_normal(mean=0, sd=1, low=0, upp=10):
  # source: https://stackoverflow.com/questions/36894191/how-to-get-a-normal-distribution-within-a-range-in-numpy
  return truncnorm(
        (low - mean) / sd, (upp - mean) / sd, loc=mean, scale=sd)

'''
You should implement this script to generate test data for your
merge sort program.

The schema definition should be separate from the data generation
code. See example schema file `schema_example.json`.
'''

def generate_data(schema, out_file, nrecords):
  '''
  Generate data according to the `schema` given,
  and write to `out_file`.
  `schema` is an list of dictionaries in the form of:
    [ 
      {
        'name' : <attribute_name>, 
        'length' : <fixed_length>,
        ...
      },
      ...
    ]
  `out_file` is the name of the output file.
  The output file must be in csv format, with a new line
  character at the end of every record.
  '''
  print("Generating %d records"  % nrecords)

  f = open(out_file, "w")

  for i in range(nrecords):
    for j, attribute in enumerate(schema):
      if attribute["type"] in ["integer", "float"]:
        if "distribution" in attribute and attribute["distribution"] == "normal":
          mu = attribute["distribution"]["mu"]
          sigma = attribute["distribution"]["sigma"]
          minn = attribute["distribution"]["min"]
          maxx = attribute["distribution"]["max"]


          X = get_truncated_normal(mu, sigma, minn, maxx)
          num = X.rvs()

        else:
          minn = attribute["distribution"]["min"]
          maxx = attribute["distribution"]["max"]
          num = random.uniform(minn, maxx)


        if attribute["type"] == "integer":
          assert 10**(attribute["length"]-1) <= minn <= num <= maxx < 10**attribute["length"], "number not in range"
          num = int(num)
        num_str = str(num)[:attribute["length"]]
        f.write(num_str)
        if j != len(schema)-1:
          f.write(",")
        else:
          f.write("\n")

      if attribute["type"] == "string":
        s = ''.join(random.choice(string.ascii_lowercase) for x in range(attribute["length"]))
        f.write(s)
        if j != len(schema)-1:
          f.write(",")
        else:
          f.write("\n")


  f.close()      




if __name__ == '__main__':
  import sys, json
  if not len(sys.argv) == 4:
    print("data_generator.py <schema json file> <output csv file> <# of records>")
    sys.exit(2)

  schema = json.load(open(sys.argv[1]))
  output = sys.argv[2]
  nrecords = int(sys.argv[3])
  print(schema)

  generate_data(schema, output, nrecords)