import GUI # 로그 출력 함수 사용 위해
import configparser as parser
import threading
import time
import queue
import cv2
import CameraHandler as Camera
import PatternData as Ptn
import AlgorithmData as Alg
from PyQt5.QtCore import *

class CInspectionHandler():
    def __init__(self, parent):
        self.parent = parent # 
        
        self.m_nTotalPtnCnt = 0
        self.m_nTotalAlgCnt = 0
        self.m_nPtnChkAlgCnt = 0
        self.m_PtnChkAlgCntLock = threading.Lock()
        self.m_bRunCamThread = True
        self.m_bRunAlgThread = True
        self.m_nTotalFinAlgCnt = 0
        self.m_TotalFinAlgCntLock = threading.Lock()
        self.m_bFinInspection = False
        self.m_nGrabCnt = 0
        self.m_nFinModelCnt = 0
        self.m_patterns = []
        
        self.m_Camera = Camera.CameraHandler()
        #self.m_hWnd = None
        self.m_strResText = ""
        
        self.m_CamThread = None
        self.m_CamQueue = queue.Queue()
        self.m_CamLock = threading.Lock()
        self.m_Cam_cv = threading.Condition(self.m_CamLock)
        
        self.m_AlgThreadVec = []
        self.m_AlgLock = threading.Lock()
        self.m_AlgQueue = queue.Queue()
        self.m_Alg_cv = threading.Condition(self.m_AlgLock)
        self.AlgThreadSleeping = True
        #self.GUI.Camera_Log("되나?")
       
        #self.Initialize()

    # Destructor
    # def __del__(self):
    #     if Camera:
    #         if Camera.CheckCamOn():
    #             self.m_CamCv.notify_all()  # Wake up all sleeping camera threads
    #             #self.m_CamMutex.lock()
    #             self.m_CamLock.acquire()
    #             while not self.m_CamQueue.empty():
    #                 self.m_CamQueue.get()  # Clear all waiting camera operations for shutdown
    #             self.m_CamQueue.put('DISCONNECT')  # Disconnect the camera if it is connected
    #             #self.m_CamMutex.unlock()
    #             self.m_CamLock.release()
    #             time.sleep(30)  # Wait until DISCONNECT is dequeued

    #         self.m_bRunCamThread = False  # Request to end camera thread
    #         self.m_CamThread.join()  # Wait for camera thread to end
    #         del Camera

    #     #self.m_AlgMutex.lock()
    #     self.m_AlgLock.acquire()
    #     while not self.m_AlgQueue.empty():
    #         self.m_AlgQueue.get()  # Clear all waiting algorithms for shutdown
    #     #self.m_AlgMutex.unlock()
    #     self.m_AlgLock.release()    
        
    #     self.m_bRunAlgThread = False  # Request to end algorithm thread

    #     self.m_AlgCv.notify_all()  # Wake up all sleeping algorithm threads

    #     for i_Alg in range(self.m_nTotalAlgCnt):
    #         self.m_AlgThreadVec[i_Alg].join()  # Wait for algorithm threads to end

    #     if self.m_patterns:
    #         del self.m_patterns
    #     if self.m_Algorithms:
    #         del self.m_Algorithms
            
    def Initialize(self):
        #m_hWnd = hWnd
        if not self.ReadConfig():
            ss = "파일을 확인하세요.\n"
            #logText = ss
            #self.PrintLog(logText)
            #self.GUI.Camera_Log(ss)
            return

        #CameraHandler = CameraHandler()  # 카메라 핸들러 객체 생성
        self.m_CamThread = CameraWorker(self)
        # self.m_CamThread.start()
        
        #self.m_CamThread = threading.Thread(target=self.ProcessingCamQueue)
        
        self.m_CamThread.start()
        self.m_CamQueue.put('CONNECT')  # 카메라 연결
        
    def StartInspection(self):
        self.m_nTotalFinAlgCnt = 0  # 검사를 완료한 총 알고리즘 개수 초기화
        self.m_nPtnChkAlgCnt = 0  # 패턴별 확인을 완료한 총 알고리즘 개수 초기화
        self.m_nGrabCnt = 0  # 현재까지 촬상한 패턴 개수 초기화
        self.m_strResText = ""
        
        self.m_CamLock.acquire()
        for i_Ptn in range(self.m_nTotalPtnCnt):
            self.m_CamQueue.put('SETCAMPARAMS')  # 파라미터 세팅
            self.m_CamQueue.put('GRAB')  # 촬상
            self.m_Cam_cv.notify()  # 자고있는 카메라 쓰레드 깨우기
        self.m_CamLock.release()
        
    def ReadConfig(self):
        properties = parser.ConfigParser()
        properties.read('config.ini')
        sec_name = ""
        buf = ""

        # Pattern Load
        self.m_nTotalPtnCnt = int(properties['general']['total pattern count'])
        
        if self.m_nTotalPtnCnt == 0:
            return False

        #self.m_patterns = [CPatternData() for _ in range(self.m_nTotalPtnCnt)]
        for i in range(self.m_nTotalPtnCnt):
            self.m_patterns.append(Ptn.PatternData())

        for i in range(self.m_nTotalPtnCnt):
            sec_name = 'pattern%02d' % i

            buf = properties[sec_name]['pattern name']  # Read pattern name

            if buf == "":
                return False

            self.m_patterns[i].SetPtnName(buf)  # Set pattern name
            self.m_patterns[i].SetShutterSpeed(properties[sec_name]['shutter speed'])  # Read and set shutter speed

        # Algorithm Load
        self.m_nTotalAlgCnt = int(properties['general']['total algorithm count'])

        if self.m_nTotalAlgCnt == 0:
            return False

        for i in range(self.m_nTotalAlgCnt):
            #self.m_AlgThreadVec.append(threading.Thread(target=self.ProcessingAlgQueue))  # Create threads for each algorithm
            self.m_AlgThreadVec.append(AlgorithmWorker(self))
            self.m_AlgThreadVec[i].start()
            
        self.m_Algorithms = [Alg.AlgorithmData() for _ in range(self.m_nTotalAlgCnt)]

        for i in range(self.m_nTotalAlgCnt):
            sec_name = "algorithm%02d" % i

            buf = properties[sec_name]['algorithm name']  # Read algorithm name

            if buf == "":
                return False

            self.m_Algorithms[i].SetAlgName(buf)  # Set algorithm name
            #print(self.m_Algorithms[i].GetAlgName())
            buf = properties[sec_name]['pattern']  # Read pattern used by the algorithm

            if buf == "":
                return False

            self.m_Algorithms[i].SetAlgPtnInfo(buf)  # Set pattern used by the algorithm

        return True
    
            
    def get_model_count(self):
        return self.m_nFinModelCnt + 1

    # GUI에서 문자열을 호출하도록 -> 문자열을 반환하는 함수로 대체
    # def print_log(self, log_text, defect):
    #     if defect:
    #         SendMessage(self.hwnd, UM_LOGMSG, DEFECTMSG, log_text)
    #         return

    #     SendMessage(self.hwnd, UM_LOGMSG, None, log_text)

    def DisconnectCam(self):
        self.m_CamLock.lock()
        self.m_CamQueue.put('DISCONNECT')
        self.m_CamLock.unlock()
        
        
