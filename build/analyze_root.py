import ROOT
import sys
import csv

input_file = sys.argv[1]

rdf = ROOT.RDataFrame("LXe", input_file)
data = rdf.AsNumpy()

hit_PMT = data["hitPMT"]

print(hit_PMT)
