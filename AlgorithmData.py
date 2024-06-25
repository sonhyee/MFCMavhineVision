
class AlgorithmData:
    def __init__(self):
        self.m_szAlgName = ""
        self.m_bAlgResult = False
        #"" for _ in range(10)
        self.m_szPtnName = ["" for _ in range(10)]  # assuming maximum number of patterns is 100
        self.m_nPtnCount = 0

    def SetAlgName(self, alg_name):
        self.m_szAlgName = alg_name
       

    def GetAlgName(self):
        return self.m_szAlgName

    def GetAlgResult(self):
        return self.m_bAlgResult

    def SetAlgPtnInfo(self, ptn_name):
        str = ptn_name
        delim = ","
        
        # pos = 0
        while (pos := str.find(delim)) != -1:
            self.m_szPtnName[self.m_nPtnCount] = str[:pos]
            self.m_nPtnCount += 1
            str = str[pos + len(delim):]

        self.m_szPtnName[self.m_nPtnCount] = str
        self.m_nPtnCount += 1

    def CheckAlgPtn(self, ptn_name):
        for i in range(self.m_nPtnCount):
            if self.m_szPtnName[i] == ptn_name:
                return True
        
        return False
    
    def RunAlgorithm(self, ptn_name):
        return True