class CameraWorker(QThread):
    def __init__(self, parent):
        super(CameraWorker,self).__init__()
        self.parent = parent # InspectionHandler를 상속
    def run(self):
        while self.parent.m_bRunCamThread:
            with self.parent.m_CamLock: # with문 탈출시 자동 release
                self.parent.m_Cam_cv.wait_for(lambda: not self.parent.m_CamQueue.empty() or not self.parent.m_bRunCamThread)

            if not self.parent.m_bRunCamThread:    
                return

            option = self.parent.m_CamQueue.get() # 원소 반환 및 제거
            #self.m_CamQueue.get()    # Retrieve and store the task if there is something to do, then delete it from the queue

            match option:
                case 'CONNECT':
                    self.parent.m_Camera.ConnectCam(self.parent.parent)
                    #logPrint(self.m_Camera.ConnectCam())
                    #self.m_Camera.ConnectCam()    # Connect the camera
                    #print(str(list(self.parent.m_CamQueue.queue)))
                    
                case 'SETCAMPARAMS':
                    print('setcamparams')
                    self.parent.m_Camera.SetCamParams(self.parent.m_patterns[self.parent.m_nGrabCnt].GetShutterSpeed())    # Set parameters
                    
                case 'GRAB':
                    print('grab')
                    img_name = self.parent.m_patterns[self.parent.m_nGrabCnt].GetPtnName() + "00" + str(self.parent.get_model_count()) + ".jpg"
                   
                    img = cv2.imread(img_name)

                    if img is None or img.size == 0:
                        while not self.parent.m_CamQueue.empty():
                            self.parent.m_CamQueue.get()

                        log_text = "Inspection Finished.\n"
                        self.parent.parent.signal.emit(log_text)
                        self.parent.m_nFinModelCnt = 0
                        continue

                    self.parent.m_Camera.Grab(self.parent.m_patterns[self.parent.m_nGrabCnt].GetPtnName())    

                    self.parent.m_patterns[self.parent.m_nGrabCnt].SetPtnImg(img)
                    #pattern_ptr = id(self.parent.m_patterns[self.parent.m_nGrabCnt])
                    print(img_name)
                    self.parent.parent.signal_img.emit(img_name)

                    self.parent.m_AlgLock.acquire()
                    self.parent.m_AlgQueue.put((self.parent.m_patterns[self.parent.m_nGrabCnt].GetPtnName(), self.parent.m_nGrabCnt))    
                    print(str(list(self.parent.m_AlgQueue.queue)))
                    print(self.parent.AlgThreadSleeping)
                    if self.parent.AlgThreadSleeping:
                        self.parent.m_Alg_cv.notify()    # Wake up all sleeping algorithm threads to enable parallel processing
                    self.parent.m_AlgLock.release()

                    self.parent.m_nGrabCnt += 1    # Increment the number of patterns captured so far
                    
                case 'DISCONNECT':
                    if self.parent.m_Camera.CheckCamOn():
                        self.parent.m_Camera.DisconnectCam()    # Disconnect the camera after checking if it is connected
                    
                
                
