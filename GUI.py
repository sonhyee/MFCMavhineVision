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
        
        
        self.logView = QPlainTextEdit(self)
        #self.logView.setAcceptRichText(False) # Plain 으로 인식
        self.logView.setGeometry(QRect(520, 90, 341, 321))
        self.logView.setObjectName("logView")
        self.logView.setCenterOnScroll(True)
        
        #self.logView = QListView(self)
        #self.logView.setGeometry(QRect(550, 110, 321, 321))
        #self.logView.setObjectName("logView")
        
        #self.listView = QListView(self)
        #self.listView.setGeometry(QRect(550, 441, 321, 121))
        #self.listView.setObjectName("listView")
        
        self.listView = QPlainTextEdit(self)
        self.listView.setGeometry(QRect(520, 421, 341, 151))
        self.listView.setObjectName("listView")
        self.listView.setCenterOnScroll(True)
        
        self.push_Start = QPushButton("START", self)
        self.push_Start.setGeometry(QRect(80, 20, 131, 41))
        self.push_Start.setFont(QFont("Arial", 14))
        self.push_Start.setObjectName("pushButton")
        self.push_Start.clicked.connect(self.start_clicked)
        
        self.frame = QFrame(self)
        self.frame.setGeometry(QRect(80, 110, 301, 401))
        self.frame.setObjectName("frame")
        
        
        self.lbl_img = QLabel(self)  # 결과 이미지 출력  대기 화면 출력 카메라 영상 출력
        self.lbl_img.setGeometry(QRect(110, 50, 301, 401))
        self.lbl_img.resize(800, 600)
        
        self.pixmap = QPixmap('black.jpg').scaled(300, 400)
        self.lbl_img.setPixmap(self.pixmap)
        
        self.label = QLabel("Panel Image", self)
        self.label.setGeometry(QRect(190, 90, 300, 40))
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
        self.pixmap = QPixmap(PanelImg).scaled(300, 400)
        self.lbl_img.setPixmap(self.pixmap)

if __name__ == '__main__':
    app = QApplication(sys.argv)
    exam = Exam()
    exam.show()
    sys.exit(app.exec_())