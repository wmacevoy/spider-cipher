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

    def param(self,name,defval=None):
        return self._params[name] if name in self._params else defval

    def tag(self): return self._params['tag']
    def otag(self): return f"<{self.tag()}{self.attrs()} >"
    def octag(self): return f"<{self.tag()}{self.attrs()}/>"
    def ctag(self): return f"</{self.tag()}>"
    def parts(self): return ""

    def parts(self):
        return f"""
{self.bg()}
{self.fg()}
"""
    def xml(self):
        return f"""<?xml version="1.0" encoding="UTF-8" standalone="no"?>"""
    def osvg(self):
        return f"""<svg width="69mm" height="94mm" viewBox="-3 -3 69 94" version="1.1" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns="http://www.w3.org/2000/svg" xmlns:svg="http://www.w3.org/2000/svg">"""
    def csvg(self):
        return f"""</svg>"""


    def defs(self):
        return f"""<defs></defs>"""

    # this includes xml header, svg tag, defs
    def svg(self):
        return f"""{self.xml()}{self.osvg()}{self.defs()}{self.__str__()}{self.csvg()}"""

    def __str__(self):
        parts=self.parts()
        if len(parts) == 0:
            return self.octag()
        else:
            return f"{self.otag()}{parts}{self.ctag()}"
