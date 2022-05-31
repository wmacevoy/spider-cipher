class SVG:
    def __init__(self,params={}):
        self._params = params.copy()
        
    def attr(self,name,suffix=""):
        if name[0] == "@":
            pname = name
            name = pname[1:]
        else:
            pname = "@" + name

        if pname in self._params:
            value=self._params[pname]
            return f"{name}=\"{value}{suffix}\""
        return ""
        
    def attrs(self,suffixes={}):
        ans=""
        for param in self._params:
            if param.startswith("@"):
                suffix = suffixes[param[1:]] if param[1:] in suffixes else ""
                ans = ans + " " + self.attr(param,suffix)
        return ans

    def number(self): return int(self._params['number'] if 'number' in self._params else 0)
    def faceNo(self): return self.number() % 10
    def suitNo(self): return self.number() // 10
    def suit(self):
        suits=['club','diamond','heart','spade']
        return suits[self.suitNo()]
    def color(self):
        colors=['#208020','#202080','#802020','#202020']
        return colors[self.suitNo()]
    def cardId(self):
        return f"card-{self.suitNo()}{self.faceNo()}"
    
    def id(self,suffix=""): return self.attr('id',suffix)

    def tag(self): return self._params['tag']
    def otag(self): return f"<{self.tag()}{self.attrs()} >"
    def octag(self): return f"<{self.tag()}{self.attrs()}/>"
    def ctag(self): return f"</{self.tag()}>"
    def parts(self): return ""

    def __str__(self):
        substr=self.parts()
        if len(substr) == 0:
            return self.octag()
        else:
            return f"{self.otag()}{self.parts()}{self.ctag()}"
