#!/usr/bin/env python

import sys
import string
module_dir = string.replace(sys.path[0], "/bin", "/python")
sys.path = [module_dir] + sys.path # prepend the directory

try: import backend
except ImportError: # then try augmenting the path with env variable
    print "Could not get the backend C++ module loaded correctly..."
    sys.exit(1)

import wx
import imagePanel
from imagePanel import ImagePanel



if len(sys.argv) < 2:
    print "Need to say which image you want to look at..."
    sys.exit(1)

try:
    image_number = int(sys.argv[1])
except ValueError:
    print "Need to supply an image number..."
    sys.exit(1)




imgBuffer = bytearray(backend.getImageSize())

backend.getTrainingImage(imgBuffer, image_number)



app = wx.PySimpleApp()
frame = wx.Frame(None, title="showImage", size=(200,200))
panel = ImagePanel(frame)
panel.loadFromBytes(imgBuffer, size=(28*4,28*4))

frame.Show()
app.MainLoop()


# poor man's visualization...
#strVersion = ""
#counter = 0
#for byte in imgBuffer:
#    if counter % 4 == 0:
#        if byte < 10:
#            strVersion += "_"
#        else:
#            strVersion += "X"
#    counter += 1
#    if counter == 112:
#        counter = 0
#        print strVersion
#        strVersion = ""
