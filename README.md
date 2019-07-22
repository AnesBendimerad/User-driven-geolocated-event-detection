# User driven geolocated event detection in social media
This repository provides the materials for the paper: " User-driven geo-spatial event detection in social media".

It is organized as follows:

- **data**: This folder contains the real world data used in the experiments.

- **sourceCode**: This folder contains the C++ algorithm implementations of the approaches defined in the paper. After compiling it, make sure you put the file ModularityOptimizer.jar (available in "example" folder) in the same repository than the compiled C++ program.

- **example**: it contains a script that launches a simple event detection in London dataset.

In what follows, we present in more details the structure of the provided dataset, and we explain how to use the C++ implementation

# The dataset
We share the input datasets required by the event detection program. These datasets are built based on geolocated tweets posted in three different cities: New York, Los Angeles, and London. Each dataset contains 4 compressed files:

- metadata.txt: This file contains metadata about the provided dataset (e.g., the dimensions of the dynamic graph G, the number of used tweets,...)

- hashtagCooccurrences.txt: this file contains the co-occurrences of terms in each timestamp

- hashtagsGraph.json: this file contains the main graph G=(V,E). It provides the list of vertices and their connectivity relations, and also the occurrence of each term in each pair (vertex, timestamp)

- coordinates.json: this file specifies the beginning time and the ending time (date and time) of each timestamp in T. It also specifies the geographic bounding box corresponding to each vertex v of the graph G=(V,E)

# The source code
We provide the C++ source code that implements the algorithms SIGLER_Cov and SIGLER_Samp. After compilation, the program can be used in two different ways:

- Data-driven event detection: it allows to process a data-driven event detection based on a given input graph. This doesn't manage user preferences.

- Launch of the event detection server: it consists in launching a server that listens to queries using the port number 20500. It proposes several types of queries that allows to process event detection in different settings, and to take into account user interactions

We detail this two alternatives in what follows

## Data-driven event detection
To launch a data-driven event detection, the following command line can be used:

```
name_of_executable objectiveEventDetection <parameters_file_path> 
```

parameters\_file\_path is the path of the file that specifies the values of differents parameters and arguments. If you want to use the algorithm SIGnaLER_Cov, the following parameters must be specified in the parameters file:

```
graphFile:<path of the input graph file>
hashtagCooccurrencesFile:<path of hashtag cooccurrences file>
outputFile:<path of the results file>
minCov:<value of the threshold minCov>
minQuality:<value of the threshold δ>
minSize:<minimum number of vertices by patterns>
minTime:<minimum size of time interval by patterns>
beginMineTime:<timestamp from which the event detection begins>
endMineTime:<timestamp in which the event detection ends>
userId:<id number, in order to make it possible to manage several users simultanuously>
```

If you want to use the algorithm SIGnaLER_Samp, you must also specify the following parameters in the file:

```
approach:sampling
samplingTimeInMS:<the time budget in milliseconds>
postProcessing:<the type of post processing>
```

If the value of postProcessing is removeRepetition, then a post processing will be applied in order to remove repetition. If the value of postProcessing is removeRedundancy, then the result will be post processed in order to return a summary that respects the constraint of minCov.

## Launch of the event detection server
To launch the event detection server, the following command line can be used:

```
name_of_executable launchServer
```

Then, you can communicate with the server using socket queries. The number of the used port is 20500. We present below the different features of this server.

- Data-driven event detection: We can also launch a data-driven event detection using the server. We use the following query:

```
objectiveEventDetection <parameters_file_path>
```

The parameters file is the same as the one presented in the "Simple Event Detection" section.

- User-driven event detection: in order to run a user-driven event detection process, a first step consists in loading the data using the query:

```
launchSubjectiveDetection <parameters_file_path>
```

The parameters file contains:

```
graphFile:<path of the input graph file>
hashtagCooccurrencesFile:<path of the terms co-occurrences file>
userId:<id number, in order to make it possible to manage several users simultanuously>
```

Afterthat, you can send queries of event detection, in which you can also specify some events liked by the user. These events will be used to learn the user preferences. The user-driven event detection query is presented below:

```
continueSubjectiveDetection <parameters_file_path>
```

the parameters file contains:

```
outputFile:<path of the results file>
minCov:<value of the threshold minCov>
minQuality:<value of the threshold δ>
minSize:<minimum number of vertices by patterns>
minTime:<minimum size of time interval by patterns>
beginMineTime:<timestamp from which the event detection begins>
endMineTime:<timestamp in which the event detection ends>
fileContainingLikedPatterns:<the file containing liked patterns in a json format>
chosenPatterns:<indices of liked patterns in fileContainingLikedPatterns, separated commas>
userId:<id number, in order to make it possible to manage several users simultanuously>
```

fileContainingLikedPatterns contains the liked patterns, and it is a results file previously produced by this current program. 

Furthermore, the server accepts the query addNewLikes. This query allows to send to the server a set of events liked by the user. The syntax of this query is:

```
addNewLikes <parameters_file_path>
```

the parameters file contains:

```
fileContainingLikedPatterns:<the file containing liked patterns in a json format>
chosenPatterns:<indices of liked patterns in fileContainingLikedPatterns, separated commas>
```




