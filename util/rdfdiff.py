#!usr/bin/env python2
#
#  RDF graph diff, useful for diffing SPDX license for release checklist.
#
#  Based on:
#
#    - https://www.w3.org/2001/sw/wiki/How_to_diff_RDF
#    - https://github.com/RDFLib/rdflib/blob/master/rdflib/compare.py
#

import os
import sys

def main():
    from rdflib import Graph
    from rdflib.compare import to_isomorphic, graph_diff

    with open(sys.argv[1]) as f:
        d1 = f.read()
    with open(sys.argv[2]) as f:
        d2 = f.read()

    print('Loading graph 1 from ' + sys.argv[1])
    g1 = Graph().parse(format='n3', data=d1)

    print('Loading graph 2 from ' + sys.argv[2])
    g2 = Graph().parse(format='n3', data=d2)

    iso1 = to_isomorphic(g1)
    iso2 = to_isomorphic(g2)

if __name__ == '__main__':
    main()
