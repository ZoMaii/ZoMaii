class book:
    def __init__(self,isbn,name,info):
        Pres=[]
        # 属性格式化
        self.id=isbn
        self.name=name
        self.save=Pres
        Pres.append(info)

    def __str__(self):
        # 返回存储内容
        return f'ISBN:{self.id}，Name:{self.name}，Info:{self.save}'

class Stream:
    def __init__(self):
        # 创建对象数据存储池
        self.item=[]
    
    def input(self,isbn,name,info):
        self.item.append(book(isbn,name,info))
    
    def info(self):
        # 查找已经被对象化的数据
        for item in self.item:
            print(item)

ba=Stream()
ba.input(9000,'WOW','yeah')
ba.input(8000,'WOW','bro')
ba.info()