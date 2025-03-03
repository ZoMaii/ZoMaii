class run_base:
    def __init__(self,context):
        self.context = context
        self.Data = context['data'][0]
        self.dataName = self.Data['name']
        self.ORM = self.Data['trend']

    def jsonMap(self):return self.context
    def getDataName(self):return self.dataName
    def getUpdataDate(self,less='updateDate'):
        dataTime = []
        for item in self.ORM[less]:
            dataTime.append(item)
        return dataTime
    def getList(self,less='list'):
        ls=[]
        for item in self.ORM[less]:
            ls.append(item)
        return ls
    def getListName(self,less='name'):
        getList = self.getList()
        mum = len(getList)
        ls = []
        for i in range(mum):
            ls.append(getList[i][less])
        return ls
    def getListData(self,TYPENAME:str,Mapping:list,less='data'):
        getList = self.getList()
        if TYPENAME in Mapping:
            INDEX = Mapping.index(TYPENAME)
            return getList[INDEX][less]
        else:
            print('没有对应的命名存在！')
