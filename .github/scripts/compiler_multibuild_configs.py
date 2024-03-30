import json
import argparse

ubuntu_configs = {
  'ubuntu-20.04': [
    {
      'compilers': ['gcc', 'g++'],
      'versions': [7, 8, 9, 10]
    },
    {
      'compilers': ['clang', 'clang++'],
      'versions': [7, 8, 9, 10, 11, 12]
    }      
  ],
  'ubuntu-22.04': [
    {
      'compilers': ['gcc', 'g++'],
      'versions': [11, 12]
    },
    {
      'compilers': ['clang', 'clang++'],
      'versions': [13, 14, 15]
    }      
  ],
  'ubuntu-23.10': [
    {
      'compilers': ['gcc', 'g++'],
      'versions': [13]
    },
    {
      'compilers': ['clang', 'clang++'],
      'versions': [16, 17]
    }
  ],
  'ubuntu-24.04': [
    {
      'compilers': ['clang', 'clang++'],
      'versions': [18]
    }
  ]
}

parser = argparse.ArgumentParser(
    description="Generate compiler multibuild configurations for Github CI"
)
parser.add_argument(
    "types",
    type=str,
    help="Build types as a JSON array (e.g., '[\"DEBUG\", \"RELEASE\"]')"
)

types = json.loads(parser.parse_args().types)

matrix = []
for bt in types:
  for os in ubuntu_configs:
    for cconfig in ubuntu_configs[os]:
      for v in cconfig['versions']:
        cc = cconfig['compilers'][0]
        cxx = cconfig['compilers'][1]
        matrix.append(
          {
            "os": os,
            "build-type": bt,
            "c-compiler": "{0:s}-{1:d}".format(cc, v),
            "cxx-compiler": "{0:s}-{1:d}".format(cxx, v),
          }
        )

# Fedora has only one version of gcc and clang per release
fedora_configs = [
    'fedora-37', # gcc-12, clang-15
    'fedora-38', # gcc-13, clang-16
    'fedora-39', # gcc-13, clang-17
  ]

for f in fedora_configs:
  for bt in types:
    for c in [('gcc','g++'), ('clang','clang++')]:
      cc = c[0]
      cxx = c[1]
      matrix.append(
        {
          "os" : f,
          "build-type": bt,
          "c-compiler": cc,
          "cxx-compiler": cxx,
        }
      )

print("matrix={{ \"include\" : {0:s} }}".format(json.dumps(matrix)))
