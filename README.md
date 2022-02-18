# System Internals Project
 A Kernel Module to Sniff Network System Calls

## Purpose
The creation of the kernel module was part of a mini project I conducted in my fourth year, as part of CMP408. The module records network data such as IP addresses and network ports and logs this to a database held in the AWS cloud. The main aim was to create a way of infiltrating the demo application running on the Rasberry Pi we were provided with and focusing on sniffing network data from visitors that used the application.

Grade: 49/55 **/** 89.09%

## Contents

* **kernelmodule**
    * holds important configuration for the module
* **send_aws.py**
    * script to send the log file data to AWS and initialise LED's
* **Other:**
    * Poster
    * Report
