
import wx

class ImagePanel(wx.Panel):
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        
        self.image = None
        
        self.Bind(wx.EVT_PAINT, self.__paintEvent)

    def __paintEvent(self,  event): # wxPaintEvent
        dc = wx.PaintDC(self)
        self.__render(dc)
    
    #def paintNow(self):
    #    dc = wx.wxScreenDC(self)
    #    self.__render(dc)
    
    def __render(self, dc):
        if self.image:
            dc.DrawBitmap(self.image, 0, 0, False)
    
    def loadFromBytes(self, bytes, size):
        w,h = size
        self.image = wx.EmptyBitmap(w,h)
        self.image.CopyFromBufferRGBA(bytes)


#app = wx.PySimpleApp()
#frame = wx.Frame(None, title="Hello wxDC", size=(800,600))
#drawPane = ImagePanel(frame)
#drawPane.image = wx.Bitmap("avatar.png")

#frame.Add(drawPane)
#frame.Show()
#app.MainLoop()

# events:
#   mouseMoved()
#   mouseDown()
#   mouseWheelMoved()
#   mouseReleased()
#   rightClick()
#   mouseLeftWindow()
#   keyPressed()
#   keyReleased()


