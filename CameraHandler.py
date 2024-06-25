import GUI  # 로그 출력 함수 사용을 위함
import time
from PyQt5.QtCore import pyqtSignal
class CameraHandler:
    #signal = pyqtSignal(str)
    def __init__(self):
        self.shutter_speed = 0
        self.cam_on = False
        self.hExam = None

    def ConnectCam(self, gui):
        self.hExam = gui
        logText = "CameraHandler::ConnectCam called."
        #print_log(logText)
        self.hExam.signal.emit(logText)
        #self.hExam.logPrint(logText)

        time.sleep(0.5)
        self.cam_on = True

        logText = "CameraHandler::ConnectCam finished."
        self.hExam.signal.emit(logText)
        #self.hExam.logPrint(logText)
        #print_log(log_text)

    def SetCamParams(self, shutter_speed):
        logText = "CameraHandler::SetCamParams called({}s).".format(shutter_speed)
        self.hExam.signal.emit(logText)

        time.sleep(1)
        self.shutter_speed = shutter_speed

        logText = "CameraHandler::SetCamParams finished({}s).".format(shutter_speed)
        self.hExam.signal.emit(logText)
        #print_log(log_text)

    def Grab(self, ptn_name):
        logText = "CameraHandler::Grab called({}).".format(ptn_name)
        
        self.hExam.signal.emit(logText)
        time.sleep(self.shutter_speed * 1000)

        logText = "CameraHandler::Grab finished({}).".format(ptn_name)
        
        self.hExam.signal.emit(logText)

    def DisconnectCam(self):
        logText = "CameraHandler::DisconnectCam called."
        
        self.hExam.signal.emit(logText)
        time.sleep(1)
        self.cam_on = False

        logText = "CameraHandler::DisconnectCam finished.\n"
        
        self.hExam.signal.emit(logText)

    def CheckCamOn(self):
        logText = "CameraHandler::CheckCamOn called.\n"
        
        self.hExam.signal.emit(logText)
        return self.cam_on
    
# def print_log(log_text):
#     GUI.Exam.logPrint(log_text)