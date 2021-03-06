#
# Simplify ask for forgiveness instead of permission
#
# y = None
# try:
#   y = f(x)
# except:
#   try:
#     y = g(x)
#   except:
#     ...
#
# y = (Maybe(lambda x : f(x))|Maybe(lambda x : g(x))|...).so(x)
#

class Maybe:
    def __init__(self,so):
        self._so = [so]
        
    def __or__(self,no):
        self._so.extend(no._so)
        return self
    
    def so(self,*args):
        for k in range(len(self._so)):
            if k+1 < len(self._so):
                try:
                    return self._so[k](*args)
                except:
                    pass
            else:
                return self._so[k](*args)
