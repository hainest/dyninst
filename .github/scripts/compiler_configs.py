import json
import argparse

configs = {
  'ubuntu-20.04': [
    {
      'compiler': 'gcc',
      'versions': [7, 8, 9, 10]
    },
    {
      'compiler': 'clang',
      'versions': [7, 8, 9, 10, 11, 12]
    }      
  ],
  'ubuntu-22.04': [
    {
      'compiler': 'gcc',
      'versions': [11, 12]
    },
    {
      'compiler': 'clang',
      'versions': [13, 14, 15]
    }      
  ],
  
  # Ubuntu 23 and 24 no longer have DockerHub images, so we can't use them
  # for testing.
  
  'ubuntu-25.04': [
    {
      'compiler': 'gcc',
      'versions': [13, 14, 15]
    },
    {
      'compiler': 'clang',
      'versions': [17, 18, 19, 20]    # clang-16 is not available in Ubuntu 25
    }
  ],

  # Fedora has only one version of gcc and clang per release
  'fedora-37': [
    {
      'compiler': 'clang',
      'versions': [15]
    },
    {
      'compiler': 'gcc',
      'versions': [12]
    }
  ],
  'fedora-38': [
    {
      'compiler': 'clang',
      'versions': [16]
    },
    {
      'compiler': 'gcc',
      'versions': [13]
    }
  ],
  'fedora-39': [
    {
      'compiler': 'clang',
      'versions': [17]
    },
    {
      'compiler': 'gcc',
      'versions': [13]
    }
  ],
  'fedora-40': [
    {
      'compiler': 'clang',
      'versions': [18]
    },
    {
      'compiler': 'gcc',
      'versions': [14]
    }
  ],
  'fedora-41': [
    {
      'compiler': 'clang',
      'versions': [19]
    },
    {
      'compiler': 'gcc',
      'versions': [14]
    }
  ],
  'fedora-42': [          # same compilers as 41, but has glibc-2.41
    {
      'compiler': 'clang',
      'versions': [19]
    },
    {
      'compiler': 'gcc',
      'versions': [14]
    }
  ],
    'fedora-43': [        # same compilers as 42, but has glibc-2.42
    {
      'compiler': 'clang',
      'versions': [19]
    },
    {
      'compiler': 'gcc',
      'versions': [14]
    }
  ],
}

parser = argparse.ArgumentParser(
    description="Generate compiler multibuild configurations for Github CI"
)
parser.add_argument(
    "--types",
    type=str,
    help="Build types as a JSON array (e.g., '[\"DEBUG\", \"RELEASE\"]')"
)
parser.add_argument(
    "--compiler-names",
    action='store_true',
    help="Return the generic names of the available compilers"
)
parser.add_argument(
    "--compiler-translate",
    type=str,
    help="Translate from a generic to a language-specific compiler"
)
parser.add_argument(
    "--os-all",
    action='store_true',
    help="Return the names of all supported operating systems"
)
parser.add_argument(
    "--os-latest",
    action='store_true',
    help="Return the names of the most recent versions of the supported operating systems"
)

args = parser.parse_args()

if args.compiler_names:
  print("[ 'gcc', 'clang' ]")
  exit()

if args.compiler_translate:
  xlat = {
    "gcc": '"cc":"gcc","cxx":"g++"',
    "clang": '"cc":"clang","cxx":"clang++"'
  }[args.compiler_translate]
  print(f"{{ {xlat} }}")
  exit()

if args.os_all:
  print('[' + ','.join([f"'{k}'" for k in configs.keys()]) + ']')
  exit()

if args.os_latest:
  print("[ 'ubuntu-25.04', 'fedora-43' ]")
  exit()

types = json.loads(args.types)

matrix = []
for bt in types:
  for os in configs:
    for cconfig in configs[os]:
      for v in cconfig['versions']:
        matrix.append(
          {
            "os": os,
            "build-type": bt,
            "compiler": cconfig['compiler'],
            "compiler-version": v,
          }
        )

print("matrix={{ \"include\" : {0:s} }}".format(json.dumps(matrix)))
