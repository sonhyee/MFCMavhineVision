import cv2
import time
from VisionAlgorithm import *

class AlgorithmData:
    def __init__(self):
        self.m_szAlgName = ""
        self.m_bAlgResult = False
        #"" for _ in range(10)
        self.m_szPtnName = ["" for _ in range(10)]  # assuming maximum number of patterns is 100
        self.m_nPtnCount = 0
        self.preprocessed = False
        # 검사할 모델 정보 저장
        self.information, tae, target_model = configuration()

    def SetAlgName(self, alg_name):
        self.m_szAlgName = alg_name
       

    def GetAlgName(self):
        return self.m_szAlgName

    def GetAlgResult(self):
        return self.m_bAlgResult

    def SetAlgPtnInfo(self, ptn_name):
        string = ptn_name
        delim = ","
        
        # pos = 0
        while (pos := string.find(delim)) != -1:
            self.m_szPtnName[self.m_nPtnCount] = str[:pos]
            self.m_nPtnCount += 1
            string = str[pos + len(delim):]

        self.m_szPtnName[self.m_nPtnCount] = string
        self.m_nPtnCount += 1

    def CheckAlgPtn(self, ptn_name):
        for i in range(self.m_nPtnCount):
            if self.m_szPtnName[i] == ptn_name:
                return True
        
        return False
    
    def preprocessing(self, ptn_name):
        self.img_trans_color, self.img_trans = preprocessing(ptn_name)
        cv2.imwrite('rotation.jpg', self.img_trans_color)
        self.img_expand = expand(self.img_trans)
        self.img_expand = cv2.cvtColor(self.img_expand, cv2.COLOR_GRAY2BGR)
        self.preprocessed = True
        
    def RunAlgorithm(self, ptn_name): # ptn_name = WHITE001.jpg
        # 먼저 들어오는 스레드로 전처리 이미지 획득 후 알고리즘 실행
        #if not self.preprocessed:
        self.preprocessing(ptn_name)
        
        #if img_trans_color is None or img_trans_color.size() == 0:
            
        time.sleep(1)
        if self.m_szAlgName == 'scratch':
            self.m_bAlgResult, vertical_lines, horizontal_lines = line_detection(self.img_expand)
        elif self.m_szAlgName == 'stain':
            img_c = cv2.cvtColor(self.img_trans_color.copy(), cv2.COLOR_BGR2GRAY)
            img_a = img_c.copy()
            img_a = cv2.cvtColor(img_a, cv2.COLOR_GRAY2BGR)
            img_morph = morph(img_c, (7,7))
            img_c = cv2.Canny(img_morph, 30, 30)
            self.m_bAlgResult = stain_detection(img_c, img_a, 8)
        elif self.m_szAlgName == 'point':
            self.m_bAlgResult = point_detection(self.img_expand, 1, 10000000, self.information)
        elif self.m_szAlgName == 'redDefect':
            self.m_bAlgResult = red_detection(self.img_trans_color)
        elif self.m_szAlgName == 'greenDefect':
            self.m_bAlgResult = green_detection(self.img_trans_color)
        elif self.m_szAlgName == 'blueDefect':
            self.m_bAlgResult = blue_detection(self.img_trans_color)
            
        return 'rotation.jpg'
        