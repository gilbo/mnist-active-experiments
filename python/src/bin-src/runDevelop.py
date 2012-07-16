#!/usr/bin/env python

import sys
import string
module_dir = string.replace(sys.path[0], "/bin", "/python")
sys.path = [module_dir] + sys.path # prepend the directory

try: import backend
except ImportError: # then try augmenting the path with env variable
    print "Could not get the backend C++ module loaded correctly..."
    sys.exit(1)



backend.runDevelop()


