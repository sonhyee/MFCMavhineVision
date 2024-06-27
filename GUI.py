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
    signal = pyqtSignal(str)     # 로그 출력 시그널
    signal_res = pyqtSignal(str) # 모델당 결과 출력 시그널
    signal_img = pyqtSignal(str) # 패널 이미지 출력 시그널
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Panel")
        self.signal.connect(self.logPrint)
        self.signal_img.connect(self.imgPrint)
        self.signal_res.connect(self.resPrint)
        
        self.initUI()
        
    def initUI(self):
        instance = None
        self.resize(1000, 600)
        
        
        self.label = QLabel("Model Selection", self)
        self.label.setGeometry(QRect(50, 165, 300, 40))
        self.label.setFont(QFont("Arial", 12))
        
        self.comboBox_model = QComboBox(self)
        self.comboBox_model.setGeometry(QRect(50, 200, 171, 22))
        self.comboBox_model.setObjectName("comboBox_model")
        self.comboBox_model.setFont(QFont("Arial", 10))
        self.comboBox_model.addItem("GALAXY S21")
        self.comboBox_model.addItem("GALAXY S22")
        self.comboBox_model.addItem("GALAXY S23")
        
      
        
        self.listView = QPlainTextEdit(self)
        self.listView.setGeometry(QRect(590, 411, 341, 171))
        self.listView.setObjectName("listView")
        self.listView.setCenterOnScroll(True)
        
        self.push_Start = QPushButton("START", self)
        self.push_Start.setGeometry(QRect(50, 50, 131, 41))
        self.push_Start.setFont(QFont("Arial", 14))
        self.push_Start.setObjectName("StartBtn")
        self.push_Start.clicked.connect(self.start_clicked)
        
        self.push_Start = QPushButton("STOP", self)
        self.push_Start.setGeometry(QRect(50, 110, 131, 41))
        self.push_Start.setFont(QFont("Arial", 14))
        self.push_Start.setObjectName("StopBtn")
        
        self.frame = QFrame(self)
        self.frame.setGeometry(QRect(80, 110, 301, 401))
        self.frame.setObjectName("frame")
        
        self.logView = QPlainTextEdit(self)
        #self.logView.setAcceptRichText(False) # Plain 으로 인식
        self.logView.setGeometry(QRect(590, 60, 361, 341))
        self.logView.setObjectName("logView")
        self.logView.setCenterOnScroll(True)
        
        self.lbl_img = QLabel(self)  # 결과 이미지 출력  대기 화면 출력 카메라 영상 출력
        self.lbl_img.setGeometry(QRect(250, 10, 320, 490))
        self.lbl_img.resize(800, 600)
        
        self.pixmap = QPixmap('black.jpg').scaled(320, 490)
        self.lbl_img.setPixmap(self.pixmap)
        
        self.label = QLabel("<Panel Image>", self)
        self.label.setGeometry(QRect(330, 30, 300, 40))
        self.label.setFont(QFont("Arial", 14))
        
        self.InspectionHandler = Inspection.CInspectionHandler(self)
        self.InspectionHandler.Initialize()
        
    def start_clicked(self):
        self.InspectionHandler.StartInspection()
        
    def logPrint(self, logText):
        self.logView.appendPlainText(logText)
    
    def resPrint(self, resText):
        self.listView.appendPlainText(resText)
    
    def imgPrint(self, PanelImg):
        self.pixmap = QPixmap(PanelImg).scaled(300, 500)
        self.lbl_img.setPixmap(self.pixmap)

if __name__ == '__main__':
    app = QApplication(sys.argv)
    exam = Exam()
    exam.show()
    sys.exit(app.exec_())