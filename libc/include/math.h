#ifndef MATH_H
#define MATH_H

#define max(a,b) \
       ({ typeof (a) _a = (a); \
           typeof (b) _b = (b); \
         _a > _b ? _a : _b; })
#define min(a,b) \
        ({ typeof (a) _a = (a); \
            typeof (b) _b = (b); \
          _a < _b ? _a : _b; })
#endif
