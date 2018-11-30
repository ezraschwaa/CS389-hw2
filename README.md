# Computer Systems Homework 5 (by Alyssa Riceman and Monica Moniot)

## 1: State Goals and Define System

We decided to keep our goals very simple: just measuring the average response time of our cache under various circumstances.

We defined our system as being comprised of our client and server programs, the hardware on which they run, and the network through which they communicate.

## 2: List Services and Outcomes

We listed off three services performed by the cache server, each with an array of possible outcomes.

* Service 1: send HTML responses to HTML requests
    * Outcome 1: the response is correct
    * Outcome 2: the response is incorrect
    * Outcome 3: no respone is sent
* Service 2: save user-input data for retrieval
    * Outcome 1: data is successfully saved in a retrievable fashion
    * Outcome 2: data is saved irretrievably or not saved at all
* Service 3: destroy itself when, and only when, given the command to do so
    * Outcome 1: on request, server successfully destroys itself
    * Outcome 2: on request, server fails to destroy itself
    * Outcome 3: server destroys itself despite a lack of request to do so

## 3: Select Metrics

We chose to focus exclusively on sustained throughput, defined as the highest messages-per-second rate at which average response time remains below 1 second.

## 4: List Parameters

Here are the parameters of interest that we came up with:

* Number of requests per second
* Average key size
* Variance in key size
* Average value size
* Variance in value size
* Network speed
* Network overhead
* Processor speed
* Processor overhead
* Ratios of different input types (read:write, write:delete, read:delete, et cetera)
* Maximum cache size

## 5: Select Factors to Study

We settled on three parameters to treat as factors of interest and use as the independent variables in our experiment, each with specific gradations of magnitude as recommended by the text.

* Factor 1: number of requests per second
    * Level 1: 2^4
    * Higher levels: 2^5, 2^6, et cetera, increasing until we get a mean time over 1ms
* Factor 3: network speed
    * Level 1: network is localhost
    * Level 2: network is some non-localhost network

## 6: Select Evaluation Technique

Measurement seems like clearly the best option for us, since we have the client and server programs conveniently available and we don't need to worry about inconveniencing our day-to-day users with traffic from our tests since we don't have any day-to-day users.

## 7: Select Workload

To ensure similarity to the ETC workload described in [this paper](https://www.researchgate.net/publication/254461663_Workload_analysis_of_a_large-scale_key-value_store), we chose to comprise our workload of 65% GET requests, 5% SET requests, and 30% DELETE requests. We chose to order these requests randomly relative to each other, on the basis that we lack any specific expected use-case to simulate and randommess seemed like a solid default in absence of reasons to choose otherwise.

## 8: Design Experiment

[WORK IN PROGRESS]

## 9: Analyze and Interpret Data

[ANALYSIS GOES HERE]

## 10: Present Results

[PRESENTATION GOES HERE]