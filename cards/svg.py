class SVG:
    def __init__(self,params={},defaults={}):
        self._params = params.copy()
        for name in defaults:
            if not name in self._params:
                self._params[name]=defaults[name]

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

    def param(self,name,defval=None):
        return self._params[name] if name in self._params else defval

    def tag(self): return self._params['tag']
    def otag(self): return f"<{self.tag()}{self.attrs()} >"
    def octag(self): return f"<{self.tag()}{self.attrs()}/>"
    def ctag(self): return f"</{self.tag()}>"
    def parts(self): return ""

    def __str__(self):
        parts=self.parts()
        if len(parts) == 0:
            return self.octag()
        else:
            return f"{self.otag()}{parts}{self.ctag()}"
