## this is an example of simple event detection in London dataset

## 1 - this script checks if the program is already compiled, and if the data are ready

file="../sourceCode/EventDetectionServer/EventDetectionServer"
if [ ! -f "$file" ]
then
    echo "Launch compilation"
    (cd ../sourceCode/EventDetectionServer && make)
    pwd
else
	echo "program already compiled"
fi

file="EventDetectionServer"
if [ ! -f "$file" ]
then
    
    (cp ../sourceCode/EventDetectionServer/EventDetectionServer EventDetectionServer)
    pwd
fi


if [ -d "../data/producedGraphLondon2017" ]
then
    echo "data are ready to use"
else
	echo "Launch extraction of data from compressed file"
    unzip ../data/producedGraphLondon2017.zip -d ../data/producedGraphLondon2017
fi

file="./results"
if [ ! -f "$file" ]
then
    mkdir results
fi

## 2 - this script finally launches the event detector :
./EventDetectionServer objectiveEventDetection parameters.txt


