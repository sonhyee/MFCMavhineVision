class PatternData:
    def __init__(self):
        self.shutter_speed = 0
        self.ptn_name = ""

    def SetPtnName(self, ptn_name):
        self.ptn_name = ptn_name

    def GetPtnName(self):
        return self.ptn_name

    def SetShutterSpeed(self, shutter_speed):
        self.shutter_speed = shutter_speed

    def GetShutterSpeed(self):
        return self.shutter_speed

    def SetPtnImg(self, img):
        self.image = img.copy()

    def GetPtnImg(self):
        img = self.image.clone()
        return img