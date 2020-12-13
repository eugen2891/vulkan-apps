def globdefs():
    print("")
    print("typedef float GEO_VERTEX[2][3];\n")
    print("typedef GEO_VERTEX GEO_TRIANGLE[3];\n")

def condimpl(decl, defn):
    print("#ifndef GEO_INTERNAL\n\nextern %s;\n\n#else\n\n%s =\n%s\n\n#endif\n" % (decl, decl, defn))

def implonly(decl, defn):
    print("#ifdef GEO_INTERNAL\n\nstatic %s =\n%s\n\n#endif\n" % (decl, defn))

def pyramid():
    out = []
    f = [[0, 2, 1], [0, 3, 2], [0, 4, 3], [0, 1, 4], [1, 2, 3], [3, 4, 1]]
    p = [[0.0, 0.6374, 0.0], [-0.5, 0.0, 0.5], [-0.5, 0.0, -0.5], [0.5, 0.0, -0.5], [0.5, 0.0, 0.5]]
    for t in f:
        a = [ p[t[1]][i] - p[t[0]][i] for i in range(3) ]
        b = [ p[t[2]][i] - p[t[1]][i] for i in range(3) ]
        x = a[1] * b[2] - a[2] * b [1]
        y = a[2] * b[0] - a[0] * b [2]
        z = a[0] * b[1] - a[1] * b [0]
        n = "{%ff, %ff, %ff}" % (x, y, z)
        ts = []
        for i in t:
            v = "{%ff, %ff, %ff}" % (p[i][0], p[i][1], p[i][2])
            ts.append("{%s, %s}" % (v, n))
        out.append("    {\n        %s\n    }" % ",\n        ".join(ts))
    condimpl("const GEO_TRIANGLE GEO_PYRAMID_VB[]", "{\n%s\n};" % ",\n".join(out))


globdefs()
pyramid()
