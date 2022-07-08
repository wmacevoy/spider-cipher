#!/usr/bin/env python3

from glyph import Glyph

paramsDict = {'good-face':{'number':35,'shift':'down'},
              'bad-face':{'number':34,'shift':'down'},
              'good-thumb':{'number':35,'shift':'none'},
              'bad-thumb':{'number':34,'shift':'none'},
              'good-heart':{'number':35,'shift':'up'},
              'bad-heart':{'number':34,'shift':'up'}}

for name in paramsDict:
    params = paramsDict[name].copy()
    params['lines'] = False
    params['hints'] = False
    glyph = Glyph.build(params)
    glyph._params.pop("@transform")

    with open(name + ".svg","w") as fd:
        fd.write(glyph.svg())