class AlgorithmWorker(QThread):
    def __init__(self, parent):
        super(AlgorithmWorker,self).__init__()
        self.parent = parent # InspectionHandler를 상속
    def run(self):
         while self.parent.m_bRunAlgThread:
            with self.parent.m_AlgLock:
                self.parent.m_Alg_cv.wait_for(lambda: not self.parent.m_AlgQueue.empty() or not self.parent.m_bRunCamThread)
                
            self.parent.m_AlgLock.acquire()
            
            if not self.parent.m_bRunAlgThread:
                self.parent.m_AlgLock.release()
                return

            self.parent.AlgThreadSleeping = False
            ptn_name, ptn_num = self.parent.m_AlgQueue.queue[0]
            alg_idx = self.parent.m_nPtnChkAlgCnt

            
            self.parent.m_PtnChkAlgCntLock.acquire()
            self.parent.m_nPtnChkAlgCnt += 1

            if self.parent.m_nPtnChkAlgCnt == self.parent.m_nTotalAlgCnt:
                self.parent.m_AlgQueue.get()
                print(str(list(self.parent.m_AlgQueue.queue)))
                self.parent.m_nPtnChkAlgCnt = 0
            self.parent.m_PtnChkAlgCntLock.release()
            self.parent.m_AlgLock.release()

            print(alg_idx)
            if self.parent.m_Algorithms[alg_idx].CheckAlgPtn(ptn_name):
                self.parent.m_Algorithms[alg_idx].RunAlgorithm(self.parent.m_patterns[ptn_num].GetPtnImg())

                if self.parent.m_Algorithms[alg_idx].GetAlgResult():
                    logText = f"[{ptn_name}00{self.parent.get_model_count()}] : Alg[{self.parent.m_Algorithms[alg_idx].GetAlgName()}] OK."
                    self.parent.parent.signal.emit(logText)
                else:
                    logText = f"[{ptn_name}00{self.parent.get_model_count()}] : Alg[{self.parent.m_Algorithms[alg_idx].GetAlgName()}] NG."
                    self.parent.m_strResText += self.parent.m_Algorithms[alg_idx].GetAlgName() + " "
                    self.parent.parent.signal.emit(logText)

            self.parent.m_TotalFinAlgCntLock.acquire()
            self.parent.m_nTotalFinAlgCnt += 1

            if self.parent.m_nTotalFinAlgCnt == self.parent.m_nTotalPtnCnt * self.parent.m_nTotalAlgCnt:
                logText = "Inspection completed.\n"
                self.parent.parent.signal.emit(logText)

                self.strAllRes = f"MODEL 00{self.parent.get_model_count()}: {self.parent.m_strResText} NG"
                self.parent.parent.signal_res.emit(self.strAllRes)

                self.parent.m_nTotalAlgCnt = 0
                self.parent.m_nFinModelCnt += 1
                self.parent.AlgThreadSleeping = True
                self.strResText = ""
                self.parent.StartInspection()

            self.parent.m_TotalFinAlgCntLock.release()