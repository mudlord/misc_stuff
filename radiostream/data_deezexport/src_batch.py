import glob

sources = glob.glob('./src/*.cpp')
for i in sources:
    print(i)