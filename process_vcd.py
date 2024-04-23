#!/usr/bin/env python3
import json
import sys
from pyDigitalWaveTools.vcd.parser import VcdParser
import pandas as pd

if len(sys.argv) > 1:
    fname = sys.argv[1]
else:
    print('Give me a vcd file to parse')
    sys.exit(-1)

#make a pandas dataframe

df = pd.DataFrame()

with open(fname) as vcd_file:
    vcd = VcdParser()
    vcd.parse(vcd_file)
    data = vcd.scope.toJson()
    with open("testout.txt", "a") as f:
        f.write(json.dumps(data, indent=4, sort_keys=True))
        f.close()
    #find max time in data
    max_time = 0
    for child in (data['children']):
        for child2 in (child['children']):
            if('data' in child2.keys()):
                if child2['data'][-1][0] > max_time:
                    max_time = child2['data'][-1][0]

    for child in (data['children']):
        for child2 in (child['children']):
            if('data' in child2.keys()):
                curr_time = 0
                wire_size = child2['type']['width']
                for datapoint in child2['data']:
                    #format the value to be of length wire_size (extend with 0s or xs (if just an x))
                    #remove the b' if it has it
                    wdata = datapoint[1]
                    wdata = wdata.replace("'", "")
                    wdata = wdata.replace("b", "")
                    if len(wdata) < wire_size:
                        if wdata == 'x':
                            wdata = 'x' * (wire_size)
                        else:
                            wdata = '0' * (wire_size - len(wdata)) + wdata
                    #put the value in the dataframe
                    df.at[child2['name'], datapoint[0]] = wdata
                    #copy the last non NaN value to the times until the current time
                    for i in range(curr_time, datapoint[0], 500):
                        df.at[child2['name'], i] = df.at[child2['name'], curr_time]
                    if datapoint[0] > curr_time:
                        curr_time = datapoint[0]
                #copy the last non NaN value to the times until the max time
                for i in range(curr_time, max_time + 1, 500):
                    df.at[child2['name'], i] = df.at[child2['name'], curr_time]

    #sort the columns by time
    df = df.reindex(sorted(df.columns), axis=1)
    #save csv 
    df.to_csv('output.csv')
            # print(child2['name'])
    # print(json.dumps(data, indent=4, sort_keys=True))