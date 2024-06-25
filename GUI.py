import sys
import AlgorithmData as Alg
import CameraHandler as Cam
import PatternData as Ptn
import threading
import queue
import configparser as parser
import cv2
import InspectionHandler as Inspection
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5.QtGui import *

class Exam(QWidget):
    signal = pyqtSignal(str)
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Panel")
        self.signal.connect(self.logPrint)
        self.initUI()
        
    def initUI(self):
        instance = None
        self.resize(1000, 600)
        
        
        self.logView = QPlainTextEdit(self)
        #self.logView.setAcceptRichText(False) # Plain 으로 인식
        self.logView.setGeometry(QRect(550, 110, 321, 321))
        self.logView.setObjectName("logView")
        
        
        #self.logView = QListView(self)
        #self.logView.setGeometry(QRect(550, 110, 321, 321))
        #self.logView.setObjectName("logView")
        
        self.listView = QListView(self)
        self.listView.setGeometry(QRect(550, 441, 321, 121))
        self.listView.setObjectName("listView")
        
        self.push_Start = QPushButton("START", self)
        self.push_Start.setGeometry(QRect(80, 20, 131, 41))
        self.push_Start.setFont(QFont("Arial", 14))
        self.push_Start.setObjectName("pushButton")
        self.push_Start.clicked.connect(self.start_clicked)
        
        self.frame = QFrame(self)
        self.frame.setGeometry(QRect(80, 110, 301, 401))
        self.frame.setObjectName("frame")
        
        self.label = QLabel("Panel Image", self)
        self.label.setGeometry(QRect(190, 70, 300, 40))
        self.label.setFont(QFont("Arial", 14))
        
        self.InspectionHandler = Inspection.CInspectionHandler(self)
        
       
    def start_clicked(self):
        self.InspectionHandler.Initialize()
        self.InspectionHandler.StartInspection()
        
        
    def logPrint(self, logText):
        self.logView.appendPlainText(logText)
         
if __name__ == '__main__':
    app = QApplication(sys.argv)
    exam = Exam()
    exam.show()
    sys.exit(app.exec